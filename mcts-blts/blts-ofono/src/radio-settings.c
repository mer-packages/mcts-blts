/* radio-cases.c -- Radio mode settings

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

/* Project includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <blts_log.h>

/* Own includes */
#include "radio-settings.h"

#include "ofono-modem-bindings.h"


#include "ofono-util.h"
#include "ofono-general.h"

#include "signal-marshal.h"

struct RAT_state
{
	my_ofono_data* data;
	DBusGProxy* proxy;

	GCallback signalcb_RadioTechnology_PropertyChanged;

	gint result;
};


static void
on_rat_property_changed (__attribute__((unused))DBusGProxy *proxy, char *key,
	    GValue* value, gpointer userdata);
static gboolean
rat_timeout(gpointer user_data);

int rat_show_properties(DBusGProxy *proxy)
{
	GHashTable *properties = NULL;
	GError     *error      = NULL;
	if (!org_ofono_RadioSettings_get_properties(proxy, &properties, &error))
	{
		display_dbus_glib_error (error);
		g_clear_error (&error);
		g_object_unref (proxy);
		error = NULL;
		proxy = NULL;
		return -1;
	}

	if (properties)
	{
		g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		g_hash_table_destroy(properties);
	}
	return 0;
}


int my_ofono_chage_radio_technology(void* user_ptr, __attribute__((unused)) int test_num)
{
	my_ofono_data *data = user_ptr;
	int i;

	GError     *error = NULL;

	struct RAT_state state;
	state.data = data;
	state.proxy = NULL;

	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{

		BLTS_DEBUG ("{Testing modem %s}\n", data->modem[i]);

		state.proxy = dbus_g_proxy_new_for_name (state.data->connection,
						   OFONO_BUS,
						   state.data->modem[i],
						   OFONO_RADIO_INTERFACE);
		/* This only happens when we're out of memory, so no point to continue */
		if (!state.proxy)
		{
			BLTS_ERROR ("Failed to create proxy on object %s using iface %s\n",
					data->modem[i], OFONO_RADIO_INTERFACE);
			state.result = -1;
			goto DONE;
		}

		BLTS_DEBUG("Trying to change radio technology to '%s'\n", state.data->RAT);

		state.signalcb_RadioTechnology_PropertyChanged= G_CALLBACK(
				on_rat_property_changed);
		dbus_g_proxy_add_signal(state.proxy, "PropertyChanged", G_TYPE_STRING,
		    G_TYPE_VALUE, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(state.proxy, "PropertyChanged",
		    state.signalcb_RadioTechnology_PropertyChanged, &state, 0);

		GValue val = G_VALUE_INIT;
		g_value_init(&val, G_TYPE_STRING);
		g_value_set_string(&val, state.data->RAT);

		if (!org_ofono_RadioSettings_set_property(state.proxy, "TechnologyPreference", &val, &error))
		{
			display_dbus_glib_error(error);
			g_error_free(error);
			g_object_unref(state.proxy);
			return -1;
		}

		guint source_id = g_timeout_add(state.data->timeout, (GSourceFunc)rat_timeout,
		    &state);

		g_main_loop_run(state.data->mainloop);

		g_source_remove(source_id);

		rat_show_properties(state.proxy);

		dbus_g_proxy_disconnect_signal(state.proxy, "PropertyChanged",
		    state.signalcb_RadioTechnology_PropertyChanged, &state);

		g_object_unref(state.proxy);
	}

DONE:
	return state.result;
}


/* Timeout callback */
static gboolean
rat_timeout(gpointer user_data)
{
	struct RAT_state* state = (struct RAT_state*) user_data;
	state->result = -1;

	BLTS_DEBUG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->data->mainloop);

	return FALSE;
}


static void
on_rat_property_changed (__attribute__((unused))DBusGProxy *proxy, char *key,
	    GValue* value, gpointer userdata)
{
	FUNC_ENTER();
	struct RAT_state* state = (struct RAT_state*) userdata;

	BLTS_DEBUG("Call forwarding property: '%s' changed to '%s'\n", key,
	    g_strdup_value_contents(value));

	BLTS_DEBUG("RAT property changed\n");
	state->result = 0;
	g_main_loop_quit(state->data->mainloop);
	FUNC_LEAVE();
}

void *radio_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *RAT = NULL;
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

	if (!data)
		return 0;

	RAT = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));
	if (!data->RAT)
		data->RAT = RAT;
	if (!data->timeout)
		data->timeout = timeout;

	return data;
}

