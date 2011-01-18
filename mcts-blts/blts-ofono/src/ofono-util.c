/* ofono-util.c -- oFono utility functions

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

#include <unistd.h>

#include <blts_log.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "ofono-util.h"
#include "ofono-general.h"
#include "signal-marshal.h"
#include "ofono-modem-bindings.h"
#include "org-ofono-Modem.h"
#include "modem-cases.h"

/** general function to verify call state  */

#define MODEM_WAKEUP_TIME	5
#define MODEM_ONLINE_TIME	5

gboolean
check_state(__attribute__((unused))gpointer key, gpointer value, gpointer user_data)
{
	gchar* expected_state = (gchar*) user_data;
	gchar* state = NULL;

	if (!expected_state)
		return FALSE;

	if (strcmp("State", (char *) key) == 0)
	{
		state = (gchar*) g_value_dup_string(value);

		if (!strcmp(expected_state, state))
		{
			g_free(state);
			return TRUE;
		}
		g_free(state);
		return FALSE;
	}
	else
		return FALSE;
}

/** general print/log function for hashtable with GValue*s */

void
hash_entry_gvalue_print(gpointer key, GValue* val,
    __attribute__((unused))  gpointer user_data)
{
	char *str;
	BLTS_DEBUG("[ %s ]\n", key);
	str = g_strdup_value_contents(val);
	BLTS_DEBUG("\t%s\n", str);
	free(str);
}

/* fetch possible modems in system and save to my_ofono_data */
void
get_modem_list(char* data, gpointer user_data)
{
	my_ofono_data* mydata = (my_ofono_data*) user_data;
	if (mydata->number_modems < MAX_MODEMS)
	{
		mydata->modem[mydata->number_modems] = malloc(sizeof(char) * strlen(data)
		    + 1);
		if (mydata->modem[mydata->number_modems] == NULL)
		{
			BLTS_DEBUG("Warning: not enough memory to allocate space for modem data\n");
			return;
		}
		strcpy(mydata->modem[mydata->number_modems], data);
		mydata->number_modems++;
	}
}

void
find_modem(gpointer modem_data, gpointer user_data)
{
	my_ofono_data* mydata = (my_ofono_data*) user_data;
	GValue *tmp_data;

	if (!modem_data)
		return;

	tmp_data = g_value_array_get_nth((GValueArray *) modem_data, 0);
	char * modem = (char *) g_value_peek_pointer(tmp_data);
	BLTS_WARNING("\tModem %s found\n", (char*) modem);
	if (mydata->number_modems < MAX_MODEMS)
	{
		mydata->modem[mydata->number_modems] = malloc(sizeof(char) * strlen(modem)
		    + 1);
		if (mydata->modem[mydata->number_modems] == NULL)
		{
			BLTS_DEBUG("Warning: not enough memory to allocate space for modem data\n");
			return;
		}
		strcpy(mydata->modem[mydata->number_modems], modem);
		mydata->number_modems++;
	}

}

#define G_ENABLE_DEBUG
static void
dbus_marshaller_init()
{
	dbus_g_object_register_marshaller(_blts__VOID__STRING_BOXED, G_TYPE_NONE,
	    G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);

	dbus_g_object_register_marshaller(_blts__VOID__STRING_BOXED, G_TYPE_NONE,
	    DBUS_TYPE_G_OBJECT_PATH, G_TYPE_VALUE, G_TYPE_INVALID);

	/* 	dbus_g_object_register_marshaller(_blts__VOID__STRING_POINTER, */
	/* 		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INVALID); */

	/* 	dbus_g_object_register_marshaller(_blts__VOID__STRING_OBJECT, */
	/* 		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID); */

}

/** initialization for every oFono test case. Finds modems and initialized values */
int
my_ofono_get_modem(my_ofono_data* data)
{
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	error = NULL;
	//GHashTable* list=NULL;
	GPtrArray *modems = NULL;
	int i;

	// initialize values
	data->connection = NULL;
	for (i = 0; i < MAX_MODEMS; i++)
		data->modem[i] = NULL;
	data->number_modems = 0;
	data->connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	data->mainloop = g_main_loop_new(NULL, FALSE);

	if (data->connection == NULL)
	{
		BLTS_DEBUG("Failed to open connection to bus: %s\n", error->message);
		g_error_free(error);
		error = NULL;
		return -1;
	}
	BLTS_DEBUG("connection object is %d\n", data->connection);

	proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
	    OFONO_MNGR_PATH, OFONO_MNGR_INTERFACE);

	if (!proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for " OFONO_MNGR_INTERFACE "\n");
		return -1;
	}

	dbus_marshaller_init();

	if (!org_ofono_Manager_get_modems(proxy, &modems, &error))
	{
		g_object_unref(proxy);
		display_dbus_glib_error(error);
		g_error_free(error);
		return -1;
	}

	g_ptr_array_foreach(modems, find_modem, data);
	g_ptr_array_free(modems, TRUE);
	g_object_unref(proxy);
	if (data->number_modems == 0)
	{
		return -1;
	}

	set_modems_power_on(data);
	set_modems_online(data);

	ensure_calls_cleared(data);

	/* reset here all supplementary services so that device is
	 * on it's 'default' state for sure
	 */
	if (reset_supplementary_services(data))
	{
		BLTS_DEBUG(
		    "Warning: can't reset supplementary services. Wrong PIN or unitialized modem?\n");
	}
	return 0;
}

/* Logs the given GError, with special handling for dbus remote errors */
void
display_dbus_glib_error(GError *error)
{
	if (!error)
		return;

	if (error->domain == DBUS_GERROR && error->code
	    == DBUS_GERROR_REMOTE_EXCEPTION)
	{
		BLTS_ERROR("Caught remote method exception %s : %s\n",
		    dbus_g_error_get_name(error), error->message);
		return;
	}
	BLTS_ERROR("Error: %s\n", error->message);
	return;
}

static void
dump_call_data(gpointer callp, __attribute__((unused))  gpointer unused)
{
	if (!callp)
		return;
	BLTS_WARNING("\t%s\n", (char*) callp);
}

/* Clear all ongoing calls, try to find out if it happened. */
int
ensure_calls_cleared(my_ofono_data *data)
{
	DBusGProxy *vcmgr = NULL;
	GHashTable *properties = NULL;
	GPtrArray *calls = NULL;
	int i, ret = 0;

	if (!data)
		return 0;
	if (data->fl_dontcleanup)
		return 0;
	if (!data->number_modems)
		return 0;

	for (i = 0; i < data->number_modems; ++i)
	{
		vcmgr = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
		    data->modem[i], OFONO_VC_INTERFACE);
		if (!vcmgr)
		{
			BLTS_DEBUG("No voice call manager interface for %s\n", data->modem[i]);
			continue;
		}

		dbus_g_proxy_call(vcmgr, "HangupAll", NULL, G_TYPE_INVALID, G_TYPE_INVALID);

		if (org_ofono_VoiceCallManager_get_properties(vcmgr, &properties, NULL))
		{
			GValue *tmp_calls;

			tmp_calls = g_hash_table_lookup(properties, "Calls");

			if (tmp_calls)
				calls = g_value_get_boxed(tmp_calls);

			if (calls && calls->len && g_ptr_array_index(calls, 0))
			{
				BLTS_WARNING(
				    "Warning: Calls still exist after HangupAll() on %s; call list:\n",
				    data->modem[i]);
				g_ptr_array_foreach(calls, dump_call_data, NULL);
				ret = -1;
			}
		}
		if (properties)
		{
			g_hash_table_unref(properties);
			properties = NULL;
		}

		g_object_unref(vcmgr);
		vcmgr = NULL;
	}
	return ret;
}

int
reset_supplementary_services(my_ofono_data* data)
{
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	char *condition = "all";
	int i, ret = 0;
	if (data->fl_dontcleanup)
		return 0;
	if (!data->number_modems)
		return -1;

	BLTS_DEBUG("{Disabling barrings using PIN number %s}\n", data->barrings_pin);
	for (i = 0; i < data->number_modems; ++i)
	{

		proxy = dbus_g_proxy_new_for_name(data->connection, "org.ofono",
		    data->modem[i], "org.ofono.CallForwarding");
		if (!proxy)
		{
			BLTS_WARNING("Failed to open proxy for org.ofono.CallForwarding \n");
		}

		if (!org_ofono_CallForwarding_disable_all(proxy, condition, &error))
		{
			ret = -1;
			g_error_free(error);
			error = NULL;
		}

		if (!data->barrings_pin)
		{
			BLTS_WARNING(
			    "No pin code given, can't initially reset supplementary services\n");
			break;
		}

		proxy = dbus_g_proxy_new_for_name(data->connection, "org.ofono",
		    data->modem[i], "org.ofono.CallBarring");
		if (!proxy)
		{
			BLTS_WARNING("Failed to open proxy for org.ofono.CallBarring\n");
		}

		if (!org_ofono_CallBarring_disable_all(proxy, data->barrings_pin, &error))
		{
			ret = -1;
			g_error_free(error);
			error = NULL;
		}
	} // all modems loop
	return ret;
}

int
set_modems_power_on(my_ofono_data *data)
{
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	int i;
	int retval = 0;
	GHashTable *props = NULL;
	GValue *variant = NULL;

	GValue val = G_VALUE_INIT;
	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);

	for (i = 0; i < data->number_modems; i++)
	{

		BLTS_DEBUG("{Powering up modem %s}\n", data->modem[i]);
		proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
		    data->modem[i], OFONO_MODEM_INTERFACE);
		if (!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_MODEM_INTERFACE "\n");
			return -1;
		}

		if (!org_ofono_Modem_get_properties(proxy, &props, &error))
		{
			BLTS_DEBUG("Modem powered: Failed to check modem properties\n");
			display_dbus_glib_error(error);
			g_error_free(error);
			retval = -1;
		}

		variant = g_hash_table_lookup(props, "Powered");
		if (g_value_get_boolean(variant) == FALSE)
		{
			if (!org_ofono_Modem_set_property(proxy, "Powered", &val, &error))
			{
				BLTS_DEBUG("Modem powered: Failed to power up modem\n");
				display_dbus_glib_error(error);
				g_error_free(error);
				retval = -1;
			}
			sleep(MODEM_WAKEUP_TIME);
		}
	}
	g_object_unref(proxy);
	return retval;
}

int
set_modems_online(my_ofono_data *data)
{
	DBusGProxy *proxy;
	int i = 0;
	int retval = 0;

	FUNC_ENTER();

	for (i = 0; i < data->number_modems; i++)
	{
		BLTS_DEBUG("{Checking modem %s online status}\n", data->modem[i]);
		GHashTable *properties = NULL;
		GError *error = NULL;
		gboolean online = FALSE;
		GValue *variant = NULL;

		proxy = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
		    data->modem[i], OFONO_MODEM_INTERFACE);

		/* This only happens when we're out of memory, so no point to continue */
		if (!proxy)
		{
			BLTS_ERROR("Failed to create proxy on object %s using iface %s\n",
			    data->modem[i], OFONO_MODEM_INTERFACE);
			retval = -1;
			goto DONE;
		}

		if (!org_ofono_Modem_get_properties(proxy, &properties, &error))
		{
			display_dbus_glib_error(error);
			g_clear_error(&error);
			g_object_unref(proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup(properties, "Online");

		if (!variant)
		{
			BLTS_ERROR(
			    "Set modem online: Modem %s does not support \"Online\" status.\n",
			    data->modem[i]);
			g_object_unref(proxy);
			proxy = NULL;
			g_hash_table_destroy(properties);
			properties = NULL;
			retval = -1;
			continue;
		}
		online = g_value_get_boolean(variant);

		if (online == TRUE)
		{
			g_object_unref(proxy);
			proxy = NULL;
			g_hash_table_destroy(properties);
			properties = NULL;
			continue;
		}
		GValue status = G_VALUE_INIT;
		g_value_init(&status, G_TYPE_BOOLEAN);
		g_value_set_boolean(&status, TRUE);

		if (!org_ofono_Modem_set_property(proxy, "Online", &status, &error))
		{
			display_dbus_glib_error(error);
			g_clear_error(&error);
			g_object_unref(proxy);
			proxy = NULL;
			g_hash_table_destroy(properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		g_hash_table_destroy(properties);
		properties = NULL;

		if (!org_ofono_Modem_get_properties(proxy, &properties, &error))
		{
			display_dbus_glib_error(error);
			g_clear_error(&error);
			g_object_unref(proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup(properties, "Online");

		if (!variant)
		{
			BLTS_ERROR(
			    "Set modem online: Modem %s does not support \"Online\" status.\n",
			    data->modem[i]);
			g_object_unref(proxy);
			proxy = NULL;
			g_hash_table_destroy(properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		online = g_value_get_boolean(variant);

		if (online != TRUE)
		{
			BLTS_ERROR("Set modem online: Can't set %s online.\n", data->modem[i]);
			g_object_unref(proxy);
			proxy = NULL;
			g_hash_table_destroy(properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		g_object_unref(proxy);
		proxy = NULL;
		g_hash_table_destroy(properties);
		properties = NULL;
		sleep(MODEM_ONLINE_TIME);
	}

	DONE: FUNC_LEAVE();

	return retval;
}
