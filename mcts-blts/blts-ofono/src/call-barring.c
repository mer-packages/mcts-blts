/* call-barring.c -- Voicecall barring functions

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <blts_log.h>

#include "ofono-util.h"
#include "ofono-general.h"
#include "signal-marshal.h"
#include "ofono-modem-bindings.h"

#include "call-barring.h"

struct call_barring_state {
	my_ofono_data*	ofono_data;
	DBusGProxy*		proxy;

	GCallback signalcb_CallBarring_PropertyChanged;
	GCallback signalcb_CallBarring_IncomingBarringInEffect;
	GCallback signalcb_CallBarring_OutgoingBarringInEffect;

	const char*	expected_key;
	GValue*		expected_value;
	gint		result;
};

static gboolean call_barring_timeout(gpointer data);
static void setup_signal_callbacks(struct call_barring_state* state);
static void remove_signal_callbacks(struct call_barring_state* state);

static void on_call_barring_property_changed(DBusGProxy *proxy,
		char *key, GValue* value, gpointer user_data);
static void on_call_barring_incoming_barring_in_effect(DBusGProxy *proxy,
		gpointer user_data);
static void on_call_barring_outgoing_barring_in_effect(DBusGProxy *proxy,
		gpointer user_data);

int ofono_call_barring_disable_all(void* user_ptr, char* modem_path)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;

	if(!data->old_pin)
	{
		LOG("usage: -o <pin>\n");
		return -1;
	}

	ofono_call_barring_properties(user_ptr, modem_path);

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										modem_path,
										OFONO_CALL_BARRING_INTERFACE);
	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
		return -1;
	}

	if(!org_ofono_CallBarring_disable_all(proxy, data->old_pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		g_object_unref (proxy);
		proxy = NULL;
		return -1;
	}

	return 0;
}


int ofono_call_barring_disable_all_incoming(void* user_ptr, char* modem_path)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;

	if(!data->old_pin)
	{
		LOG("usage: -o <pin>\n");
		return -1;
	}

	ofono_call_barring_properties(user_ptr, modem_path);

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										modem_path,
										OFONO_CALL_BARRING_INTERFACE);
	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
		return -1;
	}

	if(!org_ofono_CallBarring_disable_all_incoming(proxy, data->old_pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		g_object_unref (proxy);
		proxy = NULL;
		return -1;
	}

	g_object_unref (proxy);
	proxy = NULL;
	return 0;
}


int ofono_call_barring_disable_all_outgoing(void* user_ptr, char* modem_path)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;

	if(!data->old_pin)
	{
		LOG("usage: -o <pin>\n");
		return -1;
	}

	ofono_call_barring_properties(user_ptr, modem_path);

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										modem_path,
										OFONO_CALL_BARRING_INTERFACE);
	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
		return -1;
	}

	if(!org_ofono_CallBarring_disable_all_outgoing(proxy, data->old_pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		g_object_unref (proxy);
		proxy = NULL;
		return -1;
	}

	g_object_unref (proxy);
	proxy = NULL;

	return retval;
}


int ofono_call_barring_properties(void* user_ptr, char* modem_path)
{

	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;
	GHashTable* properties=NULL;

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										modem_path,
										OFONO_CALL_BARRING_INTERFACE);
	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
		return -1;
	}

	if(!org_ofono_CallBarring_get_properties(proxy, &properties, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		retval = -1;
	}
	if (properties) {
		g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		g_hash_table_destroy(properties);
	}
	properties = NULL;

	return retval;
}

int ofono_call_barring_change_password(void* user_ptr, char* modem_path)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;

	if(!data->old_pin ||
		!data->new_pin)
	{
		LOG("usage: -o <old pin> -n <new pin>\n");
		return -1;
	}

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										modem_path,
										OFONO_CALL_BARRING_INTERFACE);
	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
		return -1;
	}

	if(!org_ofono_CallBarring_change_password(proxy, data->old_pin, data->new_pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		retval = -1;
	}

	return retval;
}

int ofono_call_barring_set(void* user_ptr, char *arg, GValue *value)
{
	struct call_barring_state* state = (struct call_barring_state*)user_ptr;
	GError *error = NULL;

	if(!state->ofono_data->old_pin)
	{
		LOG("usage: -o <pin> \n");
		return -1;
	}

	state->expected_key = arg;
	state->expected_value = value;

	/* state->result is set to 0 in the PropertyChanged callback if the
	   property changes as expected */
	state->result = -1;

	if(!org_ofono_CallBarring_set_property(state->proxy, arg, value,
			state->ofono_data->old_pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return -1;
	}

	guint source_id = g_timeout_add(1000, (GSourceFunc)call_barring_timeout, state);

	g_main_loop_run(state->ofono_data->mainloop);

	g_source_remove(source_id);

	return state->result;
}

int ofono_call_barring_test(void* user_ptr, __attribute__((unused)) int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	int retval = 0;
	int i;

	if(my_ofono_get_modem(data))
		return -1;

	struct call_barring_state state;
	state.ofono_data = data;
	state.proxy = NULL;

	for(i=0; i<data->number_modems; i++)
	{
		state.proxy = dbus_g_proxy_new_for_name (data->connection,
				OFONO_BUS,
				data->modem[i],
				OFONO_CALL_BARRING_INTERFACE);
		if (!state.proxy)
		{
			LOG ("Failed to open proxy for " OFONO_CALL_BARRING_INTERFACE "\n");
			return -1;
		}

		setup_signal_callbacks(&state);

		GValue val = G_VALUE_INIT;
		g_value_init (&val, G_TYPE_STRING);

		g_value_set_string (&val, BARRING_OUT_ALL);
		LOG("Testing to barr all outgoing\n");
		retval += ofono_call_barring_set((void *)&state, BARRING_OUTCALL, &val);

		g_value_set_string (&val, BARRING_OUT_INT);
		LOG("Testing to barr international outgoing\n");
		retval += ofono_call_barring_set((void *)&state, BARRING_OUTCALL, &val);

		g_value_set_string (&val, BARRING_OUT_INT_NOT_HOME);
		LOG("Testing to barr international not home outgoing\n");
		retval += ofono_call_barring_set((void *)&state, BARRING_OUTCALL, &val);

		g_value_set_string (&val, BARRING_IN_ROAMING);
		LOG("Testing to barr incoming when roaming\n");
		retval += ofono_call_barring_set((void *)&state, BARRING_INCALL, &val);

		g_value_set_string (&val, BARRING_IN_ALWAYS);
		LOG("Testing to barr incoming calls\n");
		retval += ofono_call_barring_set((void *)&state, BARRING_INCALL, &val);

		remove_signal_callbacks(&state);

		g_object_unref(state.proxy);
		state.proxy = NULL;
	}


	return retval;
}

/* Signal callback setup/cleanup */
static void setup_signal_callbacks(struct call_barring_state* state)
{
	state->signalcb_CallBarring_PropertyChanged =
			G_CALLBACK(on_call_barring_property_changed);
	dbus_g_proxy_add_signal(state->proxy, "PropertyChanged",
			G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->proxy, "PropertyChanged",
			state->signalcb_CallBarring_PropertyChanged, state, 0);

	state->signalcb_CallBarring_IncomingBarringInEffect =
			G_CALLBACK(on_call_barring_incoming_barring_in_effect);
	dbus_g_proxy_add_signal(state->proxy, "IncomingBarringInEffect",
			G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->proxy, "IncomingBarringInEffect",
			state->signalcb_CallBarring_IncomingBarringInEffect, state, 0);

	state->signalcb_CallBarring_OutgoingBarringInEffect =
			G_CALLBACK(on_call_barring_outgoing_barring_in_effect);
	dbus_g_proxy_add_signal(state->proxy, "OutgoingBarringInEffect",
			G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->proxy, "OutgoingBarringInEffect",
			state->signalcb_CallBarring_OutgoingBarringInEffect, state, 0);
}

static void remove_signal_callbacks(struct call_barring_state* state)
{
	dbus_g_proxy_disconnect_signal(state->proxy, "PropertyChanged",
			state->signalcb_CallBarring_PropertyChanged, state);
	dbus_g_proxy_disconnect_signal(state->proxy, "IncomingBarringInEffect",
			state->signalcb_CallBarring_IncomingBarringInEffect, state);
	dbus_g_proxy_disconnect_signal(state->proxy, "OutgoingBarringInEffect",
			state->signalcb_CallBarring_OutgoingBarringInEffect, state);
}

/* Timeout callback */
static gboolean call_barring_timeout(gpointer user_data)
{
	struct call_barring_state* state = (struct call_barring_state*)user_data;
	state->result = -1;

	LOG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->ofono_data->mainloop);

	return FALSE;
}

/* Signal callbacks */
static void on_call_barring_property_changed(
	__attribute__((unused))DBusGProxy *proxy,
	char *key, GValue* value, gpointer user_data)
{
	struct call_barring_state* state = (struct call_barring_state*)user_data;
	const char* value_str = g_value_get_string(value);
	const char* expected_str = g_value_get_string(state->expected_value);

	if (strcmp(key, state->expected_key) != 0 || strcmp(value_str, expected_str) != 0)
	{
		LOG("Expected key '%s' value '%s' for property, got key '%s' value '%s'\n",
				state->expected_key, expected_str, key, value_str);
		state->result = -1;
	}
	else
	{
		LOG("Call barring property '%s' changed to '%s'\n", key, value_str);
		state->result = 0;
	}

	g_main_loop_quit(state->ofono_data->mainloop);
}

static void on_call_barring_incoming_barring_in_effect(
	__attribute__((unused))DBusGProxy* proxy,
	__attribute__((unused))gpointer user_data)
{
	/* NOTE: This signal was never received while testing. Either oFono does
	   not send it yet, or we have wrong expectations about its delivery. */
	LOG("Incoming barring in effect\n");
}

static void on_call_barring_outgoing_barring_in_effect(
	__attribute__((unused))DBusGProxy* proxy,
	__attribute__((unused))gpointer user_data)
{
	/* NOTE: This signal was never received while testing. Either oFono does
	   not send it yet, or we have wrong expectations about its delivery. */
	LOG("Outgoing barring in effect\n");
}

//variable data seeds
/* Convert generated parameters to test case format. */
void *barring_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *old_pin = 0, *new_pin = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	old_pin = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	new_pin = strdup(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->old_pin)
		free(old_pin);
	else
		data->old_pin = old_pin;

	if (data->new_pin)
		free(new_pin);
	else
		data->new_pin = new_pin;

	return data;
}
