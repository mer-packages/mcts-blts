/* sms-cases.c -- SMS cases

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

#define _GNU_SOURCE // To get asprintf from stdio.h
#include <stdio.h>
#include <glib.h>
#include <blts_log.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdlib.h>
#include <glib-object.h>

#include "sms-cases.h"
#include "ofono-modem-bindings.h"
#include "ofono-general.h"
#include "ofono-util.h"

struct sms_case_state {
	char* message;
	char* address;

	GMainLoop *mainloop;

	DBusGProxy *msg_manager;

	GSourceFunc case_begin;

	my_ofono_data *ofono_data;

	GValue *smsc;

	DBusGProxy *message_proxy;
	gchar* message_state;
	gchar* message_path;

	GCallback signalcb_MessageManager_PropertyChanged;
	GCallback signalcb_MessageManager_IncomingMessage;
	GCallback signalcb_MessageManager_ImmediateMessage;
	GCallback signalcb_MessageManager_MessageAdded;
	GCallback signalcb_MessageManager_MessageRemoved;
	GCallback signalcb_MessageManager_Message_PropertyChanged;

	int result;
};


static void sms_state_finalize(struct sms_case_state *state)
{
	if (!state)
		return;

	if (state->address)
		free(state->address);
	if (state->message)
		free(state->message);
	if (state->smsc)
		free(state->smsc);
	if (state->message_proxy)
		g_object_unref(state->message_proxy);
	if (state->message_state)
		g_free(state->message_state);
	if (state->message_path)
		g_free(state->message_path);
	if (state->msg_manager)
		g_object_unref(state->msg_manager);

	free(state);
}


static gboolean sms_timeout(gpointer data)
{
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

	state->result = -1;

	log_print("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}


/* Init completion (aka return from the SetProperty call). Sets up callbacks
   (as configured by caller). Queues the configured test case call when done. */
static void sms_init_complete(__attribute__((unused)) DBusGProxy *proxy, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

	if (error) {
		log_print("Init failure (while setting SMSC number): %s\n", error->message);
		state->result=-1;
		g_main_loop_quit(state->mainloop);
	}

	log_print("SMSC is %s\n", g_value_get_string(state->smsc));

	if (state->signalcb_MessageManager_PropertyChanged) {
		dbus_g_proxy_add_signal(state->msg_manager, "PropertyChanged",
			G_TYPE_STRING, G_TYPE_VALUE,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->msg_manager, "PropertyChanged",
			state->signalcb_MessageManager_PropertyChanged, state, 0);
	}

	if (state->signalcb_MessageManager_IncomingMessage) {
		dbus_g_proxy_add_signal(state->msg_manager, "IncomingMessage",
			G_TYPE_STRING, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->msg_manager, "IncomingMessage",
			state->signalcb_MessageManager_IncomingMessage, state, 0);
	}

	if (state->signalcb_MessageManager_ImmediateMessage) {
		dbus_g_proxy_add_signal(state->msg_manager, "ImmediateMessage",
			G_TYPE_STRING, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->msg_manager, "ImmediateMessage",
			state->signalcb_MessageManager_ImmediateMessage, state, 0);
	}
	
	if (state->signalcb_MessageManager_MessageAdded) {
		dbus_g_proxy_add_signal(state->msg_manager, "MessageAdded",
			DBUS_TYPE_G_OBJECT_PATH,
			dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->msg_manager, "MessageAdded",
			state->signalcb_MessageManager_MessageAdded, state, 0);
	}

	if (state->signalcb_MessageManager_MessageRemoved) {
		dbus_g_proxy_add_signal(state->msg_manager, "MessageRemoved",
			DBUS_TYPE_G_OBJECT_PATH,
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state->msg_manager, "MessageRemoved",
			state->signalcb_MessageManager_MessageRemoved, state, 0);
	}

	if (state->case_begin)
		g_idle_add(state->case_begin, state);

	FUNC_LEAVE();
}

/* Common init function. Prepares MessageManager, sets the SMSC. */
static gboolean sms_init_start(gpointer data)
{
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

	DBusGProxy *msg_manager;

	msg_manager = dbus_g_proxy_new_for_name(state->ofono_data->connection,
		OFONO_BUS, state->ofono_data->modem[0], OFONO_MESSAGE_INTERFACE);

	if (!msg_manager) {
		log_print("Cannot get proxy for " OFONO_MESSAGE_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	state->msg_manager = msg_manager;

	state->smsc = malloc(sizeof *(state->smsc));
	memset(state->smsc, 0, sizeof *(state->smsc));

	g_value_init(state->smsc, G_TYPE_STRING);
	if (state->ofono_data->smsc_address)
		g_value_set_string(state->smsc, state->ofono_data->smsc_address);
	else
		g_value_set_static_string(state->smsc, SMS_SMSC_ADDRESS);

	org_ofono_MessageManager_set_property_async(msg_manager, "ServiceCenterAddress", state->smsc,
		sms_init_complete, state);

	FUNC_LEAVE();
	return FALSE;
}




/* Some generic callbacks with debug output, for use when more specific things
   are unnecessary */

static void sms_generic_property_changed_cb(__attribute__((unused)) DBusGProxy *proxy,
	char *key, GValue *value, __attribute__((unused)) gpointer data)
{
	FUNC_ENTER();
	char *val = g_strdup_value_contents(value);
	log_print("MessageManager PropertyChanged: '%s' -> '%s'\n", key, val);
	free(val);
	FUNC_LEAVE();
}

static void sms_smsc_property_changed_cb(__attribute__((unused)) DBusGProxy *proxy,
	char *key, GValue *value, gpointer data)
{
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;
	char *val = g_strdup_value_contents(value);
	log_print("MessageManager PropertyChanged: '%s' -> '%s'\n", key, val);
	free(val);
	state->result = 0;
	g_main_loop_quit(state->mainloop);
	FUNC_LEAVE();
}

static void debug_print_asv_value(gpointer key, gpointer value,
	__attribute__((unused)) gpointer data)
{
	if (!G_IS_VALUE(value)) {
		log_print("Not a value !?!??\n");
		return;
	}
	char *val = g_strdup_value_contents((GValue *) value);
	log_print("  %s = %s\n", (char *) key, val);
	free(val);
}

static void sms_generic_incoming_message_cb(__attribute__((unused)) DBusGProxy *proxy,
	gpointer *msg, GHashTable *properties, __attribute__((unused)) gpointer data)
{
	unsigned len;
	FUNC_ENTER();
 	log_print("MessageManager IncomingMessage / ImmediateMessage:\n");
	log_print("+ Message contents:\n");
 	log_print("--------\n");
	len = strlen((char*)msg);
	if (len < 320)
		log_print("%s\n",msg);
	else
		log_print("%.40s...\n(total %d chars)\n", (char*)msg, len);
 	log_print("--------\n");
 	log_print("+ Properties:\n");
	g_hash_table_foreach(properties, debug_print_asv_value, 0);
	FUNC_LEAVE();
}

static void sms_message_added_cb(DBusGProxy *proxy, const gchar *path, GHashTable *properties, gpointer data)
{
	FUNC_ENTER();
	
	BLTS_UNUSED_PARAM(proxy);

	struct sms_case_state *state = (struct sms_case_state*)data;

	BLTS_TRACE("Message added @ %s\n", path);
	
	// Process only the message that we sent
	if (g_strcmp0(state->message_path, path) != 0)
	{
		BLTS_TRACE("Message added with path %s, expecting path %s\n",
			path, state->message_path);
		return;
	}
	
	//g_hash_table_foreach(properties, debug_print_asv_value, 0);
	GValue* message_state = g_hash_table_lookup(properties, "State");
	if (!message_state)
	{
		BLTS_ERROR("Message does not have a 'State' property\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	state->message_state = g_value_dup_string(message_state);

	FUNC_LEAVE();
}

static void sms_message_removed_cb(DBusGProxy *proxy,
	const gchar *path, gpointer data)
{
	FUNC_ENTER();
	
	BLTS_UNUSED_PARAM(proxy);
	struct sms_case_state *state = (struct sms_case_state*)data;

	BLTS_TRACE("Message removed @ %s\n", path);
	
	// Process only the message that we sent
	if (g_strcmp0(state->message_path, path) != 0)
	{
		BLTS_TRACE("Message removed with path %s, expecting path %s\n",
			path, state->message_path);
		return;
	}

	dbus_g_proxy_disconnect_signal(state->message_proxy, "PropertyChanged",
		state->signalcb_MessageManager_Message_PropertyChanged, state);

	if (g_strcmp0(state->message_state, "sent") == 0)
	{
	    state->result = 0;
	}
	else
	{
	    BLTS_ERROR("Final message state is '%s' instead of 'sent'\n", state->message_state);
	    state->result = -1;
	}
	
	g_main_loop_quit(state->mainloop);

	FUNC_LEAVE();
}

static void sms_message_property_changed_cb(DBusGProxy *proxy,
	const gchar *key, const GValue *value, gpointer data)
{
	FUNC_ENTER();
	
	BLTS_UNUSED_PARAM(data);
	struct sms_case_state *state = (struct sms_case_state*)data;

	BLTS_TRACE("Message %s: %s -> %s\n", dbus_g_proxy_get_path(proxy),
		key, g_value_get_string(value));
	if (g_strcmp0(key, "State") == 0)
	{
		// Store the new message state
		if (state->message_state)
			g_free(state->message_state);
		state->message_state = g_value_dup_string(value);
	}
	
	FUNC_LEAVE();
}

static void sms_receive_test_incoming_message_cb(__attribute__((unused)) DBusGProxy *proxy,
	gpointer *msg, GHashTable *properties, gpointer data)
{
	unsigned len;
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

 	log_print("Incoming message:\n");
	log_print("+ Message contents:\n");
 	log_print("--------\n");
	len = strlen((char*)msg);
	if (len < 320)
		log_print("%s\n",msg);
	else
		log_print("%.40s...\n(total %d chars)\n", (char*)msg, len);
 	log_print("--------\n");
 	log_print("+ Properties:\n");
	g_hash_table_foreach(properties, debug_print_asv_value, 0);

	log_print("\nReceive test complete.\n");
	state->result = 0;
	g_main_loop_quit(state->mainloop);
	FUNC_LEAVE();
}



/* Return from the send call. */
static void sms_send_complete(__attribute__((unused)) DBusGProxy *proxy, char *message_path, GError *error, gpointer data)
{
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

	if (error) {
		state->result = 1;
		log_print("SMS Send failure: %s\n", error->message);
	} else {
		state->result = 0;
		log_print("SMS Send call successful\n");
		BLTS_TRACE("Message path is %s\n", message_path);
		
		state->message_path = g_strdup(message_path);

		DBusGProxy *message;
		message = dbus_g_proxy_new_for_name(state->ofono_data->connection,
			OFONO_BUS, message_path, "org.ofono.Message");

		if (!message) {
			BLTS_ERROR("Cannot get proxy for org.ofono.Message %s\n", message_path);
			state->result = -1;
			g_main_loop_quit(state->mainloop);
			return;
		}

		state->message_proxy = message;

		dbus_g_proxy_add_signal(message, "PropertyChanged",
			G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(message, "PropertyChanged",
			state->signalcb_MessageManager_Message_PropertyChanged,
			state, 0);
	}

	FUNC_LEAVE();
}

/* Starts the send call. */
static gboolean sms_send_start(gpointer data)
{
	unsigned len;
	FUNC_ENTER();
	struct sms_case_state *state = (struct sms_case_state *) data;

	org_ofono_MessageManager_send_message_async(state->msg_manager,
		state->address, state->message, sms_send_complete, state);

	log_print("Starting send to %s, message content:\n", state->address);
	log_print("--------\n");
	len = strlen(state->message);
	if (len < 320)
		log_print("%s\n",state->message);
	else
		log_print("%.40s...\n(total %d chars)\n", state->message, len);
	log_print("--------\n");
	FUNC_LEAVE();
	return FALSE;
}

/* Starts the receive test. */
static gboolean sms_receive_start(__attribute__((unused)) gpointer data)
{
	FUNC_ENTER();
/* 	struct sms_case_state *state = (struct sms_case_state *) data; */

	log_print("Waiting for message...\n");
	FUNC_LEAVE();
	return FALSE;
}


/* Starts the receive test. */
static gboolean smsc_change_start(gpointer data)
{
	FUNC_ENTER();
 	struct sms_case_state *state = (struct sms_case_state *) data;
	GError *error = NULL;
	GValue *smsc = NULL;
	smsc = malloc(sizeof *(smsc));
	memset(smsc, 0, sizeof *(smsc));
	state->result=0;
	g_value_init(smsc, G_TYPE_STRING);
	g_value_set_static_string(smsc, "12345678901234567890123");
	log_print("Changing SMSC number (length 23)...\n");
	if(!org_ofono_MessageManager_set_property(state->msg_manager, "ServiceCenterAddress", smsc, &error))
	{
		LOG("Not changed to too long SMSC, test ok\n");
		g_error_free (error);
		error=NULL;
	}
	else
	{
		LOG("Changed to too long SMSC");
		goto error;
	}
	g_value_unset(smsc);
	g_value_init(smsc, G_TYPE_STRING);
	g_value_set_static_string(smsc, "1234567890123456789012");
	log_print("Changing SMSC number (length 22)...\n");
	if(!org_ofono_MessageManager_set_property(state->msg_manager, "ServiceCenterAddress", smsc, &error))
	{
		LOG("Not changed to too long SMSC, test ok\n");
		g_error_free (error);
		error=NULL;
		state->result = 0;
	}
	else
	{
		LOG("Changed to too long SMSC");
		goto error;
	}

	g_value_unset(smsc);
	g_value_init(smsc, G_TYPE_STRING);
	g_value_set_static_string(smsc, "123456789012345678901");
	log_print("Changing SMSC number (length 21)...\n");
	if(!org_ofono_MessageManager_set_property(state->msg_manager, "ServiceCenterAddress", smsc, &error))
	{
		LOG("Not changed to too long SMSC, test ok\n");
		g_error_free (error);
		error=NULL;
		state->result = 0;
	}
	else
	{
		LOG("Changed to too long SMSC");
		goto error;
	}

	g_value_unset(smsc);
	g_value_init(smsc, G_TYPE_STRING);
	g_value_set_static_string(smsc, "12345678901234567890");
	log_print("Changing SMSC number (length 20)...\n");
	if(!org_ofono_MessageManager_set_property(state->msg_manager, "ServiceCenterAddress", smsc, &error))
	{
		LOG("Can't change SMSC number");
		display_dbus_glib_error(error);
		g_error_free (error);
		error=NULL;
		state->result = -1;
	}


	g_value_unset(smsc);
	free(smsc);
	FUNC_LEAVE();
	return FALSE;
error:

	state->result=-1;
	g_main_loop_quit(state->mainloop);
	free(smsc);
	FUNC_LEAVE();
	return FALSE;

}



static struct sms_case_state *sms_state_init(my_ofono_data *data)
{
	struct sms_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		log_print("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}


static int sms_case_run(struct sms_case_state *state)
{
	int ret;
	BLTS_TRACE("Getting modem:\n");

	ret = my_ofono_get_modem(state->ofono_data);
	if (ret) {
		log_print("Failed getting modem.\n");
		goto done;
	}
	if (state->ofono_data->number_modems < 1) {
		log_print("No modems available.\n");
		ret = -1;
		goto done;
	}
	BLTS_TRACE("Modem ok.\n");
	state->mainloop = state->ofono_data->mainloop;

	g_timeout_add(90000, (GSourceFunc) sms_timeout, state);

	g_idle_add(sms_init_start, state);

	state->result=0xdeadbeef;

	BLTS_TRACE("Starting main loop.\n");
	g_main_loop_run(state->mainloop);
	BLTS_TRACE("Back from main loop.\n");
	
	ret = state->result;

done:

	return ret;
}



/* Test cases ------------> */

/* Plain SMS send with defaults */
int blts_ofono_send_sms_default(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct sms_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

/* 	log_set_level(5); */

	if (!data)
		return -EINVAL;

	test = sms_state_init(data);

	if (!test)
		return -1;

	test->message = strdup((data->sms_generated_message) ? (data->sms_generated_message) : "Test");
	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");

	test->signalcb_MessageManager_PropertyChanged =
		G_CALLBACK(sms_generic_property_changed_cb);
	test->signalcb_MessageManager_IncomingMessage =
		G_CALLBACK(sms_generic_incoming_message_cb);
	test->signalcb_MessageManager_ImmediateMessage =
		G_CALLBACK(sms_generic_incoming_message_cb);

	test->signalcb_MessageManager_MessageAdded =
		G_CALLBACK(sms_message_added_cb);
	test->signalcb_MessageManager_MessageRemoved =
		G_CALLBACK(sms_message_removed_cb);
	test->signalcb_MessageManager_Message_PropertyChanged =
		G_CALLBACK(sms_message_property_changed_cb);

	test->case_begin = sms_send_start;
	
	ret = sms_case_run(test);

	sms_state_finalize(test);
	return ret;
}


/* Plain SMS reception with defaults */
int blts_ofono_receive_sms_default(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct sms_case_state *test;

/* 	log_set_level(5); */

	test = sms_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->signalcb_MessageManager_PropertyChanged =
		G_CALLBACK(sms_generic_property_changed_cb);
	test->signalcb_MessageManager_IncomingMessage =
		G_CALLBACK(sms_receive_test_incoming_message_cb);
	test->signalcb_MessageManager_ImmediateMessage =
		G_CALLBACK(sms_generic_incoming_message_cb);

	test->case_begin = (GSourceFunc) sms_receive_start;
	ret = sms_case_run(test);

	sms_state_finalize(test);
	return ret;
}


/* Try different values for SMSC */
int ofono_sms_center_number(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct sms_case_state *test;

/* 	log_set_level(5); */

	test = sms_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->signalcb_MessageManager_PropertyChanged =
		G_CALLBACK(sms_smsc_property_changed_cb);
	test->signalcb_MessageManager_IncomingMessage =
		G_CALLBACK(sms_generic_incoming_message_cb);
	test->signalcb_MessageManager_ImmediateMessage =
		G_CALLBACK(sms_generic_incoming_message_cb);

	test->case_begin = (GSourceFunc) smsc_change_start;
	ret = sms_case_run(test);

	sms_state_finalize(test);
	return ret;
}

/* Convert generated parameters to test case format. */
void *sms_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *sms_generated_msg = 0, *remote_addr = 0, *smsc_addr = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	sms_generated_msg = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	remote_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	smsc_addr = strdup(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->sms_generated_message)
		free(sms_generated_msg);
	else
		data->sms_generated_message = sms_generated_msg;
	
	if (data->remote_address)
		free(remote_addr);
	else
		data->remote_address = remote_addr;
	
	if (data->smsc_address)
		free(smsc_addr);
	else
		data->smsc_address = smsc_addr;

	return data;
}

/* Generate message repeating seed until asked length is done */
struct boxed_value *sms_variant_message_generator(struct boxed_value *args)
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

