#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <blts_log.h>

#include "ofono-util.h"
#include "ofono-general.h"

#include "sim-cases.h"
#include "ofono-modem-bindings.h"


#include "signal-marshal.h"


struct sim_manager_state
{
	my_ofono_data* data;
	DBusGProxy* proxy;

	GCallback signalcb_SimManager_PropertyChanged;

	gint result;
};


static void
on_sim_manager_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key,
    GValue* value, gpointer user_data);
static gboolean sim_manager_timeout(gpointer user_data);


GHashTable* get_sim_properties(DBusGProxy *proxy)
{
	GError *error = NULL;
	GHashTable* list = NULL;

	if(!proxy)
		return NULL;

	if(!org_ofono_SimManager_get_properties(proxy, &list, &error))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		return NULL;
	}
	return list;
}


int ofono_change_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int i, retval = 0;

	if(!data->pin_type ||
	   !data->old_pin ||
	   !data->new_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <old pin> -n <new pin>\n");
		return -1;
	}


	if(!my_ofono_get_modem(data))
	{
		for(i=0; i<data->number_modems; i++)
		{
			proxy = dbus_g_proxy_new_for_name (data->connection,
												OFONO_BUS,
												data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				retval = -1;
			}

			//type, old, new
			if(!org_ofono_SimManager_change_pin(proxy, data->pin_type, data->old_pin, data->new_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}

			g_object_unref(proxy);
			proxy = NULL;
		}

	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		retval = -1;
	}

	return retval;
}

void delete_chars(char *src, char c, int len)
{
    char *dst;
    int i;
    if ( c == 0 )
        return;
    if ( len <= 0 )
        len = strlen(src);
    dst = src;
    for ( i = 0; i < len && *src != 0; i++, src++ )
    {
        if ( *src != c )
            *dst++ = *src;
    }
    *dst = 0;
     return;
}

int ofono_enter_pin(void* user_ptr, int __attribute__((unused))test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	GHashTable* props = NULL;
	int i;
	GValue *value = NULL;

	struct sim_manager_state state;
	state.data = data;
	state.proxy = NULL;

	if(!state.data->pin_type ||
	   !state.data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
		return -1;
	}

	if(!my_ofono_get_modem(state.data))
	{
		for(i=0; i<state.data->number_modems; i++)
		{
			state.proxy = dbus_g_proxy_new_for_name (state.data->connection,
												OFONO_BUS,
												state.data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!state.proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				return -1;
			}

			// check if some PIN is requested
			props = get_sim_properties(state.proxy);
			value = g_hash_table_lookup(props, "PinRequired");
			char *str = g_strdup_value_contents(value);
			g_hash_table_destroy(props);
			delete_chars(str, '\"', 0);

			if(strcmp(state.data->pin_type, str))
			{
				BLTS_DEBUG("SIM card is not requesting current given PIN number, case failed\n");
				BLTS_DEBUG("Set SIM card to state where it requests PIN\n");
				BLTS_DEBUG("Case is expecting value '%s'\n", str);
				g_object_unref(state.proxy);
				state.proxy = NULL;
				state.result = -1;
				continue;
			}


			state.signalcb_SimManager_PropertyChanged = G_CALLBACK(
			    on_sim_manager_property_changed);
			dbus_g_proxy_add_signal(state.proxy, "PropertyChanged", G_TYPE_STRING,
			    G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state, 0);


			//type, pin
			if(!org_ofono_SimManager_enter_pin(state.proxy, state.data->pin_type, state.data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				state.result = -1;
			}
			guint source_id = g_timeout_add(20000, (GSourceFunc) sim_manager_timeout,
			    &state);
			g_main_loop_run(state.data->mainloop);
			g_source_remove(source_id);

			dbus_g_proxy_disconnect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state);

			g_object_unref(state.proxy);
			state.proxy = NULL;
		}
	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		state.result = -1;
	}

	return state.result;
}

int ofono_reset_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	GHashTable* props = NULL;
	GValue *value = NULL;
	int i;

	struct sim_manager_state state;
	state.data = data;
	state.proxy = NULL;
	state.result = 0;

	if(!state.data->pin_type ||
	   !state.data->old_pin ||
	   !state.data->new_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <puk> -n <pin>\n");
		return -1;
	}


	if(!my_ofono_get_modem(state.data))
	{
		for(i=0; i<state.data->number_modems; i++)
		{
			state.proxy = dbus_g_proxy_new_for_name (state.data->connection,
												OFONO_BUS,
												state.data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!state.proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				return -1;
			}
			// check if some PIN is requested
			props = get_sim_properties(state.proxy);
			value = g_hash_table_lookup(props, "PinRequired");
			char *str = g_strdup_value_contents(value);
			g_hash_table_destroy(props);
			delete_chars(str, '\"', 0);

			if(strcmp("puk", str) || strcmp("puk2", str))
			{
				BLTS_DEBUG("SIM card is not requesting reset at current moment case failed\n");
				BLTS_DEBUG("Set SIM card to state where it requests PUK\n");
				BLTS_DEBUG("Case is expecting value '%s'\n", str);
				g_object_unref(state.proxy);
				state.proxy = NULL;
				state.result = -1;
				continue;
			}

			state.signalcb_SimManager_PropertyChanged = G_CALLBACK(
			    on_sim_manager_property_changed);
			dbus_g_proxy_add_signal(state.proxy, "PropertyChanged", G_TYPE_STRING,
			    G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state, 0);

			BLTS_DEBUG ("Enterin PUK code %s to \"%s\", new pin will be %s\n", state.data->old_pin, state.data->pin_type, state.data->new_pin);

			// type = type, old_pin = puk , new_pin = original pin
			if(!org_ofono_SimManager_reset_pin(state.proxy, state.data->pin_type, state.data->old_pin, state.data->new_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				state.result = -1;
			}
			guint source_id = g_timeout_add(20000, (GSourceFunc) sim_manager_timeout,
			    &state);
			g_main_loop_run(state.data->mainloop);
			g_source_remove(source_id);

			dbus_g_proxy_disconnect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state);

			g_object_unref(state.proxy);
			state.proxy = NULL;

		}
	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		state.result = -1;
	}

	return state.result;
}

int ofono_lock_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	int i;

	struct sim_manager_state state;
	state.data = data;
	state.proxy = NULL;
	state.result = 0;
	if(!state.data->pin_type ||
	   !state.data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
		return -1;
	}

	if(!my_ofono_get_modem(state.data))
	{
		for(i=0; i<state.data->number_modems; i++)
		{
			state.proxy = dbus_g_proxy_new_for_name (state.data->connection,
												OFONO_BUS,
												state.data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!state.proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				return -1;
			}


			state.signalcb_SimManager_PropertyChanged = G_CALLBACK(
			    on_sim_manager_property_changed);
			dbus_g_proxy_add_signal(state.proxy, "PropertyChanged", G_TYPE_STRING,
			    G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state, 0);

			BLTS_DEBUG("Locking PIN \"%s\" with number %s\n", state.data->pin_type, state.data->old_pin);
			// type, pin
			if(!org_ofono_SimManager_lock_pin(state.proxy,  state.data->pin_type, state.data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				state.result = -1;
			}
			guint source_id = g_timeout_add(20000, (GSourceFunc) sim_manager_timeout,
			    &state);
			g_main_loop_run(state.data->mainloop);
			g_source_remove(source_id);

			dbus_g_proxy_disconnect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state);

			g_object_unref(state.proxy);
			state.proxy = NULL;
		}
	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		state.result = -1;
	}

	return state.result;
}

int ofono_unlock_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	int i;

	struct sim_manager_state state;
	state.data = data;
	state.proxy = NULL;

	if(!state.data->pin_type ||
	   !state.data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
		return -1;
	}

	if(!my_ofono_get_modem(state.data))
	{
		for(i=0; i<state.data->number_modems; i++)
		{
			state.proxy = dbus_g_proxy_new_for_name (state.data->connection,
												OFONO_BUS,
												state.data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!state.proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				return -1;
			}

			state.signalcb_SimManager_PropertyChanged = G_CALLBACK(
			    on_sim_manager_property_changed);
			dbus_g_proxy_add_signal(state.proxy, "PropertyChanged", G_TYPE_STRING,
			    G_TYPE_VALUE, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state, 0);

			BLTS_DEBUG("UnLocking PIN \"%s\" with number %s\n", state.data->pin_type, state.data->old_pin);

			// type, pin
			if(!org_ofono_SimManager_unlock_pin(state.proxy, state.data->pin_type, state.data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				state.result = -1;
			}
			guint source_id = g_timeout_add(20000, (GSourceFunc) sim_manager_timeout,
			    &state);
			g_main_loop_run(state.data->mainloop);
			g_source_remove(source_id);

			dbus_g_proxy_disconnect_signal(state.proxy, "PropertyChanged",
			    state.signalcb_SimManager_PropertyChanged, &state);

			g_object_unref(state.proxy);
			state.proxy = NULL;
		}
	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		state.result = -1;
	}

	return state.result;
}

int ofono_sim_properties(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	GHashTable* properties=NULL;
	int retval = 0;
	int i;

	if(!my_ofono_get_modem(data))
	{
		for(i=0; i<data->number_modems; i++)
		{
			proxy = dbus_g_proxy_new_for_name (data->connection,
												OFONO_BUS,
												data->modem[i],
												OFONO_SIM_INTERFACE);
			if(!proxy)
			{
				BLTS_DEBUG ("Failed to open proxy for " OFONO_SIM_INTERFACE "\n");
				return -1;
			}

			if(!org_ofono_SimManager_get_properties(proxy, &properties, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
			g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
			g_hash_table_destroy(properties);
			g_object_unref (proxy);
			proxy = NULL;
			properties = NULL;
		}
	}
	else
	{
		BLTS_DEBUG("No modems found!\n");
		retval = -1;
	}
	return retval;
}


//variable data seeds
/* Convert generated parameters to test case format. */
void *sim_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *old_pin = 0, *new_pin = 0, *pin_type = 0;
	my_ofono_data *data = ((my_ofono_data *) user_ptr);
	if (!data)
		return 0;

	old_pin = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	new_pin = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	pin_type = strdup(blts_config_boxed_value_get_string(args));	

	/* These are already non-zero, if set on command line */
	
	if (data->old_pin)
		free(old_pin);
	else
		data->old_pin = old_pin;
	
	if (data->new_pin)
		free(new_pin);
	else
		data->new_pin = new_pin;
	
	if (data->pin_type)
		free(pin_type);
	else
		data->pin_type = pin_type;

	return data;
}

/** Signal callback */
static void
on_sim_manager_property_changed(__attribute__((unused))DBusGProxy *proxy, char *key,
    GValue* value, gpointer user_data)
{
	struct sim_manager_state* state = (struct sim_manager_state*) user_data;
	const char* value_str = g_value_get_string(value);

	BLTS_DEBUG("SIM manager property '%s' changed to '%s'\n", key, value_str);

	g_main_loop_quit(state->data->mainloop);
}


static gboolean
sim_manager_timeout(gpointer user_data)
{
	struct sim_manager_state* state = (struct sim_manager_state*) user_data;
	state->result = -1;
	BLTS_DEBUG("Timeout while waiting for signal\n");

	g_main_loop_quit(state->data->mainloop);

	return FALSE;
}
