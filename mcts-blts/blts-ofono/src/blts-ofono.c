/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* blts-ofono.c -- ofono using common CLI functions

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
#include <getopt.h>
#include <blts_cli_frontend.h>
#include <blts_params.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ofono-modem-bindings.h"
#include "ofono-call-bindings.h"
#include "org-ofono-Modem.h"

#include "ofono-general.h"
#include "ofono-util.h"

#include "modem-cases.h"
#include "sms-cases.h"
#include "call-cases.h"
#include "dual-call-cases.h"
#include "multiparty-call-cases.h"
#include "call-forwarding.h"
#include "call-barring.h"
#include "sim-cases.h"
#include "call-volume-cases.h"
#include "call-meter-cases.h"

static void my_ofono_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		/* What is displayed on the first 'USAGE' line */
		"[-r <number>] [-m <number>] [-f <number>] [-h hangup]\n"
		"[-n new_pin] [-o old_pin] [-y pin_type]\n"
		"[-V volume] [-a accu_cm_max] [-p ppu] [-c currency] [-t timeout]",
		/* Description of the arguments */
		"  -r: Recipient address/phone number (for voice call/SMS)\n"
		"  -m: SMS Center address (for SMS send)\n"
		"  -f: Call Forwarding address\n"
		"  -h: Hangup timeout\n"
		"  -n: New PIN code\n"
		"  -o: Old (current) PIN code\n"
		"  -y: PIN code type (puk, ..)\n"
		"  -V: Call volume for Microphone/Speaker (0-100%)\n"
		"  -a: Accumulated Call Meter maximum value\n"
		"  -p: Price Per Unit conversion value\n"
		"  -c: Three character currency code\n"
		"  -t: Set timeout for test case execution\n"
		"  --dontcleanup: Don't clean up call state before/after tests\n"
		);
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "r:m:f:h:n:o:y:V:a:p:c:t:";
static const struct option long_opts[] =
{
	{"dontcleanup", 0, NULL, 1},
	{0,0,0,0}
};

/* Return NULL in case of an error */
static void* my_ofono_argument_processor(int argc, char **argv)
{
	int c, ret;
	my_ofono_data* my_data = malloc(sizeof(my_ofono_data));
	memset(my_data, 0, sizeof(my_ofono_data));

	while((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch(c)
		{
		case 'a':
			if (my_data->accu_cm_max)
				free(my_data->accu_cm_max);
			my_data->accu_cm_max = strdup(optarg);
			break;
		case 'p':
			if (my_data->ppu)
				free(my_data->ppu);
			my_data->ppu = strdup(optarg);
			break;
		case 'c':
			if (my_data->currency)
				free(my_data->currency);
			my_data->currency = strdup(optarg);
			break;
		case 'V':
			if (my_data->volume)
				free(my_data->volume);
			my_data->volume = strdup(optarg);
			break;
		case 'n':
			if (my_data->new_pin)
				free(my_data->new_pin);
			my_data->new_pin = strdup(optarg);
			break;
		case 'o':
			if (my_data->old_pin)
				free(my_data->old_pin);
			my_data->old_pin = strdup(optarg);
			break;
		case 'y':
			if (my_data->pin_type)
				free(my_data->pin_type);
			my_data->pin_type = strdup(optarg);
			break;
		case 'r':
			if (my_data->remote_address)
				free(my_data->remote_address);
			my_data->remote_address = strdup(optarg);
			break;
		case 'f':
			if (my_data->forward_address)
				free(my_data->forward_address);
			my_data->forward_address = strdup(optarg);
			break;
		case 'm':
			if (my_data->smsc_address)
				free(my_data->smsc_address);
			my_data->smsc_address = strdup(optarg);
			break;
		case 'h':
			my_data->user_timeout = atoi(optarg);
			break;
		case 't':
			my_data->timeout = atol(optarg);
			break;
		case 0:
			/* getopt_long() flag */
			break;
		case 1:
			my_data->fl_dontcleanup = 1;
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	ret = blts_config_declare_variable_test("oFono - Set microphone volume",
		volume_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "volume", "null",
		CONFIG_PARAM_STRING, "call_volume_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Set speaker volume",
		volume_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "volume", "null",
		CONFIG_PARAM_STRING, "call_volume_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Set muted",
		volume_muted_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "call_volume_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Call and send DTMF",
		dtmf_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "dtmf_tones", "1",
		CONFIG_PARAM_STRING, "dtmf_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_register_generating_func("dtmf_generator",
			dtmf_variant_tones_generator);

	ret = blts_config_declare_variable_test("oFono - Change password for barrings",
		barring_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Disable barrings",
		barring_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Disable incoming barrings",
		barring_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Disable outgoing barrings",
		barring_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;


	ret = blts_config_declare_variable_test("oFono - Call barrings test",
		barring_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Check barring properties",
			barring_variant_set_arg_processor,
			CONFIG_PARAM_STRING, "old_pin", "1234",
			CONFIG_PARAM_STRING, "new_pin", "1234",
			CONFIG_PARAM_STRING, "call_barring_timeout", "60000",
			CONFIG_PARAM_NONE);
		if (ret)
			return NULL;

	ret = blts_config_declare_variable_test("oFono - Change PIN",
		sim_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "pin_type", "pin",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Enter PIN",
		sim_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "pin_type", "pin",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Reset PIN",
		sim_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "13243546",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "pin_type", "pin",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Lock PIN",
		sim_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "pin_type", "pin",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Unlock PIN",
		sim_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "new_pin", "1234",
		CONFIG_PARAM_STRING, "pin_type", "pin",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Call meters read",
		call_meter_variant_read_arg_processor,
		CONFIG_PARAM_STRING, "call_meter_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Call meters set",
		call_meter_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "call_meter_max", "20",
		CONFIG_PARAM_STRING, "price_per_unit", "1.0",
		CONFIG_PARAM_STRING, "currency", "FIM",
		CONFIG_PARAM_STRING, "call_meter_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Call meters reset",
		call_meter_variant_reset_arg_processor,
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "call_meter_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Call meters near max warning",
		call_meter_variant_reset_arg_processor,
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "old_pin", "1234",
		CONFIG_PARAM_STRING, "call_meter_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Create voicecall",
		voicecall_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "country_code", "+000",
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Create voicecall with hidden caller ID",
		voicecall_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "country_code", "+000",
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Answer to voicecall and deflect",
		voicecall_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "country_code", "+000",
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Answer to voicecall and hangup",
		voicecall_answer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Answer to voicecall and hangup all",
		voicecall_answer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Answer to voicecall and wait another call",
		voicecall_answer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Answer to voicecall and remote hangup",
		voicecall_answer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Cancel voicecall",
		voicecall_answer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Transfer",
		dual_call_case_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Swap",
		dual_call_case_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Release and answer",
		dual_call_case_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Hold and answer",
		dual_call_case_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Multiparty call test",
		multiparty_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Private call test",
		multiparty_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "voicecall_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Send SMS",
		sms_send_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "sms_message", "Test",
		CONFIG_PARAM_STRING, "country_code", "+000",
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "SMSC", "+000123456789",
		CONFIG_PARAM_STRING, "bearer", "cs-preferred",
		CONFIG_PARAM_STRING, "sms_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - SMSC number test",
		sms_smsc_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "valid_tst_smsc", "12345678901234567890",
		CONFIG_PARAM_STRING, "sms_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Receive SMS",
		sms_recv_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "SMSC", "+000123456789",
		CONFIG_PARAM_STRING, "bearer", "cs-preferred",
		CONFIG_PARAM_STRING, "sms_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_register_generating_func("message_generator",
		sms_variant_message_generator);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Disable forwardings",
		forwarding_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "disable_forwarding", "",
		CONFIG_PARAM_STRING, "call_forwarding_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Unconditional forwarding",
		forwarding_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "forward_address", "0987654321",
		CONFIG_PARAM_STRING, "call_forwarding_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Forward if busy",
		forwarding_if_busy_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "remote_address", "1234567890",
		CONFIG_PARAM_STRING, "forward_address", "0987654321",
		CONFIG_PARAM_STRING, "call_forwarding_keepalive", "15",
		CONFIG_PARAM_STRING, "call_forwarding_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Forward if no reply",
		forwarding_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "forward_address", "0987654321",
		CONFIG_PARAM_STRING, "call_forwarding_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("oFono - Forward if not reachable",
		forwarding_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "forward_address", "0987654321",
		CONFIG_PARAM_STRING, "call_forwarding_timeout", "60000",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	if(!my_data->barrings_pin)
	{
		ret = blts_config_get_value_string("default_barrings_pin", &my_data->barrings_pin);
		if(ret)
		{
			BLTS_WARNING("Can't read original pin value from config file\n");
			BLTS_WARNING("Defaulting to 3579\n");
			my_data->barrings_pin=strdup("3579");
		}
	}

	return my_data;
}

/* user_ptr is what you returned from my_ofono_argument_processor */
static void my_ofono_teardown(void *user_ptr)
{
	int i;
	my_ofono_data* data = (my_ofono_data*)user_ptr;

	if(user_ptr)
	{
		ensure_calls_cleared(data);
		reset_supplementary_services(data);

		if (data->accu_cm_max)
			free(data->accu_cm_max);

		if (data->ppu)
			free(data->ppu);

		if (data->currency)
			free(data->currency);

		if (data->volume)
			free(data->volume);

		if (data->remote_address)
			free(data->remote_address);

		if (data->smsc_address)
			free(data->smsc_address);

		if (data->bearer)
			free(data->bearer);

		if (data->forward_address)
			free(data->forward_address);

		if (data->old_pin)
			free(data->old_pin);

		if (data->new_pin)
			free(data->new_pin);

		if (data->pin_type)
			free(data->pin_type);

		if (data->barrings_pin)
			free(data->barrings_pin);

		for(i=0; i<MAX_MODEMS; i++) {
			if (data->modem[i])
				free(data->modem[i]);
		}

		if (data->connection)
			dbus_g_connection_unref(data->connection);
		if(data->mainloop)
		{
			g_main_loop_quit(data->mainloop);
			g_main_loop_unref(data->mainloop);
			data->mainloop = NULL;
		}
		free(user_ptr);
	}
}


/* Add required application init here; run only once. */
static void app_init_once()
{
	static int init_done = 0;

	if (init_done)
		return;

	init_done = 1;

	if (!g_thread_supported())
		g_thread_init(0);

	g_type_init();
	dbus_g_thread_init();

}

/* user_ptr is what you returned from my_ofono_argument_processor
 * test_num Test case being run from my_ofono_cases, starts from 1
 * return non-zero in case of an error */
static int my_ofono_case_query(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	GHashTable* properties=NULL;
	char *contacts = NULL;
	int i;
	int retval = 0;

	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		// testing modem
		BLTS_DEBUG("{Testing modem %s}\n", data->modem[i])
		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_MODEM_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_MODEM_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_Modem_get_properties(proxy, &properties, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			error = NULL;
			retval = -1;
		}
		if (properties) {
			g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
			g_hash_table_destroy(properties);
		}
		g_object_unref (proxy);
		proxy = NULL;
		properties = NULL;

		// testing network interface
		BLTS_DEBUG("Testing network...\n");
		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_NW_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_NW_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_NetworkRegistration_get_properties(proxy, &properties, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			error = NULL;
			retval = -1;
		}
		if (properties) {
			g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
			g_hash_table_destroy(properties);
		}
		g_object_unref (proxy);
		proxy = NULL;
		// testing phonebook interface
		BLTS_DEBUG("Testing phonebook\n");
		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_PB_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_PB_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_Phonebook_import(proxy, &contacts, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			error = NULL;
			retval = -1;
		}

		BLTS_DEBUG("Answer from PB: %s\n", contacts);
		free(contacts);
		g_object_unref (proxy);
		proxy=NULL;

	}

	if(retval == -1)
		BLTS_DEBUG("One or more modems failed test\n");

	return retval;
}


void prop_hash_table_foreach(gpointer key, GValue* val, __attribute__((unused)) gpointer user_data)
{
	BLTS_DEBUG("\t\t%s = ", key);
	BLTS_DEBUG("%s\n", g_strdup_value_contents (val));
}

static void interface_properties(char *interface, gpointer user_data)
{
	my_ofono_data* data = (my_ofono_data*)user_data;
	DBusGProxy *proxy=NULL;
	GError *error = NULL;
	GHashTable* properties=NULL;
	//my_ofono_data* data = (my_ofono_data*)user_data;
	BLTS_DEBUG("\t[%s]\n", (char*)interface);
	proxy = dbus_g_proxy_new_for_name (data->connection,
										OFONO_BUS,
										data->current_modem,
										interface);
	if(!proxy)
	{
		BLTS_DEBUG ("Failed to open proxy for %s\n", interface);
		return;
	}
	if(!dbus_g_proxy_call (proxy, "GetProperties", &error,
					G_TYPE_INVALID,
					dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &properties, G_TYPE_INVALID))
	{
		display_dbus_glib_error(error);
		g_error_free (error);
		error = NULL;
		return;
	}
	g_hash_table_foreach(properties, (GHFunc)prop_hash_table_foreach, data);
	g_hash_table_destroy(properties);
	g_object_unref (proxy);
	proxy = NULL;
	properties = NULL;

}

void modem_properties_foreach(gpointer key, GValue* val, gpointer user_data)
{
	my_ofono_data* data = (my_ofono_data*)user_data;
	BLTS_DEBUG("[ %s ]\n", key);
	BLTS_DEBUG("\t%s\n", g_strdup_value_contents (val));

	if(!strcmp(key, "Interfaces"))
	{
		int i=0;
		gchar **interf= g_value_peek_pointer(val);

		while(interf[i]!=NULL)
		{
			interface_properties(interf[i], data);
			i++;
		}
	}
}

static int ofono_list_modems(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	GHashTable* properties=NULL;
	int i;
	int retval = 0;

	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		// testing modem
		data->current_modem=data->modem[i];
		BLTS_DEBUG("{Testing modem %s}\n", data->modem[i])
		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_MODEM_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_MODEM_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_Modem_get_properties(proxy, &properties, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			retval = -1;
		}

		g_hash_table_foreach(properties, (GHFunc)modem_properties_foreach, data);
		g_hash_table_destroy(properties);
		g_object_unref (proxy);
		proxy = NULL;
		properties = NULL;

	}

	if(retval == -1)
		BLTS_DEBUG("One or more modems failed test\n");

	return retval;
}

/** Register DUT to network test case*/
static int my_ofono_case_regnetwork(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int i;
	int retval = 0;
	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		// testing network interface

		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_NW_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_NW_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_NetworkRegistration_register(proxy, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			retval = -1;
		}
		g_object_unref (proxy);
		proxy = NULL;
	}

	return retval;
}


/** De-register DUT from network test case */
static int my_ofono_case_deregnetwork(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int i;
	int retval = 0;
	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		// testing network interface

		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_NW_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_NW_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_NetworkRegistration_deregister(proxy, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			retval = -1;
		}
		g_object_unref (proxy);
		proxy = NULL;
	}


	return retval;
}

/** Enable all modems (Power ON) test case*/
static int my_ofono_case_enable_modems(void* user_ptr, __attribute__((unused))int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;


	int i;
	int retval = 0;

	GValue val = G_VALUE_INIT;
	g_value_init (&val, G_TYPE_BOOLEAN);
	g_value_set_boolean (&val, TRUE);


	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		// testing modem
		BLTS_DEBUG("{Powering up modem %s}\n", data->modem[i]);
		proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_MODEM_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_MODEM_INTERFACE "\n");
			return -1;
		}

		if(!org_ofono_Modem_set_property(proxy, "Powered", &val,  &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			retval = -1;
		}
	}
	g_object_unref(proxy);
	return retval;
}


static int my_ofono_case_forwardings(void* user_ptr, int test_num)
{
	FUNC_ENTER()
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	int retval = -1;
	int i;

	if(!data->forward_address && test_num != BLTS_OFONO_DISABLE_FORWARDINGS)
	{
		BLTS_DEBUG("forward adress: %s\n", data->forward_address);
		BLTS_DEBUG("-f switch missing or forward number missing!\n");
		FUNC_LEAVE()
		return -1;
	}

	if(my_ofono_get_modem(data))
	{
		BLTS_DEBUG("No modems, found. Cannot proceed testing.\n");
		FUNC_LEAVE()
		return -1;
	}

	for(i=0; i<data->number_modems; i++)
	{
		switch(test_num)
		{

		case BLTS_OFONO_FORWARD_IF_BUSY:
			if(data->forward_address)
				retval = ofono_call_forwarding_set_busy((void *)data, data->modem[0], data->forward_address);
			else
				BLTS_DEBUG("-f switch missing or forward number missing!\n");
			if(!retval)
			{
				BLTS_DEBUG("Calls are forwarded to '%s'\n", data->forward_address);
			}
			retval = ofono_call_forwarding_check_settings("VoiceBusy", data->forward_address, (void *)data, data->modem[i]);
			if(retval)
			{
				BLTS_DEBUG("Call forwarding state could not be confirmed with oFono\n");
			}
			break;
		case BLTS_OFONO_DISABLE_FORWARDINGS:
			retval = ofono_call_forwarding_disable_all((void *)data, data->modem[i], NULL);
			if(retval)
			{
				BLTS_DEBUG("Call forwarding state could not be confirmed with oFono\n");
			}
			break;
		case BLTS_OFONO_UNCONDITIONAL_FORWARDING:
			if(data->forward_address)
				retval = ofono_call_forwarding_set_unconditional((void *)data, data->modem[i], data->forward_address);
			else
				BLTS_DEBUG("No remote address given, no forwarding made\n");
			if(!retval)
			{
				BLTS_DEBUG("Calls are forwarded to '%s'\n", data->forward_address);
			}
			retval = ofono_call_forwarding_check_settings("VoiceUnconditional", data->forward_address, (void *)data, data->modem[i]);
			if(retval)
			{
				BLTS_DEBUG("Call forwarding could not be confirmed with oFono\n");
			}

			break;
		case BLTS_OFONO_FORWARD_IF_NO_REPLY: // no reply
			if(data->forward_address)
				retval = ofono_call_forwarding_set_no_reply((void *)data, data->modem[i], data->forward_address);
			else
				BLTS_DEBUG("No remote address given, no forwarding made\n");
			if(!retval)
			{
				BLTS_DEBUG("Calls are forwarded to '%s'\n", data->forward_address);
			}
			retval = ofono_call_forwarding_check_settings("VoiceNoReply", data->forward_address, (void *)data, data->modem[i]);
			if(retval)
			{
				BLTS_DEBUG("Call forwarding could not be confirmed with oFono\n");
			}

			break;
		case BLTS_OFONO_FORWARD_IF_NOT_REACHABLE: // if not reachable
			if(data->forward_address)
				retval = ofono_call_forwarding_set_not_reachable((void *)data, data->modem[i], data->forward_address);
			else
				BLTS_DEBUG("No remote address given, no forwarding made\n");
			if(!retval)
			{
				BLTS_DEBUG("Calls are forwarded to '%s'\n", data->forward_address);
			}
			retval = ofono_call_forwarding_check_settings("VoiceNotReachable", data->forward_address, (void *)data, data->modem[i]);
			if(retval)
			{
				BLTS_DEBUG("Call forwarding could not be confirmed with oFono\n");
			}

			break;
		}
		ofono_call_forwarding_properties((void *)data, data->modem[i]);
	}
	FUNC_LEAVE()
	return retval;
}

static int ofono_barring_properties(void* user_ptr, int test_num)
{
	my_ofono_data* data = (my_ofono_data*)user_ptr;
	int retval = 0;
	int i;

	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		switch(test_num)
		{
		case BLTS_OFONO_DISABLE_BARRINGS:
			retval = ofono_call_barring_disable_all((void *)data, data->modem[i]);
			break;
		case BLTS_OFONO_DISABLE_OUTGOING_BARRING:
			retval = ofono_call_barring_disable_all_outgoing((void *)data, data->modem[i]);
			break;
		case BLTS_OFONO_DISABLE_INCOMING_BARRING:
			retval = ofono_call_barring_disable_all_incoming((void *)data, data->modem[i]);
			break;
		case BLTS_OFONO_CHECK_BARRING_PROPERTIES:
			retval = ofono_call_barring_properties((void *)data, data->modem[i]);
			break;
		case BLTS_OFONO_CHANGE_PASSWORD_FOR_BARRINGS:
			retval = ofono_call_barring_change_password((void *)data, data->modem[i]);
			break;
		default:
			BLTS_DEBUG("Test case set internal error\n");
			retval = -1;
			break;
		}

	}
	return retval;
}


/**
 * Runs a network operator scan to discover the currently available operators
 * and retrieves also the current cached operator list after scan.
 * The operator scan can interfere with any active
 * GPRS contexts.  Expect the context to be unavailable
 * for the duration of the operator scan.
 */
static int ofono_propose_scan(void *user_ptr, __attribute__((unused))int test_num)
{

	my_ofono_data* data = (my_ofono_data*)user_ptr;
	GError *error = NULL;
	DBusGProxy *proxy=NULL;
	int i;
	GPtrArray *networks = NULL;
	GPtrArray *cached_networks = NULL;

	if(my_ofono_get_modem(data))
		return -1;

	for(i=0; i<data->number_modems; i++)
	{
		int j;		
		// testing network interface
			proxy = dbus_g_proxy_new_for_name (data->connection,
											OFONO_BUS,
											data->modem[i],
											OFONO_NW_INTERFACE);
		if(!proxy)
		{
			BLTS_DEBUG ("Failed to open proxy for " OFONO_NW_INTERFACE "\n");
			return -1;
		}
		/* Run network operator scan to discover available operators */
		if(!org_ofono_NetworkRegistration_scan(proxy, &networks, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			goto error;
		}
		else
		{
			if(!networks)
			{
				BLTS_DEBUG("No networks found on scan\n");
				goto error;
			}
		}		
		BLTS_DEBUG("\n--Networks found on scan--\n");
		for (j = 0; j < (int) networks->len; j++)
		{
			GValueArray *operator = g_ptr_array_index (networks, j);
			char *operator_name = g_value_get_boxed (g_value_array_get_nth (operator, 0));
			GHashTable *properties = g_value_get_boxed (g_value_array_get_nth (operator, 1));			
			BLTS_DEBUG("\n%s\n", operator_name);
			g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		}

		/* Retrieve the current cached operator list */
		if(!org_ofono_NetworkRegistration_get_operators(proxy, &cached_networks, &error))
		{
			display_dbus_glib_error(error);
			g_error_free (error);
			goto error;			
		}
		else
		{
			if(!cached_networks)
			{
				BLTS_DEBUG("No networks found from cache\n");
				goto error;
			}
		}
		
		BLTS_DEBUG("\n--Networks found from cache--\n");
		for (j = 0; j < (int) cached_networks->len; j++)
		{
			GValueArray *operator = g_ptr_array_index (cached_networks, j);
			char *operator_name = g_value_get_boxed (g_value_array_get_nth (operator, 0));
			GHashTable *properties = g_value_get_boxed (g_value_array_get_nth (operator, 1));			
			BLTS_DEBUG("\n%s\n", operator_name);
			g_hash_table_foreach(properties, (GHFunc)hash_entry_gvalue_print, NULL);
		}
		
		g_ptr_array_free(networks, TRUE); networks = NULL;
		g_ptr_array_free(cached_networks, TRUE); cached_networks = NULL;

		g_object_unref (proxy);
		proxy = NULL;
	}
	return 0;
	
error:
	if(networks)
		g_ptr_array_free(networks, TRUE);
	if(cached_networks)	
		g_ptr_array_free(cached_networks, TRUE);
	if(proxy)
		g_object_unref (proxy);
	return -1;
}

/* Your test definitions */

static blts_cli_testcase my_ofono_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity
	 *
	 * Test case timeouts set to 0 are handled via configuration file! */
	{ "oFono - Information Query", my_ofono_case_query, 60000 },
	{ "oFono - Register to network", my_ofono_case_regnetwork, 60000 },
	{ "oFono - De-register from network", my_ofono_case_deregnetwork, 60000 },
	{ "oFono - Enable modems", my_ofono_case_enable_modems, 60000 },
	{ "oFono - Set modems online", blts_ofono_case_modems_online, 60000 },
	{ "oFono - Set modems offline", blts_ofono_case_modems_offline, 60000 },
	{ "oFono - Create voicecall", my_ofono_case_voicecall_to, 0 },
	{ "oFono - Create voicecall with hidden caller ID", my_ofono_case_voicecall_to, 0 },
	{ "oFono - Answer to voicecall and hangup", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Answer to voicecall and hangup all", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Answer to voicecall and wait another call", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Answer to voicecall and deflect", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Answer to voicecall and remote hangup", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Cancel voicecall", my_ofono_case_voicecall_answer, 0 },
	{ "oFono - Transfer", blts_ofono_case_dual_call_transfer, 0 },
	{ "oFono - Swap", blts_ofono_case_dual_call_swap, 0 },
	{ "oFono - Release and answer", blts_ofono_case_dual_call_release_and_answer, 0 },
	{ "oFono - Hold and answer", blts_ofono_case_dual_call_hold_and_answer, 0 },
	{ "oFono - Call and send DTMF", my_ofono_case_voicecall_to_DTMF, 0 },
	{ "oFono - Disable forwardings", my_ofono_case_forwardings, 0 },
	{ "oFono - Unconditional forwarding", my_ofono_case_forwardings, 0 },
	{ "oFono - Forward if busy", my_ofono_case_forwardings, 0 },
	{ "oFono - Forward if no reply", my_ofono_case_forwardings, 0 },
	{ "oFono - Forward if not reachable", my_ofono_case_forwardings, 0 },
	{ "oFono - Send SMS", blts_ofono_send_sms_default, 0 },
	{ "oFono - Receive SMS", blts_ofono_receive_sms_default, 0 },
	{ "oFono - Change PIN", ofono_change_pin, 60000 },
	{ "oFono - Enter PIN", ofono_enter_pin, 60000 },
	{ "oFono - Reset PIN", ofono_reset_pin, 60000 },
	{ "oFono - Lock PIN", ofono_lock_pin, 60000 },
	{ "oFono - Unlock PIN", ofono_unlock_pin, 60000 },
	{ "oFono - Set microphone volume", blts_ofono_set_microphone_volume, 0 },
	{ "oFono - Set speaker volume", blts_ofono_set_speaker_volume, 0 },
	{ "oFono - Set muted", blts_ofono_set_muted, 0 },
	{ "oFono - Call meters read", blts_ofono_read_call_meter_data, 0 },
	{ "oFono - Call meters set", blts_ofono_set_call_meter_data, 0 },
	{ "oFono - Call meters reset", blts_ofono_reset_call_meter_data, 0 },
	{ "oFono - Call meters near max warning", blts_ofono_test_near_max_warning, 0 },
	{ "oFono - Check barring properties", ofono_barring_properties, 0 },
	{ "oFono - Disable barrings", ofono_barring_properties, 0 },
	{ "oFono - Disable incoming barrings", ofono_barring_properties, 0 },
	{ "oFono - Disable outgoing barrings", ofono_barring_properties, 0 },
	{ "oFono - Change password for barrings", ofono_barring_properties, 0 },
	{ "oFono - Call barrings test", ofono_call_barring_test, 0 },
	{ "oFono - List all properties", ofono_list_modems, 60000 },
	// This operation can take several...
	{ "oFono - Propose scan", ofono_propose_scan, 120000 },
	//... seconds, and up to several minutes on some modems
	{ "oFono - SMSC number test", ofono_sms_center_number, 0 },
	{ "oFono - Multiparty call test", blts_ofono_case_multiparty, 0 },
	{ "oFono - Private call test", blts_ofono_case_private_chat, 0 },

	BLTS_CLI_END_OF_LIST
};

static blts_cli my_ofono_cli =
{
	.test_cases = my_ofono_cases,
	.log_file = "blts_ofono_log.txt",
	.blts_cli_help = my_ofono_help,
	.blts_cli_process_arguments = my_ofono_argument_processor,
	.blts_cli_teardown = my_ofono_teardown
};

int main(int argc, char **argv)
{
	app_init_once();

	return blts_cli_main(&my_ofono_cli, argc, argv);
}

