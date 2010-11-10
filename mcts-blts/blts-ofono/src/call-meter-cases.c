/* call-meter-cases.c -- Call meter cases for blts-ofono

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

#include <glib.h>
#include <blts_log.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdlib.h>
#include <glib-object.h>

#include "call-meter-cases.h"
#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"
#include "ofono-general.h"
#include "ofono-util.h"

struct call_meter_case_state {
	char* address;
	char* pin;

	char* initial_call_meter;
	char* initial_accu_call_meter;
	char* initial_accu_call_meter_max;
	char* initial_price_per_unit;
	char* initial_currency;

	char* new_accu_call_meter_max;
	char* new_price_per_unit;
	char* new_currency;

	GMainLoop *mainloop;

	DBusGProxy *call_meter;
	DBusGProxy *voice_call_manager;

	GSourceFunc case_begin;

	my_ofono_data *ofono_data;

	GCallback signalcb_CallMeter_PropertyChanged;
	GCallback signalcb_CallMeter_NearMaximumWarning;

	int user_timeout;
	gboolean warning_received;
	int result;
};

/**
 * Signal handler
 * Can be used to check if call meter state changes
 */

static void on_call_meter_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key, GValue* value, __attribute__((unused))gpointer user_data)
{
	char* value_str = g_strdup_value_contents (value);

	LOG("Callmeter property: %s changed to %s\n ", key, value_str);

	g_free(value_str);
}

/* NearMaximumWarning is emitted shortly before the ACM maximum value is reached.
 * The warning is issued approximately when 30 seconds call time
 * remains or when starting a call with less than 30 seconds remaining.
 */
static void on_call_meter_near_max_warning(__attribute__((unused))DBusGProxy *proxy, gpointer user_data)
{
	struct call_meter_case_state *state = (struct call_meter_case_state *) user_data;

	LOG("NearMaximumWarning signal received\n");

	state->warning_received = TRUE;
}


GHashTable* get_call_meter_properties(DBusGProxy *proxy)
{
	GError *error = NULL;
	GHashTable* list = NULL;

	if(!proxy)
		return NULL;

	if(!org_ofono_CallMeter_get_properties(proxy, &list, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return NULL;
	}
	return list;
}

static gboolean
check_call_meter_value (GHashTable* properties, gpointer key, gpointer expected, gpointer user_data)
{
	struct call_meter_case_state *state = (struct call_meter_case_state *) user_data;
	gpointer *value = g_hash_table_lookup ((GHashTable*)properties, key);

    if (!value)
	{
		LOG("Value for key:%s not found\n", key);
        return FALSE;
    }

	if(strcmp(key, "CallMeter") == 0)
	{
		guint32 current = g_value_get_uint((GValue*) value);
		LOG("CallMeter current value:%d\n", current);
		/* if expected is given compare against that */
		if((gint)expected != -1)
		{
			LOG("CallMeter expected value:%d\n", (guint32)expected);
			return ((guint32)expected == current) ? TRUE : FALSE;
		}
		/* else CallMeter value must be bigger than the initial value */
		else
		{
			guint32 initial = atoi(state->initial_call_meter);
			return (initial < current) ? TRUE : FALSE;
		}
	}
	else if(strcmp(key, "AccumulatedCallMeter") == 0)
	{
		guint32 current = g_value_get_uint((GValue*)value);
		LOG("AccumulatedCallMeter current value:%d\n", current);
		/* if expected is given compare against that */
		if((gint)expected != -1)
		{
			LOG("AccumulatedCallMeter expected value:%d\n", (guint32)expected);
			return ((guint32)expected == current) ? TRUE : FALSE;
		}
		/* else CallMeter value must be bigger than the initial value */
		else
		{
			guint32 initial = atoi(state->initial_accu_call_meter);
			return (initial < current) ? TRUE : FALSE;
		}
	}
	else if (strcmp(key, "AccumulatedCallMeterMaximum") == 0)
	{
		guint32 initial = atoi(state->initial_accu_call_meter_max);
		guint32 current = g_value_get_uint((GValue*)value);
		guint32 exp = atoi(expected);

		LOG("AccumulatedCallMeterMaximum initial value:%d\n", initial);
		LOG("AccumulatedCallMeterMaximum current value:%d\n", current);
		LOG("AccumulatedCallMeterMaximum expected value:%d\n", exp);

		return (exp == current) ? TRUE : FALSE;
	}
	else if (strcmp(key, "PricePerUnit") == 0)
	{
		gdouble initial = atof(state->initial_price_per_unit);
		gdouble current = g_value_get_double((GValue*)value);

		LOG("PricePerUnit initial value:%f\n", initial);
		LOG("PricePerUnit current value:%f\n", current);
		LOG("PricePerUnit expected value:%f\n", atof(expected));

		return (atof(expected) == current) ? TRUE : FALSE;
	}
	else if (strcmp(key, "Currency") == 0)
	{
		const gchar* current = g_value_get_string((GValue*)value);

		LOG("Currency initial value:%s\n", state->initial_currency);
		LOG("Currency current value:%s\n", current);
		LOG("Currency expected value:%s\n", (gchar*) expected);

		if (strcmp(current, (gchar*)expected) == 0)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		LOG("new property:%s?\n", key);
	}
    return  FALSE;
}

static void
get_initial_value (gpointer key, gpointer value, gpointer data)
{
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if(!state)
		return;

	if(strcmp(key, "CallMeter") == 0)
	{
		state->initial_call_meter = g_strdup_value_contents (value);
		LOG("Save initial CallMeter %s (%s)\n", state->initial_call_meter, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "AccumulatedCallMeter") == 0)
	{
		state->initial_accu_call_meter = g_strdup_value_contents (value);
		LOG("Save initial AccumulatedCallMeter %s (%s)\n", state->initial_accu_call_meter, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "AccumulatedCallMeterMaximum") == 0)
	{
		state->initial_accu_call_meter_max =  g_strdup_value_contents (value);
		LOG("Save initial AccumulatedCallMeterMaximum %s (%s)\n", state->initial_accu_call_meter_max, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "PricePerUnit") == 0)
	{
		state->initial_price_per_unit =  g_strdup_value_contents (value);
		LOG("Save initial PricePerUnit %s (%s)\n", state->initial_price_per_unit, G_VALUE_TYPE_NAME(value));
	}
	else if (strcmp(key, "Currency") == 0)
	{
		state->initial_currency =  g_value_dup_string (value);
		LOG("Save initial Currency %s (%s)\n", state->initial_currency, G_VALUE_TYPE_NAME(value));
	}
	else
	{
		LOG("new property:%s?\n", key);
	}
}

static void restore_call_meter_values(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	GError *error = NULL;

	if(strlen(state->new_accu_call_meter_max) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_UINT);
		g_value_set_uint(new_value, atoi(state->initial_accu_call_meter_max));

		LOG("Restore AccumulatedCallMeterMaximum property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "AccumulatedCallMeterMaximum",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}
	}

	if(strlen(state->new_price_per_unit) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_DOUBLE);
		g_value_set_double(new_value, atof(state->initial_price_per_unit));

		LOG("Restore PricePerUnit property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "PricePerUnit",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}

	}

	if(strlen(state->new_currency) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_STRING);
		g_value_set_string(new_value, state->initial_currency);

		LOG("Restore Currency property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "Currency",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
		}
	}
	FUNC_LEAVE();
}

static void call_meter_state_finalize(struct call_meter_case_state *state)
{
	if (!state)
		return;

	if (state->call_meter)
		g_object_unref(state->call_meter);

	if (state->voice_call_manager)
		g_object_unref(state->voice_call_manager);

	if (state->address)
		free(state->address);
	if (state->pin)
		free(state->pin);

	if (state->initial_call_meter)
		free(state->initial_call_meter);
	if (state->initial_accu_call_meter)
		free(state->initial_accu_call_meter);
	if (state->initial_accu_call_meter_max)
		free(state->initial_accu_call_meter_max);
	if (state->initial_price_per_unit)
		free(state->initial_price_per_unit);
	if (state->initial_currency)
		free(state->initial_currency);

	if (state->new_accu_call_meter_max)
		free(state->new_accu_call_meter_max);
	if (state->new_price_per_unit)
		free(state->new_price_per_unit);
	if (state->new_currency)
		free(state->new_currency);

	free(state);
}

static gboolean call_meter_read_user_timeout(gpointer data)
{
	FUNC_ENTER();
	GError *error = NULL;
	GHashTable* properties = NULL;
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	LOG("Read user timeout reached\n");

	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		state->result = -1;
	}
	else
	{
		properties = get_call_meter_properties(state->call_meter);

		if(properties)
		{
			if (!check_call_meter_value (properties, "CallMeter", (gpointer)-1, data))
			{
				LOG("CallMeter value is not increased - test failed!\n");
				state->result = -1;
			}
			else
			{
				state->result = 0;
			}

			g_hash_table_destroy(properties);
		}
		else
		{
			state->result = -1;
		}

	}
	g_main_loop_quit(state->mainloop);

	return FALSE;
}

static gboolean call_meter_set_user_timeout(gpointer data)
{
	FUNC_ENTER();

	char* acm_max;
	char* ppu;
	char* currency;

	GError *error = NULL;
	GHashTable* properties = NULL;
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	LOG("Set user timeout reached\n");
	g_main_loop_quit(state->mainloop);

	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		goto error;
	}

	properties = get_call_meter_properties(state->call_meter);

	if(!properties)
		goto error;

	if(strlen(state->new_accu_call_meter_max) != 0)
		acm_max = state->new_accu_call_meter_max;
	else
		acm_max = state->initial_accu_call_meter_max;

	if(strlen(state->new_price_per_unit) != 0)
		ppu = state->new_price_per_unit;
	else
		ppu = state->initial_price_per_unit;

	if(strlen(state->new_currency) != 0)
		currency = state->new_currency;
	else
		currency = state->initial_currency;

	if (!check_call_meter_value (properties, "AccumulatedCallMeterMaximum", (gpointer*)acm_max, data) ||
		!check_call_meter_value (properties, "PricePerUnit", (gpointer*)ppu, data) ||
		!check_call_meter_value (properties, "Currency", (gpointer*)currency, data) )
	{
		LOG("CallMeter values are not expected ones - test failed!\n");
		goto error;
	}

	state->result = 0;
	restore_call_meter_values(state);
	g_hash_table_destroy(properties);
	g_main_loop_quit(state->mainloop);

	return FALSE;

error:

	state->result = -1;
	restore_call_meter_values(state);

	if(properties)
		g_hash_table_destroy(properties);

	g_main_loop_quit(state->mainloop);

	return FALSE;
}

static gboolean call_meter_reset_user_timeout(gpointer data)
{
	FUNC_ENTER();

	GError *error = NULL;
	GHashTable* properties = NULL;
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	LOG("Reset user timeout reached\n");
	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		goto error;
	}

	LOG("Resetting call meter data using PIN:%s\n", state->pin);
	if(!org_ofono_CallMeter_reset(state->call_meter, state->pin,  &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		goto error;
	}

	properties = get_call_meter_properties(state->call_meter);

	if(!properties)
		goto error;

	if (!check_call_meter_value (properties, "AccumulatedCallMeter", 0, data))
	{
		LOG("CallMeter values are not reset - test failed!\n");
		goto error;
	}

	state->result = 0;

	g_hash_table_destroy(properties);
	g_main_loop_quit(state->mainloop);

	return FALSE;

error:

	state->result = -1;

	if(properties)
		g_hash_table_destroy(properties);

	g_main_loop_quit(state->mainloop);

	return FALSE;
}

static gboolean call_meter_near_max_warning_user_timeout(gpointer data)
{
	FUNC_ENTER();

	GError *error = NULL;
	GHashTable* properties = NULL;
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	LOG("Near max warning user timeout reached\n");
	if(!org_ofono_VoiceCallManager_hangup_all(state->voice_call_manager, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		goto error;
	}

	if(state->warning_received)
	{
		state->result = 0;
	}
	else
	{
		LOG("NearMaximumWarning not received - test failed!\n");
		state->result = -1;
	}

	restore_call_meter_values(state);

	g_hash_table_destroy(properties);
	g_main_loop_quit(state->mainloop);

	return FALSE;

error:

	state->result = -1;

	if(properties)
		g_hash_table_destroy(properties);

	g_main_loop_quit(state->mainloop);

	return FALSE;
}

static gboolean call_master_timeout(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	state->result = -1;

	LOG("Timeout reached, failing test.\n");

	g_main_loop_quit(state->mainloop);
	return FALSE;
}

static gboolean call_meter_init_start(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	DBusGProxy *call_meter;
	DBusGProxy *voice_call_manager;

	voice_call_manager = dbus_g_proxy_new_for_name (state->ofono_data->connection,
											OFONO_BUS,
											state->ofono_data->modem[0],
											OFONO_VC_INTERFACE);

	if (!voice_call_manager){
		LOG("Cannot get proxy for " OFONO_VC_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}
	state->voice_call_manager = voice_call_manager;

	call_meter = dbus_g_proxy_new_for_name (state->ofono_data->connection,
											OFONO_BUS,
											state->ofono_data->modem[0],
											OFONO_METER_INTERFACE);

	if (!call_meter) {
		LOG("Cannot get proxy for " OFONO_METER_INTERFACE "\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	state->call_meter = call_meter;

	if (state->signalcb_CallMeter_PropertyChanged) {
				dbus_g_proxy_add_signal(state->call_meter, "PropertyChanged",
						G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
				dbus_g_proxy_connect_signal(state->call_meter, "PropertyChanged",
					state->signalcb_CallMeter_PropertyChanged, data, 0);
			}

	if (state->case_begin)
		g_idle_add(state->case_begin, state);

	FUNC_LEAVE();
	return FALSE;
}

static
gboolean save_initial_values(struct call_meter_case_state *state)
{
	FUNC_ENTER();
	GHashTable* properties;

	if(!state)
		return FALSE;

	properties = get_call_meter_properties(state->call_meter);

	if(!properties)
		return FALSE;

	LOG("Get initial values\n");
	g_hash_table_foreach(properties, (GHFunc)get_initial_value, state);
	g_hash_table_destroy(properties);
	properties = NULL;

	return TRUE;
}

static void read_call_meter_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();

	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	g_timeout_add(state->user_timeout, (GSourceFunc) call_meter_read_user_timeout, state);

}

static void set_call_meter_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();

	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	g_timeout_add(state->user_timeout, (GSourceFunc) call_meter_set_user_timeout, state);

}

static void reset_call_meter_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();

	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	g_timeout_add(state->user_timeout, (GSourceFunc) call_meter_reset_user_timeout, state);

}

static void near_max_warning_call_complete(__attribute__((unused)) DBusGProxy *proxy, __attribute__((unused))char* call_path, GError *error, gpointer data)
{
	FUNC_ENTER();

	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if (error) {
		LOG("Call failure: %s\n", error->message);
		state->result = 1;
		g_main_loop_quit(state->mainloop);
		return;
	}

	g_timeout_add(state->user_timeout, (GSourceFunc) call_meter_near_max_warning_user_timeout, state);

}
static gboolean read_call_meter_start(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", read_call_meter_call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
}

static gboolean set_call_meter_start(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	GError *error = NULL;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	if(strlen(state->new_accu_call_meter_max) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_UINT);
		g_value_set_uint(new_value, atoi(state->new_accu_call_meter_max));

		LOG("Set AccumulatedCallMeterMaximum property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "AccumulatedCallMeterMaximum",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			goto error;
		}
	}

	if(strlen(state->new_price_per_unit) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_DOUBLE);
		g_value_set_double(new_value, atof(state->new_price_per_unit));

		LOG("Set PricePerUnit property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "PricePerUnit",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			goto error;
		}

	}

	if(strlen(state->new_currency) != 0)
	{
		GValue* new_value = malloc(sizeof *(new_value));
		memset(new_value, 0, sizeof *(new_value));
		g_value_init(new_value, G_TYPE_STRING);
		g_value_set_string(new_value, state->new_currency);

		LOG("Set Currency property...\n");

		if(!org_ofono_CallMeter_set_property (state->call_meter, "Currency",  new_value, state->pin, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			goto error;
		}
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", set_call_meter_call_complete, state);

	LOG("Starting call to %s\n", state->address);


	FUNC_LEAVE();
	return FALSE;

error:

	state->result = -1;
	g_main_loop_quit(state->mainloop);
	return FALSE;

}

static gboolean reset_call_meter_start(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", reset_call_meter_call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
}

static gboolean near_max_warning_start(gpointer data)
{
	FUNC_ENTER();
	struct call_meter_case_state *state = (struct call_meter_case_state *) data;

	GError *error = NULL;

	if(!save_initial_values(state))
	{
		LOG("Save initial values failed!\n");
		state->result = -1;
		g_main_loop_quit(state->mainloop);
		return FALSE;
	}

	GValue* new_value = malloc(sizeof *(new_value));
	memset(new_value, 0, sizeof *(new_value));
	g_value_init(new_value, G_TYPE_UINT);
	g_value_set_uint(new_value, 10);

	LOG("Set AccumulatedCallMeterMaximum property to be < 30 sec...\n");
	if(!org_ofono_CallMeter_set_property (state->call_meter, "AccumulatedCallMeterMaximum",  new_value, state->pin, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		goto error;
	}

	org_ofono_VoiceCallManager_dial_async(state->voice_call_manager,
		state->address, "", near_max_warning_call_complete, state);

	LOG("Starting call to %s\n", state->address);

	FUNC_LEAVE();
	return FALSE;
error:
	state->result = -1;
	g_main_loop_quit(state->mainloop);
	return FALSE;
}


static struct call_meter_case_state *call_meter_state_init(my_ofono_data *data)
{
	struct call_meter_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		log_print("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->ofono_data = data;
	return state;
}


static int call_meter_case_run(struct call_meter_case_state *state)
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

	state->call_meter = NULL;
	state->voice_call_manager = NULL;

	g_timeout_add(state->ofono_data->timeout, (GSourceFunc) call_master_timeout, state);

	g_idle_add(call_meter_init_start, state);

	state->result=-1;

	BLTS_TRACE("Starting main loop.\n");
	g_main_loop_run(state->mainloop);
	BLTS_TRACE("Back from main loop.\n");

	ret = state->result;

done:

	return ret;
}

/* Test cases ------------> */

int blts_ofono_read_call_meter_data(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_meter_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	/* log_set_level(5); */

	test = call_meter_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->user_timeout = data->user_timeout ? data->user_timeout : 5000;
	test->signalcb_CallMeter_PropertyChanged =
		G_CALLBACK(on_call_meter_property_changed);

	test->case_begin = (GSourceFunc) read_call_meter_start;
	ret = call_meter_case_run(test);

	call_meter_state_finalize(test);
	return ret;
}

int blts_ofono_set_call_meter_data(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_meter_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	/* log_set_level(5); */

	test = call_meter_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");

	test->pin = strdup((data->old_pin) ? (data->old_pin) : "31337");

	test->new_accu_call_meter_max = strdup((data->accu_cm_max) ? (data->accu_cm_max) : "");
	test->new_price_per_unit = strdup((data->ppu) ? (data->ppu) : "");
	test->new_currency = strdup((data->currency) ? (data->currency) : "");

	if( !strlen(test->new_accu_call_meter_max) &&
		!strlen(test->new_price_per_unit) &&
		!strlen(test->new_currency) )
	{
		LOG("At least one Call Meter data parameter (-a, -p, or -c) must be given\n");
		return -1;
	}

	test->user_timeout = data->user_timeout ? data->user_timeout : 5000;
	test->signalcb_CallMeter_PropertyChanged =
		G_CALLBACK(on_call_meter_property_changed);

	test->case_begin = (GSourceFunc) set_call_meter_start;
	ret = call_meter_case_run(test);

	call_meter_state_finalize(test);
	return ret;
}

int blts_ofono_reset_call_meter_data(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_meter_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	/* log_set_level(5); */

	test = call_meter_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->pin = strdup((data->old_pin) ? (data->old_pin) : "31337");
	test->user_timeout = data->user_timeout ? data->user_timeout : 5000;
	test->signalcb_CallMeter_PropertyChanged =
		G_CALLBACK(on_call_meter_property_changed);

	test->case_begin = (GSourceFunc) reset_call_meter_start;
	ret = call_meter_case_run(test);

	call_meter_state_finalize(test);
	return ret;
}

int blts_ofono_test_near_max_warning(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret;
	struct call_meter_case_state *test;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);

 	/* log_set_level(5); */

	test = call_meter_state_init((my_ofono_data *) user_ptr);

	if (!test)
		return -1;

	test->address = strdup((data->remote_address) ? (data->remote_address) : "123456");
	test->pin = strdup((data->old_pin) ? (data->old_pin) : "31337");
	test->user_timeout = data->user_timeout ? data->user_timeout : 5000;
	test->signalcb_CallMeter_PropertyChanged =
		G_CALLBACK(on_call_meter_property_changed);
	test->signalcb_CallMeter_NearMaximumWarning =
		G_CALLBACK(on_call_meter_near_max_warning);

	test->warning_received = FALSE;
	test->case_begin = (GSourceFunc) near_max_warning_start;
	ret = call_meter_case_run(test);

	call_meter_state_finalize(test);
	return ret;
}

//variable data seeds
/* Convert generated parameters to test case format. */
void *call_meter_variant_read_arg_processor(struct boxed_value *args, void *user_ptr)
{
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	timeout = atol(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (!data->timeout)
		data->timeout = timeout;

	return data;
}

void *call_meter_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *acc_max = 0, *ppu = 0, *currency = 0;
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	acc_max = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	ppu = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	currency = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->accu_cm_max)
		free(acc_max);
	else
		data->accu_cm_max = acc_max;

	if (data->ppu)
		free(ppu);
	else
		data->ppu = ppu;

	if (data->currency)
		free(currency);
	else
		data->currency = currency;

	if (!data->timeout)
		data->timeout = timeout;

	return data;
}

void *call_meter_variant_reset_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *remote_addr = 0, *old_pin = 0;
	long timeout = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	remote_addr = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	old_pin = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	timeout = atol(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->remote_address)
		free(remote_addr);
	else
		data->remote_address = remote_addr;

	if (data->old_pin)
		free(old_pin);
	else
		data->old_pin = old_pin;

	if (!data->timeout)
		data->timeout = timeout;

	return data;
}
