/* bt_util.c -- Bluetooth connectivity helper

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include <unistd.h>
#include <stdlib.h>
#include <blts_log.h>

#include "bt_util.h"


struct bt_state {
	char *hs_mac;
	char *hs_pin;
	DBusGConnection *bus;
	DBusGProxy *manager;
	DBusGProxy *adapter;
	DBusGProxy *audio;
	DBusGProxy *audiosink;
};
static struct bt_state *state;

static void app_init_run_once()
{
	static int init_done = 0;
	if (init_done)
		return;
	if (!g_thread_supported())
		g_thread_init(0);

	g_type_init();
	dbus_g_thread_init();
	init_done = 1;
}

/* Logs the given GError, with special handling for dbus remote errors */
void display_dbus_glib_error(GError *error)
{
	if (!error)
		return;

	if (error->domain == DBUS_GERROR
		&& error->code == DBUS_GERROR_REMOTE_EXCEPTION) {
		BLTS_ERROR("Caught remote method exception %s : %s\n",
			dbus_g_error_get_name(error), error->message);
			return;
	}
	BLTS_ERROR("Error: %s\n", error->message);
	return;
}


static int run_pairing()
{
	BLTS_ERROR("*** Pairing not implemented yet\n");
	return -1;
}

static int audio_is_connected()
{
	GHashTable *props = 0;
	GError *error = 0;
	GValue *audiostate = 0;
	const char *state_str = 0;

	if (!state || !state->audiosink)
		return 0;
	if (!dbus_g_proxy_call(state->audiosink, "GetProperties", &error, G_TYPE_INVALID,
		dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &props,
		G_TYPE_INVALID)) {
		display_dbus_glib_error(error);
		BLTS_TRACE("AudioSink GetProperties call failed.\n");
		g_error_free(error);
		return 0;
	}

	if (!props) {
		BLTS_ERROR("No properties for audio device.\n");
		return 0;
	}

	audiostate = g_hash_table_lookup(props, "State");
	if (!audiostate) {
		BLTS_TRACE("No 'State' parameter..?\n");
		g_hash_table_destroy(props);
		return 0;
	}

	state_str = g_value_get_string(audiostate);
	if(!strcmp(state_str, "connected")
		|| !strcmp(state_str, "playing")) {
		return 1;
	}
	g_hash_table_destroy(props);
	return 0;
}

static int connectdevice(struct bt_state *state)
{
	GError *error = 0;
	char *remote_dev_path = 0;
	int retries, ret;
	BLTS_TRACE("Attempting Bluetooth connection to %s...\n", state->hs_mac);

	if (!dbus_g_proxy_call(state->adapter, "FindDevice", &error,
		G_TYPE_STRING, state->hs_mac, G_TYPE_INVALID,
		DBUS_TYPE_G_OBJECT_PATH, &remote_dev_path, G_TYPE_INVALID)) {
		BLTS_TRACE("Device data not found, trying to pair.\n");
		g_error_free(error);
		error = 0;
		ret = run_pairing();
		if (ret) {
			BLTS_ERROR("New bluetooth device, but cannot pair.\n");
			return -1;
		}
		if (!dbus_g_proxy_call(state->adapter, "FindDevice", &error,
			G_TYPE_STRING, state->hs_mac, G_TYPE_INVALID,
			DBUS_TYPE_G_OBJECT_PATH, &remote_dev_path, G_TYPE_INVALID)) {
			BLTS_ERROR("Device paired but not available. Giving up.\n");
			g_error_free(error);
			error = 0;
			return -1;
		}
	}

	state->audiosink = dbus_g_proxy_new_for_name(state->bus,
		"org.bluez", remote_dev_path, "org.bluez.AudioSink");

	if (!state->audiosink) {
		BLTS_ERROR("Can't get audiosink proxy.\n");
		return -1;
	}

	BLTS_TRACE("Bluez device is %s\n", remote_dev_path);
	BLTS_TRACE("Set up audio...\n");

	retries = 10;
	while (retries) {
		if (audio_is_connected())
			break;
		if (!dbus_g_proxy_call(state->audiosink, "Connect", &error,
			G_TYPE_INVALID, G_TYPE_INVALID)) {
			display_dbus_glib_error(error);
			BLTS_TRACE("Bluetooth AudioSink Connect failed.\n");
			g_error_free(error);
			error = 0;
		}
		if (audio_is_connected())
			break;
		sleep(2); /* Some devices need a while to wake up */
		if (audio_is_connected())
			break;

		retries--;
		sleep(1);
		BLTS_TRACE("Retrying...\n");
	}

	if (!retries) {
		BLTS_ERROR("Could not connect Bluetooth audio.\n");
		return -1;
	}

	BLTS_TRACE("BT audio connected.\n");

	return 0;
}


static struct bt_state *bt_init()
{
	GError *error = 0;
	char *default_adapter_path = 0;

	state = malloc(sizeof *state);
	memset(state, 0, sizeof *state);

	BLTS_TRACE("Bluetooth init...\n");
	state->bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (!state->bus) {
		display_dbus_glib_error(error);
		BLTS_ERROR("Can't connect system bus.\n");
		g_error_free(error);
		goto failure;
	}

	state->manager = dbus_g_proxy_new_for_name(state->bus,
		"org.bluez", "/", "org.bluez.Manager");
	if (!state->manager) {
		BLTS_ERROR("Can't get proxy for Bluetooth manager.\n");
		goto failure;
	}
	if (!dbus_g_proxy_call(state->manager, "DefaultAdapter", &error,
		G_TYPE_INVALID,
		DBUS_TYPE_G_OBJECT_PATH, &default_adapter_path,	G_TYPE_INVALID)) {

		display_dbus_glib_error(error);
		BLTS_ERROR("Can't find default adapter.\n");
		g_error_free(error);
		goto failure;
	}

	state->adapter = dbus_g_proxy_new_for_name(state->bus,
		"org.bluez", default_adapter_path, "org.bluez.Adapter");

	if (!state->adapter) {
		BLTS_ERROR("Can't get proxy for Bluetooth adapter.\n");
		goto failure;
	}

	if (!dbus_g_proxy_call(state->adapter, "RequestSession", &error,
		G_TYPE_INVALID, G_TYPE_INVALID)) {
		display_dbus_glib_error(error);
		BLTS_ERROR("Session request failed.\n");
		g_error_free(error);
		goto failure;
	}
	dbus_g_connection_flush(state->bus);
	BLTS_TRACE("Bluetooth is online.\n");
	if (default_adapter_path)
		free(default_adapter_path);
	return state;
failure:
	if (state) {
		free(state);
		state = 0;
	}
	if (default_adapter_path)
		free(default_adapter_path);
	return 0;
}

static void bt_finalize()
{
	GError *error = 0;
	if (!state)
		return;
	if (state->audiosink) {
		g_object_unref(state->audiosink);
	}
	if (state->audio) {
		g_object_unref(state->audio);
	}
	if (state->adapter) {
		if (!dbus_g_proxy_call(state->adapter, "ReleaseSession", &error,
					G_TYPE_INVALID, G_TYPE_INVALID)) {
			display_dbus_glib_error(error);
			BLTS_ERROR("Session release failed.\n");
			g_error_free(error);
		}
		BLTS_TRACE("Bluetooth released.\n");
		g_object_unref(state->adapter);
	}
	if (state->manager) {
		g_object_unref(state->manager);
	}

	if (state->hs_mac)
		free(state->hs_mac);
	if (state->hs_pin)
		free(state->hs_pin);

	free(state);
	state = 0;

	return;
}

int bt_util_connect_btaudio(char *remote_mac, char *pin)
{
	int ret;
	app_init_run_once();
	state = bt_init();
	if (!state) {
		BLTS_ERROR("Bluetooth init failed.\n");
		return -1;
	}

	state->hs_mac = strdup(remote_mac);
	state->hs_pin = strdup(pin);

	ret = connectdevice(state);

	return ret;
}

int bt_util_disconnect_btaudio()
{
	int ret = 0;
	GError *error = 0;
	if (!state || !state->audiosink)
		return -EINVAL;
	if (audio_is_connected()) {
		if (!dbus_g_proxy_call(state->audiosink, "Disconnect", &error,
			G_TYPE_INVALID, G_TYPE_INVALID)) {
			display_dbus_glib_error(error);
			BLTS_ERROR("Bluetooth Audio Disconnect failed.\n");
			g_error_free(error);
			ret = -1;
		}
		BLTS_TRACE("BT Audio disconnected.\n");
	}
	bt_finalize();
	return ret;
}
#if 0
int main(int argc, char **argv)
{
	int ret;

	log_open("btaudio",1);
	ret = bt_util_connect_btaudio("00:07:A4:B8:39:02","1234");
	if (ret) {
		BLTS_ERROR("Fail\n");
		return ret;
	}
	ret = bt_util_disconnect_btaudio();
	if (ret) {
		LOGERR("Fail\n");
		return ret;
	}
	BLTS_DEBUG("Ok.\n");
	return 0;
}
#endif
