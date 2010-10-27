/* multiparty-call-cases.c -- Multiparty call cases for blts-ofono

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
#include "dual-call-cases.h"
#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"

typedef gboolean (*call_handle_fp)(gpointer user_ptr);

enum MultiPartyStages
{
	MP_INIT,
	MP_FIRST_CALL_IN,
	MP_SECOND_CALL_IN,
	MP_MULTIPARTY_CREATED,
	MP_THIRD_CALL_IN,
	MP_PRIVATE_CHAT,
	MP_HANGUP,
};

#define MAX_VOICECALLS 	6

struct multipart_call_case_state {
	GMainLoop *mainloop;

	int test_stage;
	DBusGProxy *voice_calls[MAX_VOICECALLS];
	int number_voice_calls;
	DBusGProxy *voice_call_manager;

	GSourceFunc case_begin;
	int call_index;
	my_ofono_data *ofono_data;

	GCallback signalcb_VoiceCallManager_PropertyChanged;
	GCallback signalcb_VoiceCall_PropertyChanged[MAX_VOICECALLS];
	
	GCallback signalcb_VoiceCallManager_CallAdded;
	GCallback signalcb_VoiceCallManager_CallRemoved;
	
	call_handle_fp call_handle_method;

	char* voice_call_path[MAX_VOICECALLS];
	int multiparty_ongoing;

	int retries;
	int user_timeout;
	int result;
};

static void on_voice_call_manager_call_added(DBusGProxy *proxy, gchar* path, GHashTable* properties, gpointer user_data);
static void on_voice_call_manager_call_removed(DBusGProxy *proxy, gchar* path, gpointer user_data);
static void on_voice_call_manager_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data);
static void pending_call_answerable_check_complete(DBusGProxy *proxy, GHashTable *properties, GError *error, gpointer data);
static gboolean pending_call_queue_answerable_check(gpointer data);


static int parse_call_index_from_path(const char* path)
{	
	char* tmp_path = NULL; /* /../voicecallN */
	char* call_name = NULL; /* voicecallN */	
	int call_index = -1;
	
	if(!path)
	{
		LOG("Cannot parse empty path!\n");	
		goto error;
	}
	
	BLTS_TRACE("Parsing call name from call %s\n", path);
	tmp_path = strdup(path);
	
	if(!strtok_r(tmp_path, "/", &call_name))
	{
		LOG("Parsing call name failed!\n");
		goto error;
	}
		
	BLTS_TRACE("Parsing call index from call %s\n", call_name);
	if (sscanf(call_name, "voicecall%d", &call_index) != 1)
	{
		LOG("Parsing call index failed!\n");
		goto error;
	}

	free(tmp_path); 	
	
	return call_index;  	
error:
	if(tmp_path)	
		free(tmp_path); 
	return -1;	
}

static
void hangup_all_calls(struct multipart_call_case_state* state)
{
	GError *error = NULL;
	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		LOG("Cannot hangup calls! - test failed\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
	}
	else
	{
		state->result = 0;
	}
}

/* Completion of async answer call. Retry limited times if failure. */
static void pending_call_holdandanswer_complete(__attribute__((unused)) DBusGProxy *proxy,
	GError *error, gpointer data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	if (error)
	{
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}
	state->number_voice_calls++;

	FUNC_LEAVE();
}

/* Completion of async answer call. Retry limited times if failure. */
static void pending_call_answer_complete(DBusGProxy *proxy,
	GError *error, gpointer data)
{
	struct multipart_call_case_state *state;
	int call_index = 0;
	const char *name = dbus_g_proxy_get_path(proxy);
	call_index = parse_call_index_from_path(name);

	g_assert(data);
	state = (struct multipart_call_case_state *) data;
	state->call_index=call_index;
	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		if (!state->retries)
			goto test_fail;
		BLTS_WARNING("Could not answer call, retrying...\n");
		state->retries--;
		g_timeout_add(500, pending_call_queue_answerable_check, state);
	}
	LOG("call answered\n");
	state->number_voice_calls++;
	return;

test_fail:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}

/* Timer callback for retrying checks whether an incoming call can be answered */
static gboolean pending_call_queue_answerable_check(gpointer data)
{
	struct multipart_call_case_state *state;

	g_assert(data);
	state = (struct multipart_call_case_state *) data;

	org_ofono_VoiceCall_get_properties_async(state->voice_calls[state->call_index],
		pending_call_answerable_check_complete, data);

	return FALSE;
}

/* Check whether an incoming call can be answered. If not, retry later, if yes,
   answer. */
static void pending_call_answerable_check_complete(DBusGProxy *proxy,
		GHashTable *properties, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state;
	int call_index = 0;
	state = (struct multipart_call_case_state *) data;
	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		goto test_fail;
	}
	const char *name = dbus_g_proxy_get_path(proxy);
	call_index = parse_call_index_from_path(name);
	g_assert(data);

	LOG("Call state is...\n");
	g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
	state->call_index=call_index;
	if (g_hash_table_find(properties, (GHRFunc) check_state, "incoming"))
		org_ofono_VoiceCall_answer_async(state->voice_calls[call_index],
			pending_call_answer_complete, state);
	else
		g_timeout_add(500, pending_call_queue_answerable_check, state);

	return;

test_fail:
  FUNC_LEAVE();
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}

static void handle_multiparty_call(gpointer data, gpointer user_data)
{
	FUNC_ENTER();

	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;
	char *index = (char *)data;
	LOG("Call in multiparty call: %s\n", index);
	LOG("Calls total in system: %i\n", state->number_voice_calls);
	FUNC_LEAVE();
}


/* Answer to first call and start listening property changes */
 void handle_incoming_call(gchar* path, GHashTable* properties, gpointer user_data)
{
	FUNC_ENTER();
	int call_index = 0;
	DBusGProxy *call=NULL;
	gpointer incoming = NULL;
	gpointer waiting = NULL;
		
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;
	
	call_index = parse_call_index_from_path((const char*) path);
	LOG("Call index is %i\n", call_index);
	
	// create new proxy for call
	if(!state->voice_calls[call_index])
	{
		call = dbus_g_proxy_new_for_name (state->ofono_data->connection,
				OFONO_BUS, path, OFONO_CALL_INTERFACE);

		if (!call)
		{
			LOG("Cannot get proxy for " OFONO_CALL_INTERFACE "\n");
			goto error;
		}

		state->voice_calls[call_index] = call;
	}
	
	LOG("Search call state...\n");
	g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
	incoming = g_hash_table_find(properties, (GHRFunc) check_state, "incoming");
	
	if(!incoming)
	{
		waiting = g_hash_table_find(properties, (GHRFunc) check_state, "waiting");
		if(waiting)
		{
			LOG("Hold the first call and answer the second call...\n");
			state->test_stage = MP_SECOND_CALL_IN;
			org_ofono_VoiceCallManager_hold_and_answer_async(state->voice_call_manager,
					pending_call_holdandanswer_complete, state);

			if (state->signalcb_VoiceCall_PropertyChanged[call_index])
			{
				dbus_g_proxy_add_signal(state->voice_calls[call_index], "PropertyChanged",
									G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
				dbus_g_proxy_connect_signal(state->voice_calls[call_index], "PropertyChanged",
									state->signalcb_VoiceCall_PropertyChanged[call_index], state, 0);
			}
		}
		return;
	}
	
	if (state->signalcb_VoiceCall_PropertyChanged[call_index])
	{
		dbus_g_proxy_add_signal(state->voice_calls[call_index], "PropertyChanged",
							G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_calls[call_index], "PropertyChanged",
							state->signalcb_VoiceCall_PropertyChanged[call_index], state, 0);
	}

	LOG("Answer the first call...\n");
	state->test_stage = MP_FIRST_CALL_IN;
	state->retries = 10;
	org_ofono_VoiceCall_get_properties_async(state->voice_calls[call_index],
	pending_call_answerable_check_complete, state);

	return;
error:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}


/**
 * Signal handler
 * Handle "CallAdded" signals from voice call manager
 */
static void on_voice_call_manager_call_added(__attribute__((unused))DBusGProxy *proxy, gchar* path, GHashTable* properties, gpointer user_data)
{
	LOG("Call added...%s\n", path);		
	handle_incoming_call(path, properties, user_data);
}

/**
 * Signal handler
 * Handle "CallRemoved" signals from voice call manager
 */
static void on_voice_call_manager_call_removed(__attribute__((unused))DBusGProxy *proxy, gchar* path, gpointer user_data)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;	
	LOG("Call %s removed...\n", path);
		
	if(!state->number_voice_calls && state->test_stage == MP_HANGUP)
		g_main_loop_quit(state->mainloop);			
}

/**
 * Signal handler
 * Check call state and call manager state changes
 */

static void on_voice_call_manager_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;
	if(state)
	{
		LOG("manager property changed\n");
	}
	if(strcmp(key, "Calls") == 0)
	{
		LOG("Call list modification > ");
		if(!value)
		{
			LOG("No calls in system.\n");
		}
		else
		{
			g_ptr_array_foreach(g_value_peek_pointer(value), (GFunc)handle_incoming_call, user_data);
		}
	}
	else if(strcmp(key, "MultipartyCalls") == 0)
	{
		LOG("Call list modification > ");
		if(!value)
		{
			LOG("No calls in system.\n");
		}
		else
		{
			g_ptr_array_foreach(g_value_peek_pointer(value), (GFunc)handle_multiparty_call, user_data);
		}
	}
	else
	{
		char* value_str = g_value_dup_string (value);
		LOG("VoiceCallManager property: %s changed to %s\n ", key, value_str);
		free(value_str);
	}
}


/**
 * Signal handler
 * List voice call changes
 */
static void on_voice_call_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;

	if(strcmp(key, "State") == 0)
	{
		gchar* value_str =g_value_dup_string (value);
		
		if( strcmp(value_str, "disconnected") == 0)
		{
			state->number_voice_calls--;
		}
		
		LOG("Voicecall %s property: '%s' changed to '%s'\n", dbus_g_proxy_get_path(proxy), key, value_str);
		g_free(value_str);
	}

	if(strcmp(key, "Multiparty") == 0)
	{
		gboolean multiparty = g_value_get_boolean(value);
		LOG("Voicecall %s property: '%s' changed to '%d'\n", dbus_g_proxy_get_path(proxy), key, multiparty);
	}
}

/*
 * Put the current call on hold and answers the currently waiting call
 */
gboolean do_multiparty_call(gpointer user_ptr)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_ptr;


	if(state)
		LOG("Multiparty call case ongoing");
	return FALSE;
}


static void call_state_finalize(struct multipart_call_case_state *state)
{
	int i;
	if (!state)
		return;

	for(i=0; i<MAX_VOICECALLS; i++)
	{
		if(state->voice_call_path[i])
				free(state->voice_call_path[i]);
		if (state->voice_calls[i])
				g_object_unref(state->voice_calls[i]);
	}

	if (state->voice_call_manager)
		g_object_unref(state->voice_call_manager);


	free(state);
}

static gboolean call_master_timeout(gpointer data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	state->result = -1;

	LOG("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}

static gboolean call_init_start(gpointer data)
{
	FUNC_ENTER();

	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	DBusGProxy *voice_call_manager;

	voice_call_manager = dbus_g_proxy_new_for_name (state->ofono_data->connection,
											OFONO_BUS,
											state->ofono_data->modem[0],
											OFONO_VC_INTERFACE);

	if (!voice_call_manager) {
		LOG("Cannot get proxy for " OFONO_VC_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	state->voice_call_manager = voice_call_manager;

	/* Ensure that there is no previous calls in system */
	hangup_all_calls(state);

	if (state->signalcb_VoiceCallManager_PropertyChanged) {
			dbus_g_proxy_add_signal(state->voice_call_manager, "PropertyChanged",
					G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state->voice_call_manager, "PropertyChanged",
				state->signalcb_VoiceCallManager_PropertyChanged, data, 0);
		}
	if (state->signalcb_VoiceCallManager_CallAdded) {
		dbus_g_proxy_add_signal(state->voice_call_manager, "CallAdded",
			DBUS_TYPE_G_OBJECT_PATH, dbus_g_type_get_map ("GHashTable", 
			G_TYPE_STRING, G_TYPE_VALUE), G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call_manager, "CallAdded",
			state->signalcb_VoiceCallManager_CallAdded, data, 0);
	}

	if (state->signalcb_VoiceCallManager_CallRemoved) {
		dbus_g_proxy_add_signal(state->voice_call_manager, "CallRemoved",
			DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call_manager, "CallRemoved",
			state->signalcb_VoiceCallManager_CallRemoved, data, 0);
	}
	

	if (state->case_begin)
		g_timeout_add(2000, state->case_begin, state);


	FUNC_LEAVE();
	return FALSE;
}

static struct multipart_call_case_state *call_state_init(my_ofono_data *data)
{
	struct multipart_call_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		LOG("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}

static int call_case_run(struct multipart_call_case_state *state)
{
	int ret=0, i;
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

	state->voice_call_manager = NULL;

	for(i=0; i< MAX_VOICECALLS; i++)
	{
		state->voice_calls[i] = NULL;
		state->voice_call_path[i] = NULL;
	}
	state->number_voice_calls = 0;
	state->multiparty_ongoing = 0;
	state->test_stage = MP_INIT;

	g_timeout_add(120000, (GSourceFunc) call_master_timeout, state);
	g_idle_add(call_init_start, state);

	state->result=-1;

	BLTS_TRACE("Starting main loop.\n");
	g_main_loop_run(state->mainloop);
	BLTS_TRACE("Back from main loop.\n");

	ret = state->result;

done:

	return ret;
}

void pointer_array_foreach(gpointer data,  __attribute__((unused))gpointer user_data)
{
	LOG("\t%s\n", (char *)data);
	return;
}


/**
 * Wait for 2 calls in and create multiparty call
 */
static gboolean call_listen_start(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();
	GPtrArray* call_list = NULL;
	GError *error = NULL;
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	LOG("Waiting for two calls...\nCalls currently in system: %i\n", state->number_voice_calls);
	if(state->number_voice_calls == 2 && state->multiparty_ongoing)
	{
		/* Hang up multiparty */
		if(org_ofono_VoiceCallManager_hangup_multiparty (state->voice_call_manager, &error))
		{
			LOG("Multiparty hanged up\n");
			state->multiparty_ongoing = 0;
			state->result = 0;
			state->test_stage = MP_HANGUP;
			return FALSE;
		}
		if(error)
			goto error;
	}

	if(state->number_voice_calls == 2 && !state->multiparty_ongoing)
	{
		/* Create multiparty when we have 2 calls in */
		LOG("Creating multiparty call\n");
		if(org_ofono_VoiceCallManager_create_multiparty (state->voice_call_manager, &call_list, &error))
		{
			LOG("Multiparty call created\n");
			state->multiparty_ongoing = 1;
			state->test_stage = MP_MULTIPARTY_CREATED;
			LOG("Calls in multiparty call:\n");
			g_ptr_array_foreach(call_list, pointer_array_foreach, NULL);
			g_ptr_array_free(call_list, TRUE);
			return TRUE;
		}
		if(error)
			goto error;
	}

	FUNC_LEAVE();
	return TRUE;
	
error:
	display_dbus_glib_error(error);
	g_error_free (error);		
	state->result = -1;
	g_main_loop_quit(state->mainloop);	
	FUNC_LEAVE();	
	return FALSE;	
}


/**
 * Wait for 3 calls in and create multiparty call and then separate first
 * call to private chat
 */
static gboolean call_listen_start_private(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();
	GPtrArray* call_list = NULL;
	GError *error = NULL;
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	LOG("Waiting for three calls...\nCalls currently in system: %i\n", state->number_voice_calls);
	if(state->number_voice_calls == 3 && state->multiparty_ongoing && state->test_stage == MP_MULTIPARTY_CREATED)
	{
		// Create multiparty with 3rd call
		LOG("Creating multiparty call\n");
		if(org_ofono_VoiceCallManager_create_multiparty (state->voice_call_manager, &call_list, &error))
		{
			LOG("Multiparty call created\n");
			state->multiparty_ongoing = 1;
			LOG("Calls in multiparty call:\n");
			g_ptr_array_foreach(call_list, pointer_array_foreach, NULL);
			g_ptr_array_free(call_list, TRUE);
			state->test_stage = MP_THIRD_CALL_IN;
			return TRUE;
		}
		if(error)
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}
	}

	if(state->number_voice_calls == 3 && state->multiparty_ongoing && state->test_stage == MP_THIRD_CALL_IN)
	{
		// Create private chat
		LOG("Creating private chat with first call\n");
		if(org_ofono_VoiceCallManager_private_chat(state->voice_call_manager, "/phonesim/voicecall01", &call_list, &error))
		{
			LOG("Private chat created\n");
			LOG("Calls in private chat:\n");
			g_ptr_array_foreach(call_list, pointer_array_foreach, NULL);
			g_ptr_array_free(call_list, TRUE);
			state->test_stage = MP_PRIVATE_CHAT;
			return TRUE;
		}
		if(error)
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}
	}

	if(state->number_voice_calls == 2 && !state->multiparty_ongoing)
	{
		// Create multiparty when we have 2 calls in
		LOG("Creating multiparty call\n");
		if(org_ofono_VoiceCallManager_create_multiparty (state->voice_call_manager, &call_list, &error))
		{
			LOG("Multiparty call created\n");
			state->multiparty_ongoing = 1;
			LOG("Calls in multiparty call:\n");
			g_ptr_array_foreach(call_list, pointer_array_foreach, NULL);
			g_ptr_array_free(call_list, TRUE);
			state->test_stage = MP_MULTIPARTY_CREATED;
			return TRUE;
		}
		if(error)
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}

	}

	if(state->number_voice_calls == 3 && state->test_stage == MP_PRIVATE_CHAT)
	{
		// hang up all after private chat has been established
		if(org_ofono_VoiceCallManager_hangup_all (state->voice_call_manager, &error))
		{
			LOG("All calls hanged up\n");
			state->multiparty_ongoing = 0;
			state->result = 0;
			g_main_loop_quit(state->mainloop);
			return FALSE;
		}
		if(error)
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}
	}

	FUNC_LEAVE();
	return TRUE;
}

/* Test cases ------------> */

int blts_ofono_case_multiparty(void* user_ptr, __attribute__((unused))int test_num)
{
	int i, ret;
	struct multipart_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	//log_set_level(5);

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 60000;

	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	for(i=0; i<MAX_VOICECALLS; i++)
	{
		test->signalcb_VoiceCall_PropertyChanged[i] =
			G_CALLBACK(on_voice_call_property_changed);
	}

	test->call_handle_method = do_multiparty_call;

	test->case_begin = (GSourceFunc) call_listen_start;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}


int blts_ofono_case_private_chat(void* user_ptr, __attribute__((unused))int test_num)
{
	int i, ret;
	struct multipart_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	//log_set_level(5);

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 60000;

	test->signalcb_VoiceCallManager_PropertyChanged =
		G_CALLBACK(on_voice_call_manager_property_changed);
	for(i=0; i<MAX_VOICECALLS; i++)
	{
		test->signalcb_VoiceCall_PropertyChanged[i] =
			G_CALLBACK(on_voice_call_property_changed);
	}

	test->call_handle_method = do_multiparty_call;

	test->case_begin = (GSourceFunc) call_listen_start_private;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

