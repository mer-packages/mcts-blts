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


int ofono_change_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;

	int i;


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
				return -1;
			}

			//type, old, new
			if(!org_ofono_SimManager_change_pin(proxy, data->pin_type, data->old_pin, data->new_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
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

int ofono_enter_pin(void* user_ptr, int __attribute__((unused))test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;

	int i;

	if(!data->pin_type ||
	   !data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
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
				return -1;
			}

			//type, pin
			if(!org_ofono_SimManager_enter_pin(proxy, data->pin_type, data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
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

int ofono_reset_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;

	int i;
	if(!data->pin_type ||
	   !data->old_pin ||
	   !data->new_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <puk> -n <pin>\n");
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
				return -1;
			}

			// type, puk, pin
			if(!org_ofono_SimManager_reset_pin(proxy, data->pin_type, data->old_pin, data->new_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
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

int ofono_lock_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;
	int i;
	if(!data->pin_type ||
	   !data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
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
				return -1;
			}
			BLTS_DEBUG("Locking PIN \"%s\" with number %s\n", data->pin_type, data->old_pin);
			// type, pin
			if(!org_ofono_SimManager_lock_pin(proxy,  data->pin_type, data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
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

int ofono_unlock_pin(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int retval = 0;
	int i;
	if(!data->pin_type ||
	   !data->old_pin)
	{
		BLTS_DEBUG("usage: -y <pin,pin2,...> -o <pin> \n");
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
				return -1;
			}

			BLTS_DEBUG("UnLocking PIN \"%s\" with number %s\n", data->pin_type, data->old_pin);
			// type, pin
			if(!org_ofono_SimManager_unlock_pin(proxy, data->pin_type, data->old_pin, &error))
			{
				display_dbus_glib_error(error);
				g_error_free (error);
				retval = -1;
			}
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
