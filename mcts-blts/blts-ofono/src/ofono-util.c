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

/** general print/log function for hashtable with GValue*s */

void hash_entry_gvalue_print(gpointer key, GValue* val, __attribute__((unused)) gpointer user_data)
{
	char *str;
	LOG("[ %s ]\n", key);
	str = g_strdup_value_contents (val);
	LOG("\t%s\n", str);
	free(str);
}

/* fetch possible modems in system and save to my_ofono_data */
void get_modem_list(char* data, gpointer user_data)
{
	my_ofono_data* mydata = (my_ofono_data*)user_data;
	if(mydata->number_modems < MAX_MODEMS)
	{
		mydata->modem[mydata->number_modems] = malloc(sizeof(char)*strlen(data)+1);
		if(mydata->modem[mydata->number_modems] == NULL)
		{
			LOG("Warning: not enough memory to allocate space for modem data\n");
			return;
		}
		strcpy(mydata->modem[mydata->number_modems], data);
		mydata->number_modems++;
	}
}

void find_modem(gpointer modem_data,  gpointer user_data)
{
	my_ofono_data* mydata = (my_ofono_data*)user_data;
	GValue *tmp_data;

	if (!modem_data)
		return;

	tmp_data = g_value_array_get_nth ((GValueArray *)modem_data, 0);
	char * modem = (char *)g_value_peek_pointer(tmp_data);
	BLTS_WARNING("\tModem %s found\n", (char*) modem);
	if(mydata->number_modems < MAX_MODEMS)
	{
		mydata->modem[mydata->number_modems] = malloc(sizeof(char)*strlen(modem)+1);
		if(mydata->modem[mydata->number_modems] == NULL)
		{
			LOG("Warning: not enough memory to allocate space for modem data\n");
			return;
		}
		strcpy(mydata->modem[mydata->number_modems], modem);
		mydata->number_modems++;
	}

}


#define G_ENABLE_DEBUG
static void dbus_marshaller_init()
{
 	dbus_g_object_register_marshaller(_blts__VOID__STRING_BOXED,
 		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);

 	dbus_g_object_register_marshaller(_blts__VOID__STRING_BOXED,
 		G_TYPE_NONE, DBUS_TYPE_G_OBJECT_PATH, G_TYPE_VALUE, G_TYPE_INVALID);

/* 	dbus_g_object_register_marshaller(_blts__VOID__STRING_POINTER, */
/* 		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INVALID); */

/* 	dbus_g_object_register_marshaller(_blts__VOID__STRING_OBJECT, */
/* 		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID); */

}


/** initialization for every oFono test case. Finds modems and initialized values */
int my_ofono_get_modem(my_ofono_data* data)
{
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	error = NULL;
	//GHashTable* list=NULL;
	GPtrArray *modems = NULL;
	int i;

	// initialize values
	data->connection = NULL;
	for(i=0; i<MAX_MODEMS; i++)
		data->modem[i] = NULL;
	data->number_modems = 0;
	data->connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	data->mainloop = g_main_loop_new(NULL, FALSE);

	if (data->connection == NULL)
	{
		LOG ("Failed to open connection to bus: %s\n",
				error->message);
		g_error_free (error);
		error = NULL;
		return -1;
	}
	LOG("connection object is %d\n", data->connection);

	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										OFONO_MNGR_PATH,
										OFONO_MNGR_INTERFACE);

	if(!proxy)
	{
		LOG ("Failed to open proxy for " OFONO_MNGR_INTERFACE "\n");
		return -1;
	}

	dbus_marshaller_init();

	if(!org_ofono_Manager_get_modems(proxy, &modems, &error))
	{
		g_object_unref (proxy);
		display_dbus_glib_error(error);
		g_error_free (error);
		return -1;
	}


	g_ptr_array_foreach(modems, find_modem, data);
	g_ptr_array_free(modems, TRUE);
	g_object_unref (proxy);
	if(data->number_modems==0)
	{
		return -1;
	}

	ensure_calls_cleared(data);

	/* reset here all supplementary services so that device is
	 * on it's 'default' state for sure
	 */
	if(reset_supplementary_services(data))
	{
		LOG("Warning: can't reset supplementary services. Wrong PIN or unitialized modem?\n");
	}
	return 0;
}

/* Logs the given GError, with special handling for dbus remote errors */
void display_dbus_glib_error(GError *error)
{
	if (!error)
		return;

	if (error->domain == DBUS_GERROR
		&& error->code == DBUS_GERROR_REMOTE_EXCEPTION) {
		BLTS_ERROR("Caught remote method exception %s : %s\n",
			dbus_g_error_get_name(error), error->message);
			return;
	}
	BLTS_ERROR("Error: %s\n", error->message);
	return;
}

static void dump_call_data(gpointer callp, __attribute__((unused)) gpointer unused)
{
	if (!callp)
		return;
	BLTS_WARNING("\t%s\n", (char*) callp);
}

/* Clear all ongoing calls, try to find out if it happened. */
int ensure_calls_cleared(my_ofono_data *data)
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

	for (i = 0; i < data->number_modems; ++i) {
		vcmgr = dbus_g_proxy_new_for_name(data->connection, OFONO_BUS,
			data->modem[i], OFONO_VC_INTERFACE);
		if (!vcmgr) {
			BLTS_DEBUG("No voice call manager interface for %s\n",
				data->modem[i]);
			continue;
		}

		dbus_g_proxy_call(vcmgr, "HangupAll", NULL, G_TYPE_INVALID,
				G_TYPE_INVALID);

		if (org_ofono_VoiceCallManager_get_properties(vcmgr,
			&properties, NULL)) {
                    GValue *tmp_calls;

                    tmp_calls = g_hash_table_lookup(properties, "Calls");

                    if (tmp_calls) calls = g_value_get_boxed (tmp_calls);

                        if (calls && calls->len && g_ptr_array_index(calls, 0)) {
				BLTS_WARNING("Warning: Calls still exist after HangupAll() on %s; call list:\n",
					data->modem[i]);
				g_ptr_array_foreach(calls, dump_call_data, NULL);
				ret = -1;
			}
		}
		if (properties) {
			g_hash_table_unref(properties);
			properties = NULL;
		}

		g_object_unref(vcmgr);
		vcmgr = NULL;
	}
	return ret;
}

int reset_supplementary_services(my_ofono_data* data)
{
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	char *condition = "all";
	int i, ret = 0;
	if (!data->number_modems)
		return -1;

	for (i = 0; i < data->number_modems; ++i) {



		proxy = dbus_g_proxy_new_for_name (data->connection,
											"org.ofono",
											data->modem[i],
											"org.ofono.CallForwarding");
		if(!proxy)
		{
			BLTS_WARNING("Failed to open proxy for org.ofono.CallForwarding \n");
		}

		if(!org_ofono_CallForwarding_disable_all(proxy, condition, &error))
		{
			ret = -1;
			g_error_free (error);
			error = NULL;
		}

		if(!data->old_pin)
		{
			BLTS_WARNING("No pin code given, can't initially reset supplementary services\n");
			break;
		}

		proxy = dbus_g_proxy_new_for_name (data->connection,
											"org.ofono",
											data->modem[i],
											"org.ofono.CallBarring");
		if(!proxy)
		{
			BLTS_WARNING ("Failed to open proxy for org.ofono.CallBarring\n");
		}

		if(!org_ofono_CallBarring_disable_all(proxy, data->old_pin, &error))
		{
			ret = -1;
			g_error_free (error);
			error = NULL;
		}
	} // all modems loop
	return ret;
}
