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
	MP_UNDEFINED = -1,
	MP_INIT,
	MP_MULTIPARTY_CREATED1,
	MP_MULTIPARTY_CREATED2,	
	MP_PRIVATE_CHAT,
	MP_HANGUP,
	MP_LAST
};

#define MAX_VOICECALLS 	6
#define PRIVATE_CHAT_CALL_INDEX 0

typedef gboolean (*call_hangup_fp)(gpointer user_ptr);

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
	
	call_hangup_fp call_hangup_method;

	char* voice_call_path[MAX_VOICECALLS];

	int retries;
	int user_timeout;
	int result;
};

static void on_voice_call_manager_call_added(DBusGProxy *proxy, gchar* path, GHashTable* properties, gpointer user_data);
static void on_voice_call_manager_call_removed(DBusGProxy *proxy, gchar* path, gpointer user_data);
static void pending_call_answerable_check_complete(DBusGProxy *proxy, GHashTable *properties, GError *error, gpointer data);
static gboolean pending_call_queue_answerable_check(gpointer data);

static void on_voice_call_manager_create_multiparty_reply(__attribute__((unused))DBusGProxy *proxy, GPtrArray *calls, GError *error, void *data);
static void on_voice_call_manager_private_chat_reply (__attribute__((unused))DBusGProxy *proxy, GPtrArray *calls, GError *error, void *data);

char* get_multiparty_stage_str(enum MultiPartyStages stage);
void multiparty_stage_add_call(struct multipart_call_case_state* state, gchar* path);
void multiparty_stage_remove_call(struct multipart_call_case_state* state);

static int check_call_count(struct multipart_call_case_state *state, int expected_count);

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

static gboolean do_hangup_all(gpointer user_ptr)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Hanging up all calls--\n");
	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		state->result = -1;
		g_main_loop_quit(state->mainloop);		
		return TRUE;
	}
	
	state->result = 0;
	state->test_stage = MP_HANGUP;		

	return FALSE;
}

static gboolean do_hangup_multiparty(gpointer user_ptr)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Hanging up multiparty calls--\n");
	if(!org_ofono_VoiceCallManager_hangup_multiparty(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		state->result = -1;
		g_main_loop_quit(state->mainloop);		
		return TRUE;
	}
	
	state->result = 0;
	state->test_stage = MP_HANGUP;		

	return FALSE;
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
	FUNC_LEAVE();
	
}

/* Completion of async answer call. Retry limited times if failure. */
static void pending_call_answer_complete(DBusGProxy *proxy,
	GError *error, gpointer data)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;
	int call_index = 0;
	const char *name = dbus_g_proxy_get_path(proxy);
	call_index = parse_call_index_from_path(name);
	if(call_index < 0)
		goto test_fail;
	
	g_assert(data);
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
	if(call_index < 0)
		goto test_fail;	

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
	if(call_index < 0)
		goto error;
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
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) user_data;	
	multiparty_stage_add_call(state, path);
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
	multiparty_stage_remove_call(state);
		
	if(!state->number_voice_calls && state->test_stage == MP_HANGUP)
		g_main_loop_quit(state->mainloop);			
}


/**
 * Signal handler
 * List voice call changes
 */
static void on_voice_call_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{
	FUNC_ENTER();
	(void)user_data;

	if(strcmp(key, "State") == 0)
	{
		gchar* value_str =g_value_dup_string (value);				
		LOG("Voicecall %s property: '%s' changed to '%s'\n", dbus_g_proxy_get_path(proxy), key, value_str);
		g_free(value_str);
	}

	if(strcmp(key, "Multiparty") == 0)
	{
		
		gboolean multiparty = g_value_get_boolean(value);
		LOG("Voicecall %s property: '%s' changed to '%d'\n", dbus_g_proxy_get_path(proxy), key, multiparty);
	}
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

static gboolean call_user_timeout(gpointer data)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	state->result = 0;

	LOG("Own Timeout reached\n");

	if (state->call_hangup_method)
	{
		LOG("Hang up calls...\n");
		if (state->call_hangup_method(state))
		{
			state->result = -1;
		}
		else
			state->result = 0;
	}
	return FALSE;
}

gboolean check_call_count(struct multipart_call_case_state *state, int expected_count)
{
	GPtrArray* array;
	GError *error;
	int count;
	
	if(!org_ofono_VoiceCallManager_get_calls(state->voice_call_manager, &array, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return FALSE;
	}
	else
	{
		if(!array)
		{
			count = 0;
		}
		else
		{	
			int i;		
			for (i = 0; i < (int) array->len; i++)
			{
				GValueArray *call = g_ptr_array_index (array, i);
				char *path = g_value_get_boxed (g_value_array_get_nth (call, 0));
				GHashTable *properties = g_value_get_boxed (g_value_array_get_nth (call, 1));
						
				BLTS_TRACE("%s\n", path);										
				g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
			}
			count = i;	
		}
	}
	
	BLTS_TRACE("Expected call count:%d - current call count:%d\n", expected_count, count);
	if(expected_count != -1)
		if(expected_count != count)
			return FALSE;
	
	return TRUE;	
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
	
	if(!check_call_count(state, 0)) {
		LOG("Previous calls left in system! - test failed\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
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

char* get_multiparty_stage_str(enum MultiPartyStages stage)
{
	switch(stage)
	{
		case MP_UNDEFINED: return "MP_UNDEFINED"; 	
		case MP_INIT: return "MP_INIT"; 	
		case MP_MULTIPARTY_CREATED1: return "MP_MULTIPARTY_CREATED1"; 	
		case MP_MULTIPARTY_CREATED2: return "MP_MULTIPARTY_CREATED2"; 	
		case MP_PRIVATE_CHAT: return "MP_PRIVATE_CHAT"; 	
		case MP_HANGUP: return "MP_HANGUP";
		default:
		return "undefined"; 	
	}
}

void multiparty_stage_add_call(struct multipart_call_case_state* state, gchar* path)
{
	if(state->number_voice_calls + 1 > MAX_VOICECALLS)
	{
		BLTS_WARNING("Too many voice calls in system, cannot add more...\n");
		return;	
	}
	
	if(state->voice_call_path[state->number_voice_calls] != NULL)
	{
		g_free(state->voice_call_path[state->number_voice_calls]);
		state->voice_call_path[state->number_voice_calls] = NULL;
		g_object_unref(state->voice_calls[state->number_voice_calls]);
		state->voice_calls[state->number_voice_calls] = NULL;
	}
		
	state->voice_call_path[state->number_voice_calls++] = g_strdup(path);
	LOG("Calls in system now: %i\n", state->number_voice_calls);
}

void multiparty_stage_remove_call(struct multipart_call_case_state* state)
{
	if(state->number_voice_calls - 1 < 0)
	{
		BLTS_WARNING("No voice calls in system, cannot remove...\n");
		return;	
	}

	if(state->voice_call_path[state->number_voice_calls] != NULL)
	{
		g_free(state->voice_call_path[state->number_voice_calls]);
		state->voice_call_path[state->number_voice_calls] = NULL;
		g_object_unref(state->voice_calls[state->number_voice_calls]);
		state->voice_calls[state->number_voice_calls] = NULL;
	}	
	
	state->number_voice_calls--;
	LOG("Calls in system now: %i\n", state->number_voice_calls);
}

void on_voice_call_manager_create_multiparty_reply(__attribute__((unused))DBusGProxy *proxy, GPtrArray *calls, GError *error, void *data)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;

	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		BLTS_WARNING("Could not create multiparty call!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);		
	}

	LOG("Calls in multiparty call:\n");
	g_ptr_array_foreach(calls, pointer_array_foreach, NULL);
	g_ptr_array_free(calls, TRUE);

	/* update to the next stage */
	if(state->test_stage == MP_INIT)
		state->test_stage = MP_MULTIPARTY_CREATED1;
	else if	(state->test_stage == MP_MULTIPARTY_CREATED1)
		state->test_stage = MP_MULTIPARTY_CREATED2;
	else
		state->test_stage = MP_UNDEFINED;	
}

void on_voice_call_manager_private_chat_reply (__attribute__((unused))DBusGProxy *proxy, GPtrArray *calls, GError *error, void *data)
{
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;
	
	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		BLTS_WARNING("Could not create private chat!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);		
	}	
	
	LOG("Calls in private chat:\n");
	g_ptr_array_foreach(calls, pointer_array_foreach, NULL);
	g_ptr_array_free(calls, TRUE);
	
	state->test_stage = MP_PRIVATE_CHAT;	
}

/**
 * Wait for 2 calls in and create multiparty call
 */
static gboolean call_listen_start(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;
	const char* stage_str = get_multiparty_stage_str(state->test_stage);

	if(state->number_voice_calls < 1)
		return TRUE;
	
	if(state->number_voice_calls == 2 && state->test_stage == MP_INIT)
	{
		/* Create multiparty when we have 2 calls in */
		LOG("%s: Creating multiparty call\n", stage_str);		
		org_ofono_VoiceCallManager_create_multiparty_async (state->voice_call_manager, 
		on_voice_call_manager_create_multiparty_reply, state);
	}
	else if(state->number_voice_calls == 2 && state->test_stage == MP_MULTIPARTY_CREATED1)
	{
		/* Hang up multiparty */
		LOG("%s: Hang up multiparty call\n", stage_str);
		if (state->call_hangup_method)
		{
			g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);
			return FALSE;
		}		
		else
		{
			BLTS_WARNING("Hangup method undefined - test failed!\n");
			state->result = -1;
			g_main_loop_quit(state->mainloop);			
			return FALSE;
		}		
	}	
	else
	{
		LOG("%s: Waiting for two calls...\nCalls currently in system: %i\n", 
		stage_str, state->number_voice_calls);
	}

	FUNC_LEAVE();
	return TRUE;
}

/**
 * Wait for 3 calls in and create multiparty call and then separate first
 * call to private chat
 */
static gboolean call_listen_start_private(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();	
	struct multipart_call_case_state *state = (struct multipart_call_case_state *) data;
	const char* stage_str = get_multiparty_stage_str(state->test_stage); 

	if(state->number_voice_calls < 2)
		return TRUE;
	
	if(state->number_voice_calls == 2 && state->test_stage == MP_INIT)
	{
		/* Create multiparty when we have 2 calls in */
		LOG("%s: Creating multiparty call phase 1\n", stage_str);		
		org_ofono_VoiceCallManager_create_multiparty_async (state->voice_call_manager, 
		on_voice_call_manager_create_multiparty_reply, state);
	}
		
	else if(state->number_voice_calls == 3 && state->test_stage == MP_MULTIPARTY_CREATED1)
	{
		/* Create multiparty with 3rd call */
		LOG("%s: Creating multiparty call phase 2\n", stage_str);
		org_ofono_VoiceCallManager_create_multiparty_async (state->voice_call_manager, 
		on_voice_call_manager_create_multiparty_reply, state);
	}

	else if(state->number_voice_calls == 3 && state->test_stage == MP_MULTIPARTY_CREATED2)
	{
		/* Create private chat */
		LOG("%s: Creating private chat with first call\n", stage_str);
		org_ofono_VoiceCallManager_private_chat_async (state->voice_call_manager, 
		(const char*)state->voice_call_path[PRIVATE_CHAT_CALL_INDEX], 
		on_voice_call_manager_private_chat_reply, state);
	}

	else if(state->number_voice_calls == 3 && MP_PRIVATE_CHAT)
	{
		/* Hang up all calls */
		LOG("%s: Hang up all after private chat has been established\n", stage_str);

		if (state->call_hangup_method)
		{
			g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);
			return FALSE;
		}		
		else
		{
			BLTS_WARNING("Hangup method undefined - test failed!\n");
			state->result = -1;
			g_main_loop_quit(state->mainloop);			
			return FALSE;
		}
	}
	else
	{
		LOG("%s: Waiting for three calls...\nCalls currently in system: %i\n", 
		stage_str, state->number_voice_calls);	
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

 	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;
	test->call_hangup_method = do_hangup_multiparty;

	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	for(i=0; i<MAX_VOICECALLS; i++)
	{
		test->signalcb_VoiceCall_PropertyChanged[i] =
			G_CALLBACK(on_voice_call_property_changed);
	}

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

 	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;
	test->call_hangup_method = do_hangup_all;

	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);
		
	for(i=0; i<MAX_VOICECALLS; i++)
	{
		test->signalcb_VoiceCall_PropertyChanged[i] =
			G_CALLBACK(on_voice_call_property_changed);
	}

	test->case_begin = (GSourceFunc) call_listen_start_private;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

