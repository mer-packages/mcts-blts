/* call-volume-cases.c -- Call volume cases for blts-ofono

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
#include <blts_log.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdlib.h>
#include <glib-object.h>
#include <unistd.h>
#include "call-volume-cases.h"
#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"
#include "ofono-general.h"
#include "ofono-util.h"

#define VOLUME_STATE_SLEEP 5

struct call_volume_case_state {
	char* address;

	char* initial_microphone_volume;
	char* initial_speaker_volume;
	char* initial_mute_state;

	char* new_microphone_volume;
	char* new_speaker_volume;

	GMainLoop *mainloop;

	DBusGProxy *call_volume;
	DBusGProxy *voice_call_manager;

	GSourceFunc case_begin;

	my_ofono_data *ofono_data;

	GCallback signalcb_VoiceCallManager_PropertyChanged;
	GCallback signalcb_CallVolume_PropertyChanged;

	int result;
};

static gboolean save_initial_values(struct call_volume_case_state *state);
static gboolean restore_initial_value(struct call_volume_case_state *state, char* key);

GHashTable* get_call_volume_properties(DBusGProxy *proxy)
{
	GError *error = NULL;
	GHashTable* list = NULL;

	if(!proxy)
		return NULL;

	if(!org_ofono_CallVolume_get_properties(proxy, &list, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return NULL;
	}
	return list;
}

static void
get_initial_value (gpointer key, gpointer value, gpointer data)
{
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if(!state)
		return;

	if(strcmp(key, "MicrophoneVolume") == 0)
	{
		state->initial_microphone_volume = g_strdup_value_contents (value);
		LOG("Save initial MicrophoneVolume %s (%s)\n", state->initial_microphone_volume, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "SpeakerVolume") == 0)
	{
		state->initial_speaker_volume = g_strdup_value_contents (value);
		LOG("Save initial SpeakerVolume %s (%s)\n", state->initial_speaker_volume, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "Muted") == 0)
	{
		state->initial_mute_state =  g_strdup_value_contents (value);
		LOG("Save initial Muted %s (%s)\n", state->initial_mute_state, G_VALUE_TYPE_NAME(value));
	}
	else
	{
		LOG("new property?\n");
	}
}

/**
 * Signal handler
 * Can be used to check if call volume state changes
 */

static void on_call_volume_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{
	gboolean restored = TRUE;
	GError *error = NULL;
	struct call_volume_case_state *state = (struct call_volume_case_state *) user_data;

	LOG("CallVolume property: %s changed to %s\n ", key, g_strdup_value_contents (value));

	if (state->signalcb_CallVolume_PropertyChanged)
	{
		dbus_g_proxy_disconnect_signal(state->call_volume, "PropertyChanged",
		state->signalcb_CallVolume_PropertyChanged, user_data);
	}

	/* sleep here before original volume is set */
	sleep(VOLUME_STATE_SLEEP);

	if(!restore_initial_value(state, key))
	{
		LOG("Restore initial value failed!\n");
		restored = FALSE;
	}

	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		state->result = -1;
	}
	else
	{
		if(restored)
			state->result = 0;
		else
			state->result = -1;
	}

	g_main_loop_quit(state->mainloop);
}

static void call_volume_state_finalize(struct call_volume_case_state *state)
{
	if (!state)
		return;

	if (state->call_volume)
		g_object_unref(state->call_volume);

	if (state->voice_call_manager)
		g_object_unref(state->voice_call_manager);

	if (state->address)
		free(state->address);

	if (state->initial_microphone_volume)
		free(state->initial_microphone_volume);

	if (state->initial_speaker_volume)
		free(state->initial_speaker_volume);

	if (state->initial_mute_state)
		free(state->initial_mute_state);

	free(state);
}

static gboolean call_master_timeout(gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	state->result = -1;

	LOG("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}

static gboolean call_volume_init_start(gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	DBusGProxy *call_volume;
	DBusGProxy *voice_call_manager;

	voice_call_manager = dbus_g_proxy_new_for_name (state->ofono_data->connection,
											OFONO_BUS,
											state->ofono_data->modem[0],
											OFONO_VC_INTERFACE);

	if (!voice_call_manager){
		LOG("Cannot get proxy for " OFONO_VC_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}
	state->voice_call_manager = voice_call_manager;

	call_volume = dbus_g_proxy_new_for_name (state->ofono_data->connection,
											OFONO_BUS,
											state->ofono_data->modem[0],
											OFONO_CV_INTERFACE);

	if (!call_volume) {
		LOG("Cannot get proxy for " OFONO_CV_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	state->call_volume = call_volume;

	if (state->signalcb_VoiceCallManager_PropertyChanged) {
			dbus_g_proxy_add_signal(state->voice_call_manager, "PropertyChanged",
					G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state->voice_call_manager, "PropertyChanged",
				state->signalcb_VoiceCallManager_PropertyChanged, data, 0);
		}

	if (state->signalcb_CallVolume_PropertyChanged) {
				dbus_g_proxy_add_signal(state->call_volume, "PropertyChanged",
						G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
				dbus_g_proxy_connect_signal(state->call_volume, "PropertyChanged",
					state->signalcb_CallVolume_PropertyChanged, data, 0);
			}

	if (state->case_begin)
		g_idle_add(state->case_begin, state);

	FUNC_LEAVE();
	return FALSE;
}

static void set_microphone_volume_complete(__attribute__((unused)) DBusGProxy *proxy, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		log_print("Set MicrophoneVolume failure:  %s\n", error->message);
		state->result=-1;
		g_main_loop_quit(state->mainloop);
	}

	if(strcmp(state->initial_microphone_volume, state->new_microphone_volume) == 0 )
	{
		log_print("Initial value is equal with new value\n");
		state->result=0;
		g_main_loop_quit(state->mainloop);
	}

	FUNC_LEAVE();
}

static void set_speaker_volume_complete(__attribute__((unused)) DBusGProxy *proxy, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		log_print("Set SpeakerVolume failure:  %s\n", error->message);
		state->result=-1;
		g_main_loop_quit(state->mainloop);
	}

	if(strcmp(state->initial_speaker_volume, state->new_speaker_volume) == 0 )
	{
		log_print("Initial value is equal with new value\n");
		state->result=0;
		g_main_loop_quit(state->mainloop);
	}

	FUNC_LEAVE();
}

static void toggle_mute_state_complete(__attribute__((unused)) DBusGProxy *proxy, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		log_print("Toggle Muted failure:  %s\n", error->message);
		state->result=-1;
		g_main_loop_quit(state->mainloop);
	}

	FUNC_LEAVE();
}

static gboolean save_initial_values(struct call_volume_case_state *state)
{
	FUNC_ENTER();
	GHashTable* properties;

	if(!state)
		return FALSE;

	properties = get_call_volume_properties(state->call_volume);

	if(!properties)
		return FALSE;

	LOG("Get initial values\n");
	g_hash_table_foreach(properties, (GHFunc)get_initial_value, state);

	g_hash_table_destroy(properties);
	properties = NULL;

	return TRUE;
}

static gboolean restore_initial_value(struct call_volume_case_state *state, char* key)
{
	FUNC_ENTER();
	GValue *initial_value;

	if(!state)
		return FALSE;

	initial_value = malloc(sizeof *(initial_value));
	memset(initial_value, 0, sizeof *(initial_value));

	if(strcmp(key, "MicrophoneVolume") == 0)
	{
		LOG("Restore initial MicrophoneVolume %s\n", state->initial_microphone_volume);
		g_value_init(initial_value, G_TYPE_UCHAR);
		g_value_set_uchar(initial_value, atoi(state->initial_microphone_volume));

		org_ofono_CallVolume_set_property(state->call_volume,
				"MicrophoneVolume", initial_value, NULL);
	}
	else if (strcmp(key, "SpeakerVolume") == 0)
	{
		LOG("Restore initial SpeakerVolume %s\n", state->initial_speaker_volume);
		g_value_init(initial_value, G_TYPE_UCHAR);
		g_value_set_uchar(initial_value, atoi(state->initial_speaker_volume));

		org_ofono_CallVolume_set_property(state->call_volume,
				"SpeakerVolume", initial_value, NULL);
	}
	else if (strcmp(key, "Muted") == 0)
	{
		LOG("Restore initial Muted %s\n", state->initial_mute_state);
		gboolean restored_state;
		if (strcmp(state->initial_mute_state, "TRUE") == 0)
			restored_state = TRUE;
		else
			restored_state = FALSE;

		g_value_init(initial_value, G_TYPE_BOOLEAN);
		g_value_set_boolean(initial_value, restored_state);

		org_ofono_CallVolume_set_property(state->call_volume,
				"Muted", initial_value, NULL);

	}
	else
	{
			LOG("new property?\n");
			return FALSE;
	}

	free(initial_value);
	return TRUE;
}

static void microphone_volume_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();
	GValue *new_value;
	int changed_value;
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	/* sleep here before new volume is set */
	sleep(VOLUME_STATE_SLEEP);

	new_value = malloc(sizeof *(new_value));
	memset(new_value, 0, sizeof *(new_value));
	g_value_init(new_value, G_TYPE_UCHAR);

	/* if volume is not given as parameter, we must use initial value as reference */
	if (strcmp(state->new_microphone_volume, "null") == 0)
	{
		int initial = atoi(state->initial_microphone_volume);

		LOG("initial value: %d\n", initial);

		if(initial < 50)
			changed_value=75;
		else
			changed_value=25;

		g_value_set_uchar(new_value, changed_value);
	}
	else
	{
		changed_value = atoi(state->new_microphone_volume);
		g_value_set_uchar(new_value, changed_value);
	}

	LOG("changed value: %d\n", changed_value);
	org_ofono_CallVolume_set_property_async (state->call_volume,
			"MicrophoneVolume", new_value, set_microphone_volume_complete, state);

	free(new_value);
}
static void speaker_volume_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();
	GValue *new_value;
	int changed_value;
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	/* sleep here before new volume is set */
	sleep(VOLUME_STATE_SLEEP);

	new_value = malloc(sizeof *(new_value));
	memset(new_value, 0, sizeof *(new_value));
	g_value_init(new_value, G_TYPE_UCHAR);

	/* if volume is not given as parameter, we must use initial value as reference */
	if (strcmp(state->new_speaker_volume, "null") == 0)
	{
		int initial = atoi(state->initial_speaker_volume);

		LOG("initial value: %d\n", initial);

		if(initial < 50)
			changed_value=75;
		else
			changed_value=25;

		g_value_set_uchar(new_value, changed_value);
	}
	else
	{
		changed_value = atoi(state->new_speaker_volume);
		g_value_set_uchar(new_value, changed_value);
	}

	LOG("changed value: %d\n", changed_value);

	org_ofono_CallVolume_set_property_async (state->call_volume,
			"SpeakerVolume", new_value, set_speaker_volume_complete, state);
	free(new_value);

	FUNC_LEAVE();
}

static void mute_state_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();
	GValue *new_value;
	gboolean new_state;

	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	if (strcmp(state->initial_mute_state, "TRUE") == 0)
		new_state = FALSE;
	else
		new_state = TRUE;

	new_value = malloc(sizeof *(new_value));
	memset(new_value, 0, sizeof *(new_value));
	g_value_init(new_value, G_TYPE_BOOLEAN);

	g_value_set_boolean(new_value, new_state);

	LOG("new state %d\n", new_state);

	org_ofono_CallVolume_set_property_async (state->call_volume,
			"Muted", new_value, toggle_mute_state_complete, state);

	free(new_value);
	FUNC_LEAVE();
}


static gboolean set_microphone_volume_start(gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", microphone_volume_call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
}

static gboolean set_speaker_volume_start(gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", speaker_volume_call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
}

static gboolean toggle_mute_state_start(gpointer data)
{
	FUNC_ENTER();
	struct call_volume_case_state *state = (struct call_volume_case_state *) data;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", mute_state_call_complete, state);

	FUNC_LEAVE();
	return FALSE;
}

static struct call_volume_case_state *call_volume_state_init(my_ofono_data *data)
{
	struct call_volume_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		log_print("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}


static int call_volume_case_run(struct call_volume_case_state *state)
{
	int ret;
	BLTS_TRACE("Getting modem:\n");

	ret = my_ofono_get_modem(state->ofono_data);
	if (ret) {
		LOG("Failed getting modem.\n");
		goto done;
	}
	if (state->ofono_data->number_modems < 1) {
		LOG("No modems available.\n");
		ret = -1;
		goto done;
	}
	BLTS_TRACE("Modem ok.\n");
	state->mainloop = state->ofono_data->mainloop;

	state->call_volume = NULL;
	state->voice_call_manager = NULL;

	g_timeout_add(60000, (GSourceFunc) call_master_timeout, state);

	g_idle_add(call_volume_init_start, state);

	state->result=-1;

	BLTS_TRACE("Starting main loop.\n");
	g_main_loop_run(state->mainloop);
	BLTS_TRACE("Back from main loop.\n");

	ret = state->result;

done:

	return ret;
}

/* Test cases ------------> */

int blts_ofono_set_microphone_volume(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_volume_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	test = call_volume_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->new_microphone_volume = strdup((data->volume) ? (data->volume) : "null");
	test->signalcb_VoiceCallManager_PropertyChanged = NULL;
	test->signalcb_CallVolume_PropertyChanged =
		G_CALLBACK(on_call_volume_property_changed);

	test->case_begin = (GSourceFunc) set_microphone_volume_start;
	ret = call_volume_case_run(test);

	call_volume_state_finalize(test);
	return ret;
}

int blts_ofono_set_speaker_volume(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_volume_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	test = call_volume_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->new_speaker_volume = strdup((data->volume) ? (data->volume) : "null");
	test->signalcb_VoiceCallManager_PropertyChanged = NULL;
	test->signalcb_CallVolume_PropertyChanged =
		G_CALLBACK(on_call_volume_property_changed);

	test->case_begin = (GSourceFunc) set_speaker_volume_start;
	ret = call_volume_case_run(test);

	call_volume_state_finalize(test);
	return ret;
}
int blts_ofono_set_muted(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_volume_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	test = call_volume_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->signalcb_VoiceCallManager_PropertyChanged = NULL;
	test->signalcb_CallVolume_PropertyChanged =
		G_CALLBACK(on_call_volume_property_changed);

	test->case_begin = (GSourceFunc) toggle_mute_state_start;
	ret = call_volume_case_run(test);
	call_volume_state_finalize(test);
	return ret;
}

/* Convert generated parameters to test case format. */
void *volume_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *volume = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	volume = strdup(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->volume)
		free(volume);
	else
		data->volume = volume;

	return data;
}
