/* call-settings.c -- Call settings test cases

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

#include "call-settings.h"

struct call_settings_case_state
{
	GMainLoop *mainloop;

	my_ofono_data *ofono_data;

	DBusGProxy* callsettings_proxy;

	GCallback signalcbCallSettings_PropertyChanged;

	int user_timeout;
	int result;
};

static void
on_call_settings_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key,
        GValue* value, gpointer user_data)
{
	FUNC_ENTER();
	struct call_settings_case_state* state =
	        (struct call_settings_case_state*) user_data;
	if (G_VALUE_HOLDS_BOXED(value))
	{
		BLTS_DEBUG("Call settings properties changed:\n");
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
		BLTS_DEBUG("call settings property: '%s' changed to '%s'\n", key,
				g_value_get_string(value));
	}
	g_main_loop_quit(state->ofono_data->mainloop);
}

static void
setup_call_setting_callbacks(struct call_settings_case_state* state)
{
	state->signalcbCallSettings_PropertyChanged = G_CALLBACK(
	        on_call_settings_property_changed);

	dbus_g_proxy_add_signal(state->callsettings_proxy, "PropertyChanged",
	        G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(state->callsettings_proxy, "PropertyChanged",
	        G_CALLBACK(on_call_settings_property_changed), state, NULL);
}

static void
remove_call_setting_callbacks(struct call_settings_case_state* state)
{
	dbus_g_proxy_disconnect_signal(state->callsettings_proxy,
	        "PropertyChanged", state->signalcbCallSettings_PropertyChanged,
	        state);
}

static gboolean
call_settings_property_timeout(gpointer user_data)
{
	struct call_settings_case_state* state =
	        (struct call_settings_case_state*) user_data;
	state->result = -1;

	BLTS_DEBUG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->ofono_data->mainloop);

	return FALSE;

}

int
blts_ofono_settings_waiting(void* user_ptr, __attribute__((unused))int test_num)
{
	FUNC_ENTER()
	int retval = 0;
	my_ofono_data* data = (my_ofono_data*) user_ptr;
	struct call_settings_case_state state;

	state.result = 0;
	state.ofono_data = data;

	GHashTable* properties = NULL;
	char *call_waiting_state = NULL;
	char *modem_path = NULL;
	GError *error = NULL;
	GValue val = G_VALUE_INIT;

	if (my_ofono_get_modem(data))
		return -1;

	if (data->number_modems == 0)
		return -1;

	modem_path = data->modem[0];

	BLTS_DEBUG("setting up proxy %s from path %s\n", OFONO_CALL_SETTINGS_INTERFACE, modem_path);

	state.callsettings_proxy = dbus_g_proxy_new_for_name(data->connection,
	        OFONO_BUS, modem_path, OFONO_CALL_SETTINGS_INTERFACE);

	if (!state.callsettings_proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_CALL_SETTINGS_INTERFACE "\n");
		FUNC_LEAVE();
		return -1;
	}

	setup_call_setting_callbacks(&state);

	BLTS_DEBUG("Call setting properties:\n");
	if (!org_ofono_CallSettings_get_properties(state.callsettings_proxy,
	        &properties, &error))
	{
		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		g_object_unref(state.callsettings_proxy);
		remove_call_setting_callbacks(&state);
		retval = -1;
	}
	if (properties)
	{
		call_waiting_state = g_value_dup_string((GValue *)g_hash_table_lookup(properties, "VoiceCallWaiting"));

		g_hash_table_foreach(properties, (GHFunc) hash_entry_gvalue_print, NULL);

		BLTS_DEBUG("Current call waiting state is '%s'\n", call_waiting_state);
		g_value_init(&val, G_TYPE_STRING);
		if(!g_strcmp0(call_waiting_state, "disabled"))
		{
				BLTS_DEBUG("Setting 'call waiting' to enabled\n");
				g_value_set_string(&val, "enabled");
		}
		else
		{
			BLTS_DEBUG("Setting 'call waiting' to disabled\n");
			g_value_set_string(&val, "disabled");
		}
		free(call_waiting_state);
		g_hash_table_destroy(properties);
	}
	else
	{
		BLTS_DEBUG("No properties for voice call available\n");
		remove_call_setting_callbacks(&state);
		return -1;
	}

	properties = NULL;


	if (!org_ofono_CallSettings_set_property(state.callsettings_proxy,
	        "VoiceCallWaiting", &val, &error))
	{
		g_value_unset(&val);

		display_dbus_glib_error(error);
		g_error_free(error);
		error = NULL;
		remove_call_setting_callbacks(&state);
		g_object_unref(state.callsettings_proxy);
		state.callsettings_proxy = NULL;
		retval = -1;
	}

	g_value_unset(&val);

	guint source_id = g_timeout_add(data->timeout,
	        (GSourceFunc) call_settings_property_timeout, &state);

	g_main_loop_run(state.ofono_data->mainloop);

	g_source_remove(source_id);

	if (state.result)
		retval = state.result;

	remove_call_setting_callbacks(&state);
	g_object_unref(state.callsettings_proxy);
	FUNC_LEAVE();
	return retval;
}
