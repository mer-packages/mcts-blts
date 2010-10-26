/* dual-call-cases.c -- Dual voicecall cases for blts-ofono

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

#define FIRST_CALL 		0
#define SECOND_CALL 	1

struct dual_call_case_state {
	GMainLoop *mainloop;

	DBusGProxy *voice_calls[2];
	DBusGProxy *voice_call_manager;

	GSourceFunc case_begin;

	my_ofono_data *ofono_data;

	GCallback signalcb_VoiceCall1_PropertyChanged;
	GCallback signalcb_VoiceCall2_PropertyChanged;

	GCallback signalcb_VoiceCallManager_CallAdded;
	GCallback signalcb_VoiceCallManager_CallRemoved;

	call_handle_fp call_handle_method;

	gchar* first_voice_call_path;
	gchar* second_voice_call_path;

	int retries;
	int user_timeout;
	int result;
};

static void on_first_voice_call_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data);
static void on_second_voice_call_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data);

static void on_voice_call_manager_call_added(DBusGProxy *proxy, gchar* path, GHashTable* properties, void *user_data);
static void on_voice_call_manager_call_removed(DBusGProxy *proxy, gchar* path, void *user_data);

static gboolean do_transfer(gpointer user_ptr);
static gboolean do_swap(gpointer user_ptr);
static gboolean do_hold_and_answer(gpointer user_ptr);
static gboolean do_release_and_answer(gpointer user_ptr);

static gboolean call_user_timeout(gpointer data);
static void pending_call_answerable_check_complete(DBusGProxy *proxy, GHashTable *properties, GError *error, gpointer data);
static gboolean pending_call_queue_answerable_check(gpointer data);

static gboolean pending_call_state_check(gpointer data);
static void pending_call_state_check_complete(DBusGProxy *proxy, GHashTable *properties, GError *error, gpointer data);

static int check_call_count(struct dual_call_case_state *state, int expected_count);

static
void hangup_all_calls(struct dual_call_case_state* state)
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
static void pending_call_answer_complete(__attribute__((unused)) DBusGProxy *proxy,
	GError *error, gpointer data)
{
	struct dual_call_case_state *state;

	g_assert(data);
	state = (struct dual_call_case_state *) data;

	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		if (!state->retries)
			goto test_fail;
		BLTS_WARNING("Could not answer call, retrying...\n");
		state->retries--;
		g_timeout_add(500, pending_call_queue_answerable_check, state);
	}

	return;

test_fail:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}

/* Timer callback for retrying checks whether an incoming call can be answered */
static gboolean pending_call_queue_answerable_check(gpointer data)
{
	struct dual_call_case_state *state;

	g_assert(data);
	state = (struct dual_call_case_state *) data;

	org_ofono_VoiceCall_get_properties_async(state->voice_calls[FIRST_CALL],
		pending_call_answerable_check_complete, data);

	return FALSE;
}

/* Check whether an incoming call can be answered. If not, retry later, if yes,
   answer. */
static void pending_call_answerable_check_complete(__attribute__((unused)) DBusGProxy *proxy,
	GHashTable *properties, GError *error, gpointer data)
{
	struct dual_call_case_state *state;

	g_assert(data);
	state = (struct dual_call_case_state *) data;

	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		goto test_fail;
	}

	if (g_hash_table_find(properties, (GHRFunc) check_state, "incoming"))
		org_ofono_VoiceCall_answer_async(state->voice_calls[FIRST_CALL],
			pending_call_answer_complete, state);
	else
		g_timeout_add(500, pending_call_queue_answerable_check, state);

	return;

test_fail:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}
/* Callback for retrying checks whether the first call is in held state */
static gboolean pending_call_state_check(gpointer data)
{
	struct dual_call_case_state *state;

	g_assert(data);
	state = (struct dual_call_case_state *) data;

	org_ofono_VoiceCall_get_properties_async(state->voice_calls[FIRST_CALL],
		pending_call_state_check_complete, data);

	return FALSE;
}

/* Check that first call is really in held state */
static void pending_call_state_check_complete(__attribute__((unused)) DBusGProxy *proxy,
	GHashTable *properties, GError *error, gpointer data)
{
	struct dual_call_case_state *state;

	g_assert(data);
	state = (struct dual_call_case_state *) data;

	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		goto test_fail;
	}

	if (g_hash_table_find(properties, (GHRFunc) check_state, "held"))
	{
		if (state->call_handle_method == do_hold_and_answer)
				hangup_all_calls(state);
		else
			state->call_handle_method(state);
	}
	else
	{
		g_timeout_add(500, pending_call_state_check, state);
	}

	return;

test_fail:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}

static void transfer_complete(__attribute__((unused)) DBusGProxy *proxy,  GError *error, gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	if (error)
	{
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	hangup_all_calls(state);

	FUNC_LEAVE();
}

static void swap_complete(__attribute__((unused)) DBusGProxy *proxy,  GError *error, gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	if (error)
	{
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	hangup_all_calls(state);

	FUNC_LEAVE();
}

static void release_and_answer_complete(__attribute__((unused)) DBusGProxy *proxy,  GError *error, gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	if (error)
	{
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	hangup_all_calls(state);

	FUNC_LEAVE();
}

static void hold_and_answer_complete(__attribute__((unused)) DBusGProxy *proxy, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	if (error)
	{
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	/* ensure that the first call is in held state */
	state->retries = 10;
	org_ofono_VoiceCall_get_properties_async(state->voice_calls[FIRST_CALL],
	pending_call_state_check_complete, state);

	FUNC_LEAVE();
}

/* Answer to first call and start listening property changes */
static void handle_first_call(gchar* path, GHashTable* properties, gpointer user_data)
{
	int call_index = 0;
	DBusGProxy *call=NULL;
	gpointer incoming = NULL;

	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;

	if(state->voice_calls[FIRST_CALL] != NULL)
	{
		LOG("First call already in system!\n");
		goto error;
	}

	LOG("First call received...\n");
	call_index = FIRST_CALL;

	call = dbus_g_proxy_new_for_name (state->ofono_data->connection,
			OFONO_BUS, path, OFONO_CALL_INTERFACE);

	if (!call)
	{
		LOG("Cannot get proxy for " OFONO_CALL_INTERFACE "\n");
		goto error;
	}

	state->voice_calls[call_index] = call;

	LOG("Search incoming state...\n");
	g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
	incoming = g_hash_table_find(properties, (GHRFunc) check_state, "incoming");
	if(!incoming)
	{
		LOG("Not incoming call - test failed!\n");
		goto error;
	}

	LOG("Incoming voice call '%s'\n", (char *)path);
	state->first_voice_call_path = g_strdup(path);

	if (state->signalcb_VoiceCall1_PropertyChanged)
	{
		dbus_g_proxy_add_signal(state->voice_calls[call_index], "PropertyChanged",
							G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_calls[call_index], "PropertyChanged",
							state->signalcb_VoiceCall1_PropertyChanged, state, 0);
	}

	LOG("Answer the call...\n");
	state->retries = 10;
	org_ofono_VoiceCall_get_properties_async(state->voice_calls[call_index],
	pending_call_answerable_check_complete, state);

	return;
error:
	state->result = -1;
	
	g_main_loop_quit(state->mainloop);
}

/* Redirect second call and start listening property changes */
static void handle_second_call(gchar* path, GHashTable* properties, gpointer user_data)
{
	int call_index = 0;
	DBusGProxy *call=NULL;
	gpointer waiting = NULL;

	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;

	if(state->voice_calls[FIRST_CALL] == NULL)
	{
		LOG("First call not received!\n");
		goto error;
	}

	/* skip already handled voice call */
	if(strcmp(state->first_voice_call_path, path) == 0)
	{
		LOG("Voice call:%s skipped\n", state->first_voice_call_path);
		return;
	}

	LOG("Second call received...\n");
	call_index = SECOND_CALL;

	call = dbus_g_proxy_new_for_name (state->ofono_data->connection,
			OFONO_BUS, path, OFONO_CALL_INTERFACE);

	if (!call)
	{
		LOG("Cannot get proxy for " OFONO_CALL_INTERFACE "\n");
		goto error;
	}

	state->voice_calls[call_index] = call;
	
	LOG("Search waiting state...\n");
	g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
	waiting = g_hash_table_find(properties, (GHRFunc) check_state, "waiting");
	if(!waiting)
	{
		LOG("Not waiting call - test failed!\n");
		goto error;
	}

	LOG("Incoming voice call '%s'\n", (char *)path);
	state->second_voice_call_path = g_strdup(path);

	if (state->signalcb_VoiceCall2_PropertyChanged)
	{
		dbus_g_proxy_add_signal(state->voice_calls[call_index], "PropertyChanged",
							G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_calls[call_index], "PropertyChanged",
							state->signalcb_VoiceCall2_PropertyChanged, state, 0);
	}

	if (state->call_handle_method)
	{
		g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);
	}

	return;
error:
	state->result = -1;

	g_main_loop_quit(state->mainloop);
}


/**
 * Signal handler
 * Handle first or second call when signal "CallAdded" from voice call manager is sent
 */
static void on_voice_call_manager_call_added(__attribute__((unused))DBusGProxy *proxy, gchar* path, GHashTable* properties, void *user_data)
{	
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;		
	
	LOG("Call added...%s\n", path);	

	if(!state->voice_calls[FIRST_CALL])
	{
		LOG("Handle first call...\n");
		handle_first_call(path, properties, user_data);
	}
	else if (!state->voice_calls[SECOND_CALL])
	{
		LOG("Handle second call...\n");
		handle_second_call(path, properties, user_data);
	}
	else
	{
		LOG("Already two calls in system...\n");
	}

	return;
}

/**
 * Signal handler
 * Handle "CallRemoved" signals from voice call manager - end test case when signals 
 * for both test calls are received
 */
static void on_voice_call_manager_call_removed(__attribute__((unused))DBusGProxy *proxy, gchar* path, void *user_data)
{
	static int removed_calls = 0;
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;	
	
	LOG("Call %s removed...\n", path);
	removed_calls++;
	LOG("Calls removed total: %d\n", removed_calls);
	
	/* End test case when both "CallRemoved" signals have been received and
	   VoiceCall1 and VoiceCall2 have both got 'disconnected' property change signals */
	if(state->first_voice_call_path && state->second_voice_call_path && removed_calls >= 2)
		if(!state->voice_calls[FIRST_CALL] && !state->voice_calls[SECOND_CALL])
				g_main_loop_quit(state->mainloop);			
	return;
}


/**
 * Signal handler
 * List voice call 1 property changes
 */
static void on_first_voice_call_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{

	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;

	gchar* value_str = g_value_dup_string (value);

	LOG("Voicecall 1 property: '%s' changed to '%s'\n", key, value_str);

	if(strcmp(key, "State") == 0)
	{
		if( strcmp(value_str, "disconnected") == 0)
		{
			if (state->voice_calls[FIRST_CALL])
			{
				g_object_unref(state->voice_calls[FIRST_CALL]);
				state->voice_calls[FIRST_CALL] = NULL;
			}

		}
	}

	g_free(value_str); 
}

/**
 * Signal handler
 * List voice call 2 property changes
 */
static void on_second_voice_call_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{

	struct dual_call_case_state *state = (struct dual_call_case_state *) user_data;

	gchar* value_str = g_value_dup_string (value);

	LOG("Voicecall 2 property: '%s' changed to '%s'\n", key, value_str);

	if(strcmp(key, "State") == 0)
	{
		if( strcmp(value_str, "disconnected") == 0)
		{
			if (state->voice_calls[SECOND_CALL])
			{
				g_object_unref(state->voice_calls[SECOND_CALL]);
				state->voice_calls[SECOND_CALL] = NULL;
			}

		}
	}

	g_free(value_str);
}

/**
 * Joins the currently Active and Held calls together and disconnects
 * both calls
 */
gboolean do_transfer(gpointer user_ptr)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_ptr;

	LOG("-- Transfer calls--\n");

	org_ofono_VoiceCallManager_transfer_async (state->voice_call_manager, transfer_complete, state);

	return FALSE;
}
/*
 * Swaps Active and Held calls - The call that were Active
 * will be Held	and the calls that were Held will be Active.
 */
gboolean do_swap(gpointer user_ptr)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_ptr;

	LOG("-- Swap calls--\n");

	org_ofono_VoiceCallManager_swap_calls_async (state->voice_call_manager, swap_complete, state);

	return FALSE;
}

/*
 * Put the current call on hold and answers the currently waiting call
 */
gboolean do_hold_and_answer(gpointer user_ptr)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_ptr;


	LOG("-- Hold and answer--\n");

	org_ofono_VoiceCallManager_hold_and_answer_async (state->voice_call_manager, hold_and_answer_complete, state);

	return FALSE;
}

/*
 * Release currently active call and answers the currently waiting call
 */
gboolean do_release_and_answer(gpointer user_ptr)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) user_ptr;


	LOG("-- Release and answer--\n");

	org_ofono_VoiceCallManager_release_and_answer_async (state->voice_call_manager, release_and_answer_complete, state);

	return FALSE;
}

static void call_state_finalize(struct dual_call_case_state *state)
{
	if (!state)
		return;

	if(state->first_voice_call_path)
		g_free(state->first_voice_call_path);

	if(state->second_voice_call_path)
		g_free(state->second_voice_call_path);

	if (state->voice_call_manager)
		g_object_unref(state->voice_call_manager);

	if (state->voice_calls[FIRST_CALL])
		g_object_unref(state->voice_calls[FIRST_CALL]);

	if (state->voice_calls[SECOND_CALL])
		g_object_unref(state->voice_calls[SECOND_CALL]);

	g_free(state);
}

static gboolean call_master_timeout(gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	state->result = -1;

	LOG("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}

static gboolean call_user_timeout(gpointer data)
{
	FUNC_ENTER();
	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	state->result = 0;

	LOG("Own Timeout reached\n");

	if (state->call_handle_method == do_release_and_answer)
	{
		do_release_and_answer(state);
	}
	else
	{
		do_hold_and_answer(state);
	}

	return FALSE;
}

gboolean check_call_count(struct dual_call_case_state *state, int expected_count)
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

	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

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
	BLTS_TRACE("Hangup previous calls...\n");
	hangup_all_calls(state);

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
			g_idle_add(state->case_begin, state);

	FUNC_LEAVE();
	return FALSE;
}

static struct dual_call_case_state *call_state_init(my_ofono_data *data)
{
	struct dual_call_case_state *state;
	state = g_malloc(sizeof *state);
	if (!state) {
		LOG("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}

static int call_case_run(struct dual_call_case_state *state)
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

	state->voice_call_manager = NULL;
	state->voice_calls[FIRST_CALL] = NULL;
	state->voice_calls[SECOND_CALL] = NULL;
	state->first_voice_call_path = NULL;
	state->second_voice_call_path = NULL;

	g_timeout_add(30000, (GSourceFunc) call_master_timeout, state);
	g_idle_add(call_init_start, state);

	state->result=-1;

	BLTS_TRACE("Starting main loop.\n");
	g_main_loop_run(state->mainloop);
	BLTS_TRACE("Back from main loop.\n");

	ret = state->result;

done:

	return ret;
}


static gboolean call_listen_start(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();

	struct dual_call_case_state *state = (struct dual_call_case_state *) data;

	if(!check_call_count(state, 0))
	{
		LOG("Previous calls left in system! - test failed\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
	}

	LOG("Start listening calls...\n");

	FUNC_LEAVE();
	return FALSE;
}

/* Test cases ------------> */

int blts_ofono_case_dual_call_transfer(void* user_ptr, __attribute__((unused))int test_num)
{
	int ret;
	struct dual_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 /*	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;

	test->signalcb_VoiceCall1_PropertyChanged =
		G_CALLBACK(on_first_voice_call_property_changed);
	test->signalcb_VoiceCall2_PropertyChanged =
		G_CALLBACK(on_second_voice_call_property_changed);
	test->signalcb_VoiceCallManager_CallAdded = G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = G_CALLBACK(on_voice_call_manager_call_removed);

	test->call_handle_method = do_transfer;

	test->case_begin = (GSourceFunc) call_listen_start;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

int blts_ofono_case_dual_call_swap(void* user_ptr, __attribute__((unused))int test_num)
{
	int ret;
	struct dual_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 /*	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;

	test->signalcb_VoiceCall1_PropertyChanged =
		G_CALLBACK(on_first_voice_call_property_changed);
	test->signalcb_VoiceCall2_PropertyChanged =
		G_CALLBACK(on_second_voice_call_property_changed);
	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	test->call_handle_method = do_swap;

	test->case_begin = (GSourceFunc) call_listen_start;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

int blts_ofono_case_dual_call_release_and_answer(void* user_ptr, __attribute__((unused))int test_num)
{
	int ret;
	struct dual_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

  /*	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;

	test->signalcb_VoiceCall1_PropertyChanged =
		G_CALLBACK(on_first_voice_call_property_changed);
	test->signalcb_VoiceCall2_PropertyChanged =
		G_CALLBACK(on_second_voice_call_property_changed);
	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	test->call_handle_method = do_release_and_answer;

	test->case_begin = (GSourceFunc) call_listen_start;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

int blts_ofono_case_dual_call_hold_and_answer(void* user_ptr, __attribute__((unused))int test_num)
{
	int ret;
	struct dual_call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 /*	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;

	test->signalcb_VoiceCall1_PropertyChanged =
		G_CALLBACK(on_first_voice_call_property_changed);
	test->signalcb_VoiceCall2_PropertyChanged =
		G_CALLBACK(on_second_voice_call_property_changed);
	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	test->call_handle_method = do_hold_and_answer;

	test->case_begin = (GSourceFunc) call_listen_start;
	ret = call_case_run(test);

	call_state_finalize(test);
	return ret;
}

