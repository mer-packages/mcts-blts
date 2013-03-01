/* wlan-core-cli.c -- WLAN core tests command line interface

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
#include <getopt.h>

#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>
#include <blts_timing.h>
#include "wlan-core-ioctl.h"
#include "wlan-core-profiler.h"
#include "wlan-core-definitions.h"
#include "wlan-core-enumerate.h"
#include "wlan-core-utils.h"
#include "wlan-core-scan.h"
#include "wlan-core-connect.h"
#include "wlan-core-wpa.h"
#include "wlan-core-adhoc.h"

static void app_deinitialize(void * user_ptr);
static void app_init_once(wlan_core_data *data);

static void wlan_core_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-p] [-S] [--nltrace]",
		"  -p: Enables profiling of ioctl calls\n"
		"  -S: (Power)Save mode on\n"
		"  --nltrace: Output trace of performed nl80211 commands\n"
		);
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "pS";
static const struct option long_opts[] =
{
	{"profiling", no_argument, NULL, 'p'},
	{"savemode", no_argument, NULL, 'S'},
	{"nltrace", 0, NULL, 0},
	{0,0,0,0}
};

/* Parse expected phy values from variable list */
static void *variant_args_expected_phys(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = (wlan_core_data *) user_ptr;

	if (!user_ptr || !data->cmd) {
		BLTS_ERROR("Error: Invalid argument in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (!args) {
		BLTS_WARNING("Warning: No arguments to process in %s:%s\n",__FILE__,__func__);
		return user_ptr;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->expected_phys = blts_config_boxed_value_get_string(args);
	return data;
}

/* Parse device name parameter from variable list */
static void *variant_args_dev(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = (wlan_core_data *) user_ptr;

	if (!user_ptr || !data->cmd) {
		BLTS_ERROR("Error: Invalid argument in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (!args) {
		BLTS_WARNING("Warning: No arguments to process in %s:%s\n",__FILE__,__func__);
		return user_ptr;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->ifname = blts_config_boxed_value_get_string(args);
	return data;
}

/* Parse device name and ssid from variable list */
static void *variant_args_dev_and_ssid(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = variant_args_dev(args, user_ptr);

	if (!data)
	{
		return NULL;
	}

	args = args->next; /* skip dev */

	if (!args)
	{
		BLTS_WARNING("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_STRING)
	{
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->ssid = blts_config_boxed_value_get_string(args);
	return data;
}

/* Parse device name ssid and wep keys from variable list */
static void *variant_args_dev_ssid_and_wep_keys(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = variant_args_dev_and_ssid(args, user_ptr);

	if (!data)
	{
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args)
	{
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	args = args->next; /* skip ssid */
	if (!args)
	{
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_STRING)
	{
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->wep_keys = blts_config_boxed_value_get_string(args);

    args = args->next;

    if (!args)
    {
    	BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
        return NULL;
    }

    if (args->type != CONFIG_PARAM_INT && args->type != CONFIG_PARAM_LONG)
    {
		BLTS_ERROR ("Error: Type mismatch in %s:%s\n", __FILE__, __func__);
		return NULL;
    }

    data->cmd->wep_keyidx = blts_config_boxed_value_get_int(args);

	return data;
}

static void *variant_args_dev_ssid_and_wpapsk(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = variant_args_dev_and_ssid(args, user_ptr);

	if (!data) {
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	args = args->next; /* skip ssid */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->wpa_passphrase = blts_config_boxed_value_get_string(args);

	return data;
}

static void *variant_args_dev_ssid_and_channel(struct boxed_value *args, void *user_ptr)
{
	wlan_core_data* data = variant_args_dev_and_ssid(args, user_ptr);

	if (!data) {
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	args = args->next; /* skip ssid */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_INT && args->type != CONFIG_PARAM_LONG) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->cmd->channel = blts_config_boxed_value_get_int(args);

	return data;
}


/* Return NULL in case of an error */
static void* wlan_core_argument_processor(int argc, char **argv)
{
	int c, ret, opt_index;
	wlan_core_data* my_data = malloc(sizeof (*my_data));
	memset(my_data, 0, sizeof(*my_data));

	app_init_once(my_data);

	while((c = getopt_long(argc, argv, short_opts, long_opts, &opt_index)) != -1)
	{
		switch(c)
		{
		case 0:
			if(!strcmp("nltrace", long_opts[opt_index].name))
				my_data->trace_netlink = 1;
			fprintf(stderr, "Netlink trace enabled\n");
			break;
		case 'p':
			my_data->flags |= CLI_FLAG_PROFILING;
			break;
		case 'S':
			my_data->flags |= CLI_FLAG_POWER_SAVE;
			break;

		default:
			free(my_data);
			return NULL;
		}
	}

	if (blts_log_get_level() == LEVEL_TRACE)
		my_data->flags |= CLI_FLAG_VERBOSE_LOG;

	ret = blts_config_declare_variable_test("Core-WLAN-Enumerate hardware",
			variant_args_expected_phys,
			CONFIG_PARAM_STRING, "expected_phys", "phy0",
			CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Enumerate supported features",
		variant_args_dev,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Scan for specific AP",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Associate with open AP",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Associate with WEP encryption",
		variant_args_dev_ssid_and_wep_keys,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "wep_ssid", "test-WEP",
//		CONFIG_PARAM_STRING, "wep_keys", "0:a:abcde",
		CONFIG_PARAM_STRING, "wep_keys", "0:h:1234567890",
		CONFIG_PARAM_INT, "wep_tx_keyidx", 0,
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Associate with WPA2-PSK AP",
		variant_args_dev_ssid_and_wpapsk,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "wpa_ssid", "test-WPA",
		CONFIG_PARAM_STRING, "wpa_passphrase", "abcdefgh",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Scan and associate on max power save",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Disconnect with deauthenticate",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Disconnect with disassociate",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Disconnect with AP loss",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "open_ssid", "test-open",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Disconnect from adhoc network",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "adhoc_ssid", "test-adhoc",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Establish new adhoc network",
		variant_args_dev_ssid_and_channel,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "adhoc_ssid2", "test-adhoc2",
		CONFIG_PARAM_INT, "adhoc_channel", 1,
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-WLAN-Join established adhoc network",
		variant_args_dev_and_ssid,
		CONFIG_PARAM_STRING, "wlan_device", "wlan0",
		CONFIG_PARAM_STRING, "adhoc_ssid2", "test-adhoc2",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	return my_data;
}

/* user_ptr is what you returned from my_example_argument_processor */
static void wlan_core_teardown(void *user_ptr)
{
	wlan_core_data* data = (wlan_core_data*)user_ptr;

	if(user_ptr)
	{
		// free allocated memory
		app_deinitialize(data);
		free(user_ptr);
	}
}

/* user_ptr is what you returned from my_example_argument_processor
 * test_num Test case being run from my_example_cases, starts from 1
 * return non-zero in case of an error */

static int wlan_core_case_hardware_enum(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = enum_hardware(data);

	return ret;
}

static int wlan_core_case_supported_features_enum(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = enum_features(data);

	return ret;
}

static int wlan_core_case_scan_for_specific_ap(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = scan_for_specific_ap(data);

	return ret;
}

static int wlan_core_case_associate_with_open_ap(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = associate_with_open_ap(data);

	return ret;
}

static int wlan_core_case_associate_with_wep_ap(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = associate_with_wep_ap(data);

	return ret;
}

static int wlan_core_case_scan_and_associate_on_power_save(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	data->cmd->power_save = 1;
	ret = nl80211_set_power_save(data);
	if(ret)
	{
		BLTS_ERROR("Set power save failed!\n");
		return ret;
	}

	BLTS_DEBUG("Power save is on...\n");
	ret = associate_with_open_ap(data);

	/* if power save mode is undefined (-1) here, test fails */
	if(!ret && data->cmd->power_save == -1)
	{
		BLTS_ERROR("Test case executed, but power save mode is undefined\n");
		ret = -1;
	}

	return ret;
}

static int wlan_core_case_associate_with_wpa2psk_ap(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = associate_with_wpa2psk_ap(data);

	return ret;
}

static int wlan_core_case_disconnect_with_deauthenticate(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = disconnect_from_open_ap(data, DEAUTHENTICATE);

	return ret;
}

static int wlan_core_case_disconnect_with_disassociate(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = disconnect_from_open_ap(data, DISASSOCIATE);

	return ret;
}

static int wlan_core_case_disconnect_with_ap_loss(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = disconnect_from_open_ap(data, AP_LOSS);

	return ret;
}

static int wlan_core_case_disconnect_from_adhoc_network(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = disconnect_from_open_adhoc_network(data);

	return ret;
}

static int wlan_core_case_establish_new_adhoc_network(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = create_open_adhoc_network(data);

	return ret;
	
}

static int wlan_core_case_join_established_adhoc_network(void* user_ptr, int test_num)
{
	int ret;

	wlan_core_data* data = (wlan_core_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	ret = join_established_open_adhoc_network(data);

	return ret;	
}

static void app_deinitialize(void * user_ptr)
{
	wlan_core_data* data = (wlan_core_data*)user_ptr;

	scan_results_free(data->scan_res);

	free(data->cmd);

	/* try to run commands which are needed after testing */
        if(run_commands_before_and_after_testing("restart_commands"))
                BLTS_ERROR("Run commands after testing failed!\n");

	/* try to restart processes that were stopped during initialization */	
	if(restart_processes_after_testing())
		BLTS_ERROR("Restart processes after testing failed!\n");

}

static void app_init_once(wlan_core_data *data)
{
	static int init_done = 0;

	if (init_done)
		return;

	/* try to run commands which are needed before testing */
	if(run_commands_before_and_after_testing("stop_commands"))
		BLTS_ERROR("Run commands before testing failed!\n");
		
	/* try to shutdown processes that may prevent test case execution */
	if(stop_processes_before_testing())
		BLTS_ERROR("Stop processes before testing failed!\n");

	init_done = 1;

	data->cmd = (wlan_command_data*)malloc(sizeof(wlan_command_data));
	memset(data->cmd, 0, sizeof(wlan_command_data));
	if (data->cmd==NULL)
	{
		BLTS_DEBUG("Not enough memory to allocate wlan data\n");
		exit(1);
	}
	data->cmd->expected_phys = NULL;
	data->cmd->ifname = NULL;
	data->cmd->ssid = NULL;
	data->scan_res = NULL;
	//TODO: necessary initilization here
}

static int wlan_core_run_case(void* user_ptr, int test_num)
{
	int ret = 0;
	wlan_core_data* data = (wlan_core_data*)user_ptr;

	BLTS_DEBUG("nl80211_init...\n");
	ret = nl80211_init(data);
	if(ret)
	{
		BLTS_DEBUG("nl80211_init failed!\n");
		return -1;
	}

	if(data->flags&CLI_FLAG_PROFILING)
	{
		ioctl_start_profiling();
	}

	if(nl80211_finish_device_init(data, test_num))
	{
		ret = -1;
		goto skip;
	}

	switch (test_num)
	{
		case CORE_HARDWARE_ENUMERATION: ret = wlan_core_case_hardware_enum(user_ptr, test_num); break;
		case CORE_SUPPORTED_FEATURES_ENUMERATION: ret = wlan_core_case_supported_features_enum(user_ptr, test_num); break;
		case CORE_SCAN_FOR_SPECIFIC_AP: ret = wlan_core_case_scan_for_specific_ap(user_ptr, test_num); break;
		case CORE_ASSOCIATE_WITH_OPEN_AP: ret = wlan_core_case_associate_with_open_ap(user_ptr, test_num); break;
		case CORE_ASSOCIATE_WITH_WEP_AP: ret = wlan_core_case_associate_with_wep_ap(user_ptr, test_num); break;
		case CORE_SCAN_AND_ASSOCIATE_POWER_SAVE: ret = wlan_core_case_scan_and_associate_on_power_save(user_ptr, test_num); break;
		case CORE_ASSOCIATE_WITH_WPA2PSK_AP: ret = wlan_core_case_associate_with_wpa2psk_ap(user_ptr, test_num); break;
		case CORE_DISCONNECT_WITH_DEAUTHENTICATE: ret = wlan_core_case_disconnect_with_deauthenticate(user_ptr, test_num); break;
		case CORE_DISCONNECT_WITH_DISASSOCIATE: ret = wlan_core_case_disconnect_with_disassociate(user_ptr, test_num); break;
		case CORE_DISCONNECT_WITH_AP_LOSS: ret = wlan_core_case_disconnect_with_ap_loss(user_ptr, test_num); break;
		case CORE_DISCONNECT_FROM_ADHOC_NETWORK: ret = wlan_core_case_disconnect_from_adhoc_network(user_ptr, test_num); break;
		case CORE_ESTABLISH_NEW_ADHOC_NETWORK: ret = wlan_core_case_establish_new_adhoc_network(user_ptr, test_num); break;
		case CORE_JOIN_ESTABLISHED_ADHOC_NETWORK: ret = wlan_core_case_join_established_adhoc_network(user_ptr, test_num); break;
		default: BLTS_DEBUG("Not supported case number%d\n", test_num);
	}

skip:

	if(data->flags&CLI_FLAG_PROFILING)
	{
		ioctl_print_profiling_data();
		ioctl_stop_profiling();
	}

	nl80211_cleanup(data);

	return ret;
}

static blts_cli_testcase wlan_core_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity */
	{ "Core-WLAN-Enumerate hardware", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Enumerate supported features", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Scan for specific AP", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Associate with open AP", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Associate with WEP encryption", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Scan and associate on max power save", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Associate with WPA2-PSK AP", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Disconnect with deauthenticate", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Disconnect with disassociate", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Disconnect with AP loss", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Disconnect from adhoc network", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Establish new adhoc network", wlan_core_run_case, 60000 },
	{ "Core-WLAN-Join established adhoc network", wlan_core_run_case, 60000 },
	
	BLTS_CLI_END_OF_LIST
};

static blts_cli wlan_core_cli =
{
	.test_cases = wlan_core_cases,
	.log_file = "blts_wlan_core_log.txt",
	.blts_cli_help = wlan_core_help,
	.blts_cli_process_arguments = wlan_core_argument_processor,
	.blts_cli_teardown = wlan_core_teardown
};

int main(int argc, char **argv)
{
	/* You can do something here if you wish */
	return blts_cli_main(&wlan_core_cli, argc, argv);
}

