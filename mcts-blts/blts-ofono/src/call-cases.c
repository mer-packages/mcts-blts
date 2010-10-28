/* call-cases.c -- Voicecall cases for blts-ofono

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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <blts_log.h>

#include "ofono-util.h"
#include "ofono-general.h"

#include "call-cases.h"
#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"
#include "call-forwarding.h"


#include "signal-marshal.h"

typedef gboolean (*call_hangup_fp)(gpointer user_ptr);

struct call_case_state {
	char* address;
	char* hide_caller_id;

	GMainLoop *mainloop;

	DBusGProxy *voice_call;
	DBusGProxy *voice_call_manager;

	GSourceFunc case_begin;

	my_ofono_data *ofono_data;

	GCallback signalcb_VoiceCall_PropertyChanged;
	GCallback signalcb_VoiceCall_DisconnectReason;

	GCallback signalcb_VoiceCallManager_CallAdded;
	GCallback signalcb_VoiceCallManager_CallRemoved;

	call_hangup_fp call_hangup_method;

	gchar* voice_call_path;

	int retries;
	int user_timeout;
	char* disconnect_reason;
	char* expected_disconnect_reason;
	int result;
	int waiting_call_wait;
};


static void on_voice_call_property_changed(DBusGProxy *proxy, char *key, GValue* value, gpointer user_data);
static void on_voice_call_disconnect_reason(DBusGProxy *proxy, char *reason, gpointer user_data);

static void on_voice_call_manager_call_added(DBusGProxy *proxy, gchar* path, GHashTable* properties, gpointer user_data);
static void on_voice_call_manager_call_removed(DBusGProxy *proxy, gchar* path, gpointer user_data);

gboolean do_hangup(gpointer user_ptr);
gboolean do_hangup_all(gpointer user_ptr);
gboolean do_deflect(gpointer user_ptr);
gboolean do_cancel(gpointer user_ptr);


static gboolean call_user_timeout(gpointer data);
static void pending_call_answerable_check_complete(DBusGProxy *proxy,
	GHashTable *properties, GError *error, gpointer data);
static gboolean pending_call_queue_answerable_check(gpointer data);

GHashTable* get_voicecall_properties(DBusGProxy *proxy)
{
	GError *error = NULL;
	GHashTable* list = NULL;

	if(!proxy)
		return NULL;

	if(!org_ofono_VoiceCall_get_properties(proxy, &list, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return NULL;
	}
	return list;
}

GHashTable* get_voicecall_manager_properties(DBusGProxy *proxy)
{
	GError *error = NULL;
	GHashTable* list = NULL;

	if(!proxy)
		return NULL;

	if(!org_ofono_VoiceCallManager_get_properties(proxy, &list, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return NULL;
	}
	return list;
}

/* Completion of async answer call. Retry limited times if failure. */
static void pending_call_answer_complete(__attribute__((unused)) DBusGProxy *proxy,
	GError *error, gpointer data)
{
	struct call_case_state *state;

	g_assert(data);
	state = (struct call_case_state *) data;

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
	struct call_case_state *state;

	g_assert(data);
	state = (struct call_case_state *) data;

	org_ofono_VoiceCall_get_properties_async(state->voice_call,
		pending_call_answerable_check_complete, data);

	return FALSE;
}

/* Check whether an incoming call can be answered. If not, retry later, if yes,
   answer. */
static void pending_call_answerable_check_complete(__attribute__((unused)) DBusGProxy *proxy,
	GHashTable *properties, GError *error, gpointer data)
{
	struct call_case_state *state;

	g_assert(data);
	state = (struct call_case_state *) data;

	if (error) {
		display_dbus_glib_error(error);
		g_error_free(error);
		goto test_fail;
	}

	if (g_hash_table_find(properties, (GHRFunc) check_state, "incoming"))
		org_ofono_VoiceCall_answer_async(state->voice_call,
			pending_call_answer_complete, state);
	else
		g_timeout_add(500, pending_call_queue_answerable_check, state);

	return;

test_fail:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}


/**
 * Handle incoming voice call for hangup test cases
 */
static void handle_incoming_call(gchar* path, GHashTable* properties, gpointer user_data)
{
	DBusGProxy *call=NULL;
	gpointer incoming = NULL;
	gpointer waiting = NULL;

	struct call_case_state *state = (struct call_case_state *) user_data;

	LOG("Incoming voice call '%s'\n", (char *)path);
	if (state->voice_call != NULL && state->waiting_call_wait == 0)
	{
		LOG("Previous call already active - test failed\n");
		goto error;
	}

	call = dbus_g_proxy_new_for_name (state->ofono_data->connection,
			OFONO_BUS, path, OFONO_CALL_INTERFACE);

	if (!call)
	{
		LOG("Cannot get proxy for " OFONO_CALL_INTERFACE "\n");
		goto error;
	}

	state->voice_call = call;

	if (state->signalcb_VoiceCall_PropertyChanged)
	{
		dbus_g_proxy_add_signal(state->voice_call, "PropertyChanged",
							G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call, "PropertyChanged",
							state->signalcb_VoiceCall_PropertyChanged, state, 0);
	}

	if (state->signalcb_VoiceCall_DisconnectReason)
	{
		dbus_g_proxy_add_signal(state->voice_call, "DisconnectReason",
							G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call, "DisconnectReason",
							state->signalcb_VoiceCall_DisconnectReason, state, 0);
	}


	LOG("Search incoming/waiting calls...\n");
	if (properties) {
		g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		incoming = g_hash_table_find(properties, (GHRFunc) check_state, "incoming");
		waiting = g_hash_table_find(properties, (GHRFunc) check_state, "waiting");
	}

	if(incoming)
	{
		state->voice_call_path = g_strdup(path);
		
		/* if the cancel or deflect hangup method is active do not answer at all*/
		if(state->call_hangup_method != do_cancel &&
		   state->call_hangup_method != do_deflect)
		{
			LOG("Answer the call...\n");

			state->retries = 10;
			org_ofono_VoiceCall_get_properties_async(state->voice_call,
				pending_call_answerable_check_complete, state);
		}

		if (state->call_hangup_method && state->waiting_call_wait == 0)
		{
			g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);
		}
		else
		{
			if(state->waiting_call_wait == 0)
			{
				LOG("Wait remote side to hang up the call...\n");
				state->result = 0;
			}
			else
			{
				LOG("Expecting another call...\n");
			}
		}
	}
	else if(waiting)
	{
		LOG("We have call waiting!\n");
		g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);
		state->result = 0;
	}
	return;
error:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
}

/**
 * Signal handler
 * List voice call property changes
 */
static void on_voice_call_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, gpointer user_data)
{
	gchar* value_str = g_value_dup_string (value);
	struct call_case_state *state = (struct call_case_state *) user_data;

	LOG("Voicecall property: '%s' changed to '%s'\n", key, value_str);

	if(!strcmp(key, "State") && !state->voice_call_path)
	{

		if(strcmp(value_str, "disconnected") == 0)
			g_main_loop_quit(state->mainloop);
	}
	
	g_free(value_str);	
}


/**
 * Signal handler
 * Displays and handles disconnect reason
 *
 * NOTE:
 * Not all implementations are able to provide this information,
 * so applications should treat the emission of this signal as optional.
 */
static void on_voice_call_disconnect_reason(__attribute__((unused))DBusGProxy *proxy, char *reason, gpointer user_data)
{
	struct call_case_state *state = (struct call_case_state *) user_data;

	LOG("Disconnect reason: '%s'\n", reason);
	state->disconnect_reason = strdup(reason);
}


/**
 * Signal handler
 * Handle first or second call when signal "CallAdded" from voice call manager is sent
 */
static void on_voice_call_manager_call_added(__attribute__((unused))DBusGProxy *proxy, gchar* path, GHashTable* properties, gpointer user_data)
{
	LOG("Call added...%s\n", path);		
	handle_incoming_call(path, properties, user_data);
}

/**
 * Signal handler
 * Handle "CallRemoved" signals from voice call manager - end test case when signals 
 * for both test calls are received
 */
static void on_voice_call_manager_call_removed(__attribute__((unused))DBusGProxy *proxy, gchar* path, gpointer user_data)
{
	struct call_case_state *state = (struct call_case_state *) user_data;	
	LOG("Call %s removed...\n", path);
		
	if(!strcmp(state->voice_call_path, path))
		g_main_loop_quit(state->mainloop);			
}


/**
 * Hang up call
 */

gboolean do_hangup(gpointer user_ptr)
{
	struct call_case_state *state = (struct call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Hanging up call--\n");
	if(!org_ofono_VoiceCall_hangup(state->voice_call, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return TRUE;
	}

	return FALSE;
}


/**
 * Hang up all calls
 */

gboolean do_hangup_all(gpointer user_ptr)
{
	struct call_case_state *state = (struct call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Hanging up all calls--\n");
	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return TRUE;
	}
	return FALSE;
}


/**
 * Deflect call
 */

gboolean do_deflect(gpointer user_ptr)
{
	struct call_case_state *state = (struct call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Deflect to address:%s--\n", state->address);

	if(!org_ofono_VoiceCall_deflect(state->voice_call, state->address, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return TRUE;
	}
	return FALSE;
}

/**
 * Cancel call
 */

gboolean do_cancel(gpointer user_ptr)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) user_ptr;
	GError *error = NULL;

	LOG("-- Cancel call--\n");
	if(!org_ofono_VoiceCall_hangup(state->voice_call, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return TRUE;
	}
	return FALSE;
}

static void call_state_finalize(struct call_case_state *state)
{
	if (!state)
		return;

	if (state->address)
		free(state->address);

	if (state->disconnect_reason)
		free(state->disconnect_reason);

	if (state->voice_call_manager)
	{
		g_object_unref(state->voice_call_manager);
	}

	if (state->voice_call)
	{
		g_object_unref(state->voice_call);
	}

	if(state->voice_call_path)
		g_free(state->voice_call_path);

	free(state);
}

static gboolean call_master_timeout(gpointer data)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) data;

	state->result = -1;

	LOG("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}

static gboolean call_user_timeout(gpointer data)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) data;

	state->result = 0;

	LOG("Own Timeout reached\n");

	if (state->call_hangup_method)
	{
		LOG("Hang up the call...\n");
		if (state->call_hangup_method(state))
		{
			LOG("Error\n");
			state->result = -1;
		}
		else
			state->result = 0;
	}
	return FALSE;
}

static gboolean call_init_start(gpointer data)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) data;

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

static struct call_case_state *call_state_init(my_ofono_data *data)
{
	struct call_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		LOG("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}

static int call_case_run(struct call_case_state *state)
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
	state->voice_call = NULL;
	state->voice_call_path = NULL;

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

static gboolean call_send_dtmf(gpointer data)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) data;
	GError *error = NULL;

	LOG("Sending DTMF.\n");

	if(!org_ofono_VoiceCallManager_send_tones(state->voice_call_manager, state->ofono_data->dtmf_tone, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return TRUE;
	}

	return FALSE;
}


static int dtmf_case_run(struct call_case_state *state)
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
	state->voice_call = NULL;
	g_timeout_add(10000, (GSourceFunc) call_send_dtmf, state);
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


static void call_complete(__attribute__((unused)) DBusGProxy *proxy, char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();
	DBusGProxy *call_proxy=NULL;
	struct call_case_state *state = (struct call_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	call_proxy = dbus_g_proxy_new_for_name (state->ofono_data->connection,
						OFONO_BUS, call_path, OFONO_CALL_INTERFACE);

	LOG("Call path > '%s'\n", call_path);

	if (!call_proxy)
	{
		LOG("Cannot get proxy for " OFONO_CALL_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	state->voice_call = call_proxy;

	if (state->signalcb_VoiceCall_PropertyChanged)
	{
		dbus_g_proxy_add_signal(state->voice_call, "PropertyChanged",
							G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call, "PropertyChanged",
						state->signalcb_VoiceCall_PropertyChanged, state, 0);
	}
	if (state->signalcb_VoiceCall_DisconnectReason)
	{
		dbus_g_proxy_add_signal(state->voice_call, "DisconnectReason",
							G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->voice_call, "DisconnectReason",
						state->signalcb_VoiceCall_DisconnectReason, state, 0);
	}

	g_timeout_add(state->user_timeout, (GSourceFunc) call_user_timeout, state);

	FUNC_LEAVE();
}

static gboolean call_voicecallto_start(gpointer data)
{
	FUNC_ENTER();
	struct call_case_state *state = (struct call_case_state *) data;

	GHashTable* properties = get_voicecall_manager_properties(state->voice_call_manager);
	if (properties) {
		g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		g_hash_table_destroy(properties);
	}
	properties = NULL;

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, state->hide_caller_id, call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
}


static gboolean call_answer_start(__attribute__((unused))gpointer data)
{
	FUNC_ENTER();

	LOG("Start listening calls\n");

	FUNC_LEAVE();
	return FALSE;
}

/* Test cases ------------> */
int my_ofono_case_voicecall_to(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;
	test->disconnect_reason = NULL;
	test->call_hangup_method = do_hangup;

	test->signalcb_VoiceCall_PropertyChanged =
		G_CALLBACK(on_voice_call_property_changed);
	test->signalcb_VoiceCall_DisconnectReason =
		G_CALLBACK(on_voice_call_disconnect_reason);		
	test->signalcb_VoiceCallManager_CallAdded = NULL;
	test->signalcb_VoiceCallManager_CallRemoved  = NULL; 
		
	switch(testnum)
	{
		case BLTS_OFONO_CREATE_VOICECALL:
			// should be same as "disabled" and in most cases "default"
			test->hide_caller_id = "";
			test->case_begin = (GSourceFunc) call_voicecallto_start;
			ret = call_case_run(test);
			break;
		case BLTS_OFONO_CREATE_VOICECALL_WITH_HIDDEN_CALLER_ID:
			test->hide_caller_id = "enabled";
			test->case_begin = (GSourceFunc) call_voicecallto_start;
			ret = call_case_run(test);
			break;
		default:  LOG("Test %d - internal error, case unknown\n", testnum);
			call_state_finalize(test);
			return -1;

	}
	call_state_finalize(test);
	return ret;
}

int my_ofono_case_voicecall_answer(void* user_ptr, int test_num)
{
	int ret;
	struct call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);
	test->waiting_call_wait = 0;
	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->user_timeout = data->user_timeout ? data->user_timeout : 10000;
	test->disconnect_reason = NULL;

	test->signalcb_VoiceCall_PropertyChanged =
		G_CALLBACK(on_voice_call_property_changed);
	test->signalcb_VoiceCall_DisconnectReason =
		G_CALLBACK(on_voice_call_disconnect_reason);
	test->signalcb_VoiceCallManager_CallAdded = 
		G_CALLBACK(on_voice_call_manager_call_added);
	test->signalcb_VoiceCallManager_CallRemoved = 
		G_CALLBACK(on_voice_call_manager_call_removed);

	switch(test_num)
	{
		case BLTS_OFONO_ANSWER_TO_VOICECALL_AND_HANGUP:
				test->call_hangup_method = do_hangup;
				test->expected_disconnect_reason = "local";
				break;

		case BLTS_OFONO_ANSWER_TO_VOICECALL_AND_HANGUP_ALL:
				test->call_hangup_method = do_hangup_all;
				test->expected_disconnect_reason = "local";
				break;

		case BLTS_OFONO_ANSWER_TO_VOICECALL_AND_DEFLECT:
				test->call_hangup_method = do_deflect;
				test->expected_disconnect_reason = "local";
				break;

		case BLTS_OFONO_ANSWER_TO_VOICECALL_AND_REMOTE_HANGUP:
				test->call_hangup_method = NULL;
				test->expected_disconnect_reason = "remote";
				break;

		case BLTS_OFONO_CANCEL_VOICECALL:
				test->call_hangup_method = do_cancel;
				test->expected_disconnect_reason = "local";
				break;

		case BLTS_OFONO_ANSWER_TO_VOICECALL_AND_WAIT_FOR_CALL:
			test->waiting_call_wait = 1;
			test->call_hangup_method = do_hangup_all;
			test->expected_disconnect_reason = "local";
			break;

		default:  LOG("Test %d - call_hangup_method undefined\n", test_num);
				call_state_finalize(test);
				return -1;
	}

	test->case_begin = (GSourceFunc) call_answer_start;
	ret = call_case_run(test);

	//if disconnect reason signal is emitted, verify that it is expected one
	if(!ret && test->disconnect_reason)
		ret = strcmp( test->disconnect_reason,  test->expected_disconnect_reason );

	call_state_finalize(test);
	return ret;
}

int my_ofono_case_voicecall_to_DTMF(void* user_ptr, __attribute__((unused))int test_num)
{
	int ret;
	struct call_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = call_state_init(data);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->user_timeout = data->user_timeout ? data->user_timeout : 20000;
	test->disconnect_reason = NULL;
	test->call_hangup_method = do_hangup;

	test->signalcb_VoiceCall_PropertyChanged =
		G_CALLBACK(on_voice_call_property_changed);
	test->signalcb_VoiceCall_DisconnectReason =
		G_CALLBACK(on_voice_call_disconnect_reason);
	test->signalcb_VoiceCallManager_CallAdded = NULL;
	test->signalcb_VoiceCallManager_CallRemoved = NULL;

	test->case_begin = (GSourceFunc) call_voicecallto_start;
	ret = dtmf_case_run(test);


	call_state_finalize(test);
	return ret;
}

//variable data seeds
/* Convert generated parameters to test case format. */
void *voicecall_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *addr_prefix = NULL, *addr = NULL, *new_addr = NULL;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	addr_prefix = blts_config_boxed_value_get_string(args);
	args = args->next;
	addr = blts_config_boxed_value_get_string(args);

	/* These are already non-zero, if set on command line */

	if (!data->remote_address) {
		if (asprintf(&new_addr, "%s%s", addr_prefix, addr) < 0)
			return NULL;
		data->remote_address = new_addr;
	}
	return data;
}

void *dtmf_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *remote_addr = 0, *dtmf_tone = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	remote_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	dtmf_tone = strdup(blts_config_boxed_value_get_string(args));

	if (data->remote_address)
		free(remote_addr);
	else
		data->remote_address = remote_addr;

	if (data->dtmf_tone)
		free(dtmf_tone);
	else
		data->dtmf_tone = dtmf_tone;

	return data;
}

/* Generate tones with basic chars until asked length is done */
struct boxed_value *dtmf_variant_tones_generator(struct boxed_value *args)
{
	char *seed, *message;
	unsigned seed_len, total_len, i;
	seed = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	total_len = blts_config_boxed_value_get_int(args);

	seed_len = strlen(seed);
	message = malloc(total_len + 1);
	for (i = 0; i < total_len; ++i)
		message[i] = seed[i % seed_len];
	message[total_len] = '\0';

	return blts_config_boxed_value_new_string_take(message);
}
