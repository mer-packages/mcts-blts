/* data-context.c -- Dual voicecall cases for blts-ofono

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

#include "ofono-util.h"
#include "ofono-general.h"
#include "signal-marshal.h"
#include "ofono-modem-bindings.h"

#include "data-context.h"
#include <blts_params.h>

struct context_case_state
{
	GMainLoop *mainloop;

	my_ofono_data *ofono_data;

	DBusGProxy* datacontext_proxy;

	GCallback signalcbConnectionContext_PropertyChanged;

	int user_timeout;
	int result;
	char * wished_state;
};

gboolean
org_ofono_ConnectionContext_get_properties(DBusGProxy *proxy,
        GHashTable** OUT_arg0, GError **error)
{
	FUNC_ENTER()
	return dbus_g_proxy_call(proxy, "GetProperties", error, G_TYPE_INVALID,
	        dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
	        OUT_arg0, G_TYPE_INVALID);
}

gboolean
org_ofono_ConnectionContext_set_property(DBusGProxy *proxy,
        const char * IN_arg0, const GValue* IN_arg1, GError **error)

{
	FUNC_ENTER();
	return dbus_g_proxy_call(proxy, "SetProperty", error, G_TYPE_STRING,
	        IN_arg0, G_TYPE_VALUE, IN_arg1, G_TYPE_INVALID, G_TYPE_INVALID);
}

static void
on_connection_context_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key,
        GValue* value, gpointer user_data)
{
	FUNC_ENTER();
	struct context_case_state* state = (struct context_case_state*) user_data;
	if (G_VALUE_HOLDS_BOXED(value))
	{
		BLTS_DEBUG("Context properties changed:\n");
		GHashTable* properties = (GHashTable *) g_value_get_boxed(value);
		if (properties)
		{
			g_hash_table_foreach(properties, (GHFunc) hash_entry_gvalue_print,
			        NULL);
		}
		return;
	}
	else if (G_VALUE_HOLDS_STRING(value))
	{
		BLTS_DEBUG("Data context property: '%s' changed to '%s'\n", key,
				g_value_get_string(value));

		if ((!g_strcmp0(key, "Active")) && (!g_strcmp0(g_value_get_string(value),
		        state->wished_state)))
			g_main_loop_quit(state->ofono_data->mainloop);
	}
}

static void
setup_connectioncontext_callbacks(struct context_case_state* state)
{
	state->signalcbConnectionContext_PropertyChanged = G_CALLBACK(
	        on_connection_context_property_changed);

	dbus_g_proxy_add_signal(state->datacontext_proxy, "PropertyChanged",
	        G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->datacontext_proxy, "PropertyChanged",
	        G_CALLBACK(on_connection_context_property_changed), state, NULL);
}

static void
remove_connectioncontext_callbacks(struct context_case_state* state)
{
	dbus_g_proxy_disconnect_signal(state->datacontext_proxy, "PropertyChanged",
	        state->signalcbConnectionContext_PropertyChanged, state);
}

static gboolean
context_property_timeout(gpointer user_data)
{
	struct context_case_state* state = (struct context_case_state*) user_data;
	state->result = -1;

	BLTS_DEBUG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->ofono_data->mainloop);

	return FALSE;

}

char *
ofono_get_contextpath(char *modem_path, my_ofono_data *data)
{
	FUNC_ENTER()
	GPtrArray *contexts = NULL;
	char * contextpath = NULL;
	GError *error = NULL;
	GValue val = G_VALUE_INIT;

	DBusGProxy *proxyconnman = NULL;
	proxyconnman = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
	        modem_path, OFONO_CONNMAN_INTERFACE);

	if (!proxyconnman)
	{
		BLTS_DEBUG("Failed to open proxy for " OFONO_CONNMAN_INTERFACE "\n");
		FUNC_LEAVE();
		return NULL;
	}

	if (!org_ofono_ConnectionManager_get_contexts(proxyconnman, &contexts,
	        &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		g_object_unref(proxyconnman);
		FUNC_LEAVE();
		return NULL;

	}

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);

	if (!org_ofono_ConnectionManager_set_property(proxyconnman, "Powered",
	        &val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		g_object_unref(proxyconnman);
		g_value_unset(&val);
		FUNC_LEAVE();
		return NULL;
	}
	g_value_unset(&val);
	g_object_unref(proxyconnman);

	if (contexts == NULL)
	{
		BLTS_DEBUG("No available contexts...\n");
		g_ptr_array_free(contexts, TRUE);
		FUNC_LEAVE();
		return NULL;
	}
	GValueArray *pathindex = g_ptr_array_index(contexts, 0);
	contextpath = g_value_get_boxed(g_value_array_get_nth(pathindex, 0));
	FUNC_LEAVE();
	return contextpath;
}

int
ofono_test_data_context(void* user_ptr, __attribute__((unused))int test_num)
{
	FUNC_ENTER()
	int retval = 0;
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	char * contextpath = NULL;
	struct context_case_state state;

	state.result = 0;
	state.ofono_data = data;

	GHashTable* properties = NULL;

	char *modem_path = NULL;
	GError *error = NULL;
	GValue val = G_VALUE_INIT;

	if (my_ofono_get_modem(data))
		return -1;

	if (data->number_modems == 0)
		return -1;

	modem_path = data->modem[0];

	contextpath = ofono_get_contextpath(modem_path, data);
	if (!contextpath)
	{
		BLTS_DEBUG("No contexts found\n");
		return -1;
	}

	state.datacontext_proxy = dbus_g_proxy_new_for_name(data->connection,
	        OFONO_BUS, contextpath, OFONO_CONTEXT_INTERFACE);
	if (!state.datacontext_proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CONTEXT_INTERFACE "\n");
		FUNC_LEAVE();
		return -1;
	}

	setup_connectioncontext_callbacks(&state);

	BLTS_DEBUG("Properties of connection context (first available being used):\n");
	if (!org_ofono_ConnectionContext_get_properties(state.datacontext_proxy,
	        &properties, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		g_object_unref(state.datacontext_proxy);
		remove_connectioncontext_callbacks(&state);
		retval = -1;
	}
	if (properties)
	{
		g_hash_table_foreach(properties, (GHFunc) hash_entry_gvalue_print, NULL);
		g_hash_table_destroy(properties);
	}
	properties = NULL;

	BLTS_DEBUG("Activating gprs\n");

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);
	state.wished_state = "TRUE";

	if (!org_ofono_ConnectionManager_set_property(state.datacontext_proxy,
	        "Active", &val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		remove_connectioncontext_callbacks(&state);
		g_object_unref(state.datacontext_proxy);
		state.datacontext_proxy = NULL;
		retval = -1;
	}

	guint source_id = g_timeout_add(data->timeout,
	        (GSourceFunc) context_property_timeout, &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	BLTS_DEBUG("Deactivating gprs\n");
	g_value_reset(&val);
	g_value_set_boolean(&val, FALSE);
	state.wished_state = "FALSE";
	if (!org_ofono_ConnectionManager_set_property(state.datacontext_proxy,
	        "Active", &val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		remove_connectioncontext_callbacks(&state);
		g_object_unref(state.datacontext_proxy);
		state.datacontext_proxy = NULL;
		retval = -1;
	}
	g_value_unset(&val);
	source_id = g_timeout_add(data->timeout,
	        (GSourceFunc) context_property_timeout, &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	if (state.result)
		retval = state.result;

	remove_connectioncontext_callbacks(&state);
	g_object_unref(state.datacontext_proxy);
	FUNC_LEAVE();
	return retval;
}

int
ofono_test_data_context_download(void* user_ptr,
        __attribute__((unused))int test_num)
{
	FUNC_ENTER()
	int retval = 0;
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	char * contextpath = NULL;
	struct context_case_state state;

	state.result = 0;
	state.ofono_data = data;
	char *modem_path = NULL;
	GError *error = NULL;
	GValue val = G_VALUE_INIT;
	char *command;

	if (my_ofono_get_modem(data))
		return -1;

	if (data->number_modems == 0)
		return -1;

	modem_path = data->modem[0];

	contextpath = ofono_get_contextpath(modem_path, data);
	BLTS_TRACE("Contextpath = %s\n", contextpath);
	if (!contextpath)
	{
		BLTS_DEBUG("No contexts found\n");
		return -1;
	}

	BLTS_TRACE("Timeout=%i\n", data->timeout);
	state.datacontext_proxy = dbus_g_proxy_new_for_name(data->connection,
	        OFONO_BUS, contextpath, OFONO_CONTEXT_INTERFACE);
	if (!state.datacontext_proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CONTEXT_INTERFACE "\n");
		FUNC_LEAVE();
		return -1;
	}

	setup_connectioncontext_callbacks(&state);

	BLTS_DEBUG("Activating gprs\n");

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);
	state.wished_state = "TRUE";

	if (!org_ofono_ConnectionManager_set_property(state.datacontext_proxy,
	        "Active", &val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		remove_connectioncontext_callbacks(&state);
		g_object_unref(state.datacontext_proxy);
		state.datacontext_proxy = NULL;
		return -1;
	}

	guint source_id = g_timeout_add(data->timeout,
	        (GSourceFunc) context_property_timeout, &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	BLTS_DEBUG("Ping test to '%s'\n", data->ping_address);
	// 20 = command 'ping...' length
	command = (char*) malloc(strlen(data->ping_address) + 20);
	if (command == NULL)
	{
		BLTS_DEBUG("OOM");
		return -1;
	}
	sprintf(command, "ping -c 1 -I gprs0 %s", data->ping_address);
	retval = system(command);
	free(command);
	if (retval)
	{
		BLTS_DEBUG("Ping test failed\n");
	}
	else
	{
		BLTS_DEBUG("Ping test complete, data connection verified\n");
	}

	BLTS_DEBUG("Deactivating gprs\n");
	g_value_reset(&val);
	g_value_set_boolean(&val, FALSE);
	state.wished_state = "FALSE";
	if (!org_ofono_ConnectionManager_set_property(state.datacontext_proxy,
	        "Active", &val, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		remove_connectioncontext_callbacks(&state);
		g_object_unref(state.datacontext_proxy);
		state.datacontext_proxy = NULL;
		retval = -1;
	}
	g_value_unset(&val);
	source_id = g_timeout_add(data->timeout,
	        (GSourceFunc) context_property_timeout, &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	if (state.result)
		retval = state.result;

	remove_connectioncontext_callbacks(&state);
	g_object_unref(state.datacontext_proxy);
	FUNC_LEAVE();
	return retval;
}

void *
data_context_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	long timeout = 0;
	char *ping_address = NULL;

	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;
	ping_address = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));

	data->ping_address = ping_address;

	if (!data->timeout)
		data->timeout = timeout;

	return data;
}
