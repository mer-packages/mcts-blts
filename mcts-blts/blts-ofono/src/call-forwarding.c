/* call-forwarding.c -- Voicecall forwarding functions

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

#include "call-forwarding.h"
#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"

#include "signal-marshal.h"

struct call_forwarding_state
{
	my_ofono_data* ofono_data;
	DBusGProxy* proxy;

	GCallback signalcb_CallForwarding_PropertyChanged;

	const char* expected_key;
	GValue* expected_value;
	gint result;
};

// disable
int
ofono_call_forwarding_disable_all(void* user_ptr, char* modem_path,
    char *condition)
{
	FUNC_ENTER();
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	int retval = 0;

	if (condition == NULL)
		condition = "all";

	proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS, modem_path,
	    OFONO_CALL_FORWARDING_INTERFACE);
	if (!proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CALL_FORWARDING_INTERFACE "\n");
		return -1;
		FUNC_LEAVE();
	}

	if (!org_ofono_CallForwarding_disable_all(proxy, condition, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		retval = -1;
	}
	g_object_unref(proxy);
	FUNC_LEAVE();
	return retval;
}

// forwardings

static void
on_call_forwarding_manager_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key,
    GValue* value, __attribute__((unused))gpointer user_data)
{
	FUNC_ENTER();
	struct call_forwarding_state* state =
	    (struct call_forwarding_state*) user_data;
	BLTS_DEBUG("Call forwarding property: '%s' changed to '%s'\n", key,
	    g_strdup_value_contents(value));
	g_main_loop_quit(state->ofono_data->mainloop);
}

/* Timeout callback */
static gboolean
call_forwarding_timeout(gpointer user_data)
{
	struct call_forwarding_state* state =
	    (struct call_forwarding_state*) user_data;
	state->result = -1;

	BLTS_DEBUG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->ofono_data->mainloop);

	return FALSE;

}

static void
setup_signal_callbacks(struct call_forwarding_state* state)
{
	state->signalcb_CallForwarding_PropertyChanged = G_CALLBACK(
	    on_call_forwarding_manager_property_changed);

	dbus_g_proxy_add_signal(state->proxy, "PropertyChanged", G_TYPE_STRING,
	    G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->proxy, "PropertyChanged", G_CALLBACK(
	    on_call_forwarding_manager_property_changed), state, NULL);
}

static void
remove_signal_callbacks(struct call_forwarding_state* state)
{
	dbus_g_proxy_disconnect_signal(state->proxy, "PropertyChanged",
	    state->signalcb_CallForwarding_PropertyChanged, state);
}

/**
 * Handles all forwarding messages (DBUS), used by all forward set functions
 */
static int
forwarding_handling(void* user_ptr, char* modem_path, GValue *val, char *action)
{
	FUNC_ENTER();
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	GError *error = NULL;
	struct call_forwarding_state state;
	state.ofono_data = data;
	state.proxy = NULL;

	state.proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
	    modem_path, OFONO_CALL_FORWARDING_INTERFACE);
	if (!state.proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CALL_FORWARDING_INTERFACE "\n");
		FUNC_LEAVE();
		return -1;
	}

	setup_signal_callbacks(&state);

	if (!org_ofono_CallForwarding_set_property(state.proxy, action, val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		g_object_unref(state.proxy);
		FUNC_LEAVE();

		return -1;
	}
	guint source_id = g_timeout_add(data->timeout, (GSourceFunc) call_forwarding_timeout,
	    &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	remove_signal_callbacks(&state);
	g_object_unref(state.proxy);
	FUNC_LEAVE();
	return 0;

}

int
ofono_call_forwarding_set_unconditional(void* user_ptr, char* modem_path,
    char* number)
{
	FUNC_ENTER();
	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_STRING);
	g_value_set_string(&val, number);
	FUNC_LEAVE();
	return forwarding_handling(user_ptr, modem_path, &val, "VoiceUnconditional");
}

int
ofono_call_forwarding_set_busy(void* user_ptr, char* modem_path, char* number)
{
	FUNC_ENTER();
	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_STRING);
	g_value_set_string(&val, number);
	FUNC_LEAVE();
	return forwarding_handling(user_ptr, modem_path, &val, "VoiceBusy");
}

int
ofono_call_forwarding_set_no_reply(void* user_ptr, char* modem_path,
    char* number)
{
	FUNC_ENTER();
	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_STRING);
	g_value_set_string(&val, number);
	FUNC_LEAVE();
	return forwarding_handling(user_ptr, modem_path, &val, "VoiceNoReply");
}

int
ofono_call_forwarding_set_no_reply_timeout(void* user_ptr, char* modem_path,
    uint timeout)
{
	FUNC_ENTER();
	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_INT);
	g_value_set_int(&val, timeout);
	FUNC_LEAVE();
	return forwarding_handling(user_ptr, modem_path, &val, "VoiceNoReplyTimeout");
}

int
ofono_call_forwarding_set_not_reachable(void* user_ptr, char* modem_path,
    char* number)
{
	FUNC_ENTER();
	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_STRING);
	g_value_set_string(&val, number);
	FUNC_LEAVE();
	return forwarding_handling(user_ptr, modem_path, &val, "VoiceNotReachable");
}

/* Convert generated parameters to test case format. */
void *
forwarding_if_busy_variant_set_arg_processor(struct boxed_value *args,
    void *user_ptr)
{
	FUNC_ENTER();
	char *remote_addr = 0, *forward_addr = 0;
	int user_timeout = 0;
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
	{
		FUNC_LEAVE();
		return 0;
	}

	remote_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	forward_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	user_timeout = strtol(blts_config_boxed_value_get_string(args), NULL, 10);
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->remote_address)
		free(remote_addr);
	else
		data->remote_address = remote_addr;

	if (data->forward_address)
		free(forward_addr);
	else
		data->forward_address = forward_addr;

	if (!data->timeout)
		data->timeout = timeout;

	FUNC_LEAVE();
	return data;
}

void *
forwarding_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	FUNC_ENTER();
	char *forward_addr = 0;
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	forward_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->forward_address)
		free(forward_addr);
	else
		data->forward_address = forward_addr;

	if (!data->timeout)
		data->timeout = timeout;

	FUNC_LEAVE();
	return data;
}

// debug
int
ofono_call_forwarding_properties(void* user_ptr, char* modem_path)
{
	FUNC_ENTER();
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	int retval = 0;
	GHashTable* properties = NULL;

	proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS, modem_path,
	    OFONO_CALL_FORWARDING_INTERFACE);
	if (!proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CALL_FORWARDING_INTERFACE "\n");
		return -1;
	}

	if (!org_ofono_CallForwarding_get_properties(proxy, &properties, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		g_object_unref(proxy);
		retval = -1;
	}
	g_hash_table_foreach(properties, (GHFunc) hash_entry_gvalue_print, NULL);
	g_hash_table_destroy(properties);
	properties = NULL;
	g_object_unref(proxy);
	FUNC_LEAVE();
	return retval;
}

int
ofono_call_forwarding_check_settings(char *forwarding, char *number,
    void* user_ptr, char* modem_path)
{
	FUNC_ENTER()
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	int retval = -1;
	GHashTable* properties = NULL;
	GHashTableIter iter;
	gpointer key, value;
	char *get_for, *get_num;

	proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS, modem_path,
	    OFONO_CALL_FORWARDING_INTERFACE);

	if (!proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CALL_FORWARDING_INTERFACE "\n");
		FUNC_LEAVE();
		return -1;
	}

	if (!org_ofono_CallForwarding_get_properties(proxy, &properties, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		g_object_unref(proxy);
		retval = -1;
		return retval;
	}

	g_hash_table_iter_init(&iter, properties);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		get_for = key;
		get_num = g_value_fits_pointer(value) ? g_value_peek_pointer(value) : NULL;
		if (!g_strcmp0(key, forwarding))
		{
			fprintf(stderr, "comparing key %s to %s\n", (char *) key, forwarding);
			fprintf(stderr, "comparing number %s to %s\n", (char *) get_num, number);
			if (!strcmp(get_num, number))
				retval = 0;
		}
	}

	g_hash_table_destroy(properties);
	properties = NULL;
	g_object_unref(proxy);
	FUNC_LEAVE();
	return retval;
}
