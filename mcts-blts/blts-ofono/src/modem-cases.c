/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* modem-cases.c -- Modem cases for blts-ofono

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
#include <blts_log.h>

/* Own includes */
#include "modem-cases.h"
#include "ofono-util.h"

/* Automatically generated */
#include "org-ofono-Modem.h"

/*
 * Public functions
 */

/* Sets all modems online */
int
blts_ofono_case_modems_online (void *user_data, __attribute__((unused)) int testnum)
{
	DBusGProxy *proxy;
	int i = 0;
	int retval = 0;
	my_ofono_data *data = user_data;

	FUNC_ENTER();

	if (my_ofono_get_modem (data)) {
		retval = -1;
		goto DONE;
	}

	for (i = 0; i < data->number_modems; i++) {
		GHashTable *properties = NULL;
		GError     *error      = NULL;
		gboolean    online     = FALSE;
		GValue     *variant    = NULL;

		BLTS_DEBUG ("{Testing modem %s}\n", data->modem[i]);

		proxy = dbus_g_proxy_new_for_name (data->connection,
						   OFONO_BUS,
						   data->modem[i],
						   OFONO_MODEM_INTERFACE);

		/* This only happens when we're out of memory, so no point continying */
		if (!proxy) {
			BLTS_ERROR ("Failed to create proxy on object %s using iface %s\n",
				    data->modem[i], OFONO_MODEM_INTERFACE);
			retval = -1;
			goto DONE;
		}

		if (!org_ofono_Modem_get_properties (proxy, &properties, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup (properties, "Online");

		if (!variant) {
			BLTS_ERROR ("Modem %s does not support \"Online\" status.\n",
				    data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}
#if 0
		online = g_value_get_boolean (variant);

		if (online == TRUE) {
			BLTS_DEBUG ("{Modem %s already online, skipping}\n", data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			continue;
		}
#endif
		GValue status = G_VALUE_INIT;
		g_value_init (&status, G_TYPE_BOOLEAN);
		g_value_set_boolean (&status, TRUE);

		if (!org_ofono_Modem_set_property (proxy, "Online", &status, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		g_hash_table_destroy (properties);
		properties = NULL;

		/* XXX:
		 * Potential sleep might be needed to give modem time to come online...
		 * ...or then we should monitor the property changed signal
		 */

		if (!org_ofono_Modem_get_properties (proxy, &properties, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup (properties, "Online");

		if (!variant) {
			BLTS_ERROR ("Modem %s does not support \"Online\" status.\n",
				    data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		online = g_value_get_boolean (variant);

		if (online != TRUE) {
			BLTS_ERROR ("Setting modem %s online failed.\n", data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		BLTS_DEBUG ("{Modem %s success}\n", data->modem[i]);

		g_object_unref (proxy);
		proxy = NULL;
		g_hash_table_destroy (properties);
		properties = NULL;
	}

DONE:
	FUNC_LEAVE();

	return retval;
}

/* Sets all modems offline */
int
blts_ofono_case_modems_offline (void *user_data, int testnum)
{
	DBusGProxy *proxy;
	int i = 0;
	int retval = 0;
	my_ofono_data *data = user_data;

	FUNC_ENTER();

	if (my_ofono_get_modem (data)) {
		retval = -1;
		goto DONE;
	}

	for (i = 0; i < data->number_modems; i++) {
		GHashTable *properties = NULL;
		GError     *error      = NULL;
		gboolean    online     = FALSE;
		GValue     *variant    = NULL;

		BLTS_DEBUG ("{Testing modem %s}\n", data->modem[i]);

		proxy = dbus_g_proxy_new_for_name (data->connection,
						   OFONO_BUS,
						   data->modem[i],
						   OFONO_MODEM_INTERFACE);

		/* This only happens when we're out of memory, so no point continying */
		if (!proxy) {
			BLTS_ERROR ("Failed to create proxy on object %s using iface %s\n",
				    data->modem[i], OFONO_MODEM_INTERFACE);
			retval = -1;
			goto DONE;
		}

		if (!org_ofono_Modem_get_properties (proxy, &properties, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup (properties, "Online");

		if (!variant) {
			BLTS_ERROR ("Modem %s does not support \"Online\" status.\n",
				    data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}
#if 0
		online = g_value_get_boolean (variant);

		if (online == FALSE) {
			BLTS_DEBUG ("{Modem %s already offline, skipping}\n", data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			continue;
		}
#endif
		GValue status = G_VALUE_INIT;
		g_value_init (&status, G_TYPE_BOOLEAN);
		g_value_set_boolean (&status, FALSE);

		if (!org_ofono_Modem_set_property (proxy, "Online", &status, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		g_hash_table_destroy (properties);
		properties = NULL;

		/* XXX:
		 * Potential sleep might be needed to give modem time to go offline...
		 * ...or then we should monitor the property changed signal
		 */

		if (!org_ofono_Modem_get_properties (proxy, &properties, &error)) {
			display_dbus_glib_error (error);
			g_clear_error (&error);
			g_object_unref (proxy);
			proxy = NULL;
			retval = -1;
			continue;
		}

		variant = g_hash_table_lookup (properties, "Online");

		if (!variant) {
			BLTS_ERROR ("Modem %s does not support \"Online\" status.\n",
				    data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		online = g_value_get_boolean (variant);

		if (online == TRUE) {
			BLTS_ERROR ("Setting modem %s offline failed.\n", data->modem[i]);
			g_object_unref (proxy);
			proxy = NULL;
			g_hash_table_destroy (properties);
			properties = NULL;
			retval = -1;
			continue;
		}

		BLTS_DEBUG ("{Modem %s success}\n", data->modem[i]);

		g_object_unref (proxy);
		proxy = NULL;
		g_hash_table_destroy (properties);
		properties = NULL;
	}

	if (!data->fl_dontcleanup) {
		blts_ofono_case_modems_online (user_data, testnum);
	}

DONE:
	FUNC_LEAVE();

	return retval;
}
