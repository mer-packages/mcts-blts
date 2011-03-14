/* bt_fute_cli.c -- Bluez functional tests

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>

#include "bt_fute.h"

static void bt_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-m]",
		"  -m: remote/server MAC address (format: \"00:00:00:00:00\")\n");
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "m:";
static const struct option long_opts[] =
{
	{"mac-address", no_argument, NULL, 'm'},
	{0,0,0,0}
};


/* Return NULL in case of an error */
static void* bt_argument_processor(int argc, char **argv)
{
	int c;
	int ret;
	struct bt_data *my_data = malloc(sizeof(*my_data));
	memset(my_data, 0, sizeof(*my_data));

	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch (c)
		{
		case 'm':
			if (my_data->mac_address)
				free(my_data->mac_address);
			my_data->mac_address = strdup(optarg);
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	if (!my_data->mac_address)
	{
		ret	= blts_config_get_value_string("mac_address", &my_data->mac_address);
		if (ret)
		{
			BLTS_WARNING("Cannot read mac_address value from config file\n");
			BLTS_WARNING("Defaulting to 00:00:00:00:00:00\n");
			my_data->mac_address = strdup("00:00:00:00:00:00");
		}
	}

	BLTS_DEBUG("MAC address to use: %s\n", my_data->mac_address);

	return my_data;
}

/* user_ptr is what you returned from bt_argument_processor */
static void bt_teardown(void *user_ptr)
{
	if(user_ptr)
	{
		struct bt_data *data = (struct bt_data *)user_ptr;

		if (data->mac_address)
			free(data->mac_address);

		free(user_ptr);
	}
}

/* user_ptr is what you returned from bt_argument_processor
 * test_num Test case being run from bt_cases, starts from 1
 * return non-zero in case of an error */


static int bt_run_case(void* user_ptr, int test_num)
{
	int ret = 0;
	struct bt_data *data = (struct bt_data *)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);

	switch (test_num)
	{
	case CORE_BT_SCAN:
		ret = fute_bt_scan();
		break;
	case CORE_BT_CHECK:
		ret = fute_bt_drivers_depcheck();
		break;
	case CORE_BT_RECEIVE_L2CAP:
		ret = fute_bt_l2cap_echo_server();
		break;
	case CORE_BT_CONNECT_L2CAP:
		ret = fute_bt_l2cap_echo_client(data->mac_address,0);
		break;
	case CORE_BT_PING_PONG_L2CAP:
		ret = fute_bt_l2cap_echo_client(data->mac_address,1);
		break;
	case CORE_BT_RECEIVE_RFCOMM:
		ret = fute_bt_rfcomm_echo_server();
		break;
	case CORE_BT_CONNECT_RFCOMM:
		ret = fute_bt_rfcomm_echo_client(data->mac_address,0);
		break;
	case CORE_BT_PING_PONG_RFCOMM:
		ret = fute_bt_rfcomm_echo_client(data->mac_address,1);
		break;
	case CORE_BT_CONNECT_HCI:
		ret = fute_bt_hci_connect_disconnect(data->mac_address);
		break;
	case CORE_BT_TRANSFER_ACL_DATA_WITH_HCI:
		ret = fute_bt_hci_transfer_acl_data(data->mac_address);
		break;
	case CORE_BT_RECEIVE_ACL_DATA_WITH_HCI:
		ret = fute_bt_hci_receive_acl_data();
		break;
	case CORE_BT_CHANGE_NAME_WITH_HCI:
		ret = fute_bt_hci_change_name();
		break;
	case CORE_BT_VERIFY_NAME_WITH_HCI:
		ret = fute_bt_hci_verify_name(data->mac_address);
		break;
	case CORE_BT_CHANGE_CLASS_WITH_HCI:
		ret = fute_bt_hci_change_class();
		break;
	case CORE_BT_VERIFY_CLASS_WITH_HCI:
		ret = fute_bt_hci_verify_class(data->mac_address);
		break;
	case CORE_BT_RESET_CONNECTION_WITH_HCI:
		ret = fute_bt_hci_reset_connection(data->mac_address);
		break;
	case CORE_BT_AUDIT_INCOMING_HCI_CONNECTION:
		ret = fute_bt_hci_audit_incoming_connect();
		break;
	case CORE_BT_READ_HCI_CONTROLLER_INFO_LOCAL:
		ret = fute_bt_hci_ctrl_info_local();
		break;
	case CORE_BT_READ_HCI_CONTROLLER_INFO_REMOTE:
		ret = fute_bt_hci_ctrl_info_remote(data->mac_address);
		break;
	case CORE_BT_READ_CONNECTED_LINK_INFO_LOCAL:
		ret = fute_bt_hci_link_info_local();
		break;
	case CORE_BT_READ_CONNECTED_LINK_INFO_REMOTE:
		ret = fute_bt_hci_link_info_remote(data->mac_address);
		break;
	case CORE_BT_AUTHENTICATION_WITH_PAIRING_AS_MASTER:
		ret = fute_bt_hci_ll_pairing(data->mac_address, 1);
		break;
	case CORE_BT_AUTHENTICATION_WITH_PAIRING_AS_SLAVE:
		ret = fute_bt_hci_ll_pairing("00:00:00:00:00:00", 0);
		break;

#ifdef HAVE_BTLE_API
	case CORE_BT_LE_SCAN:
		ret = fute_bt_le_scan();
		break;
	case CORE_BT_LE_ADVERTISE:
		ret = fute_bt_le_advertise();
		break;
	case CORE_BT_LE_CONNECT:
		ret = fute_bt_le_connect_disconnect(data->mac_address);
		break;
#else
	case CORE_BT_LE_SCAN:
	case CORE_BT_LE_ADVERTISE:
	case CORE_BT_LE_CONNECT:
		ret = -1;
		BLTS_ERROR("ERROR: Bluetooth LE support was not compiled in.\n");
		BLTS_ERROR("Use a newer version of Bluez when building the tests.\n");
		break;
#endif
	default:
		BLTS_DEBUG("Not supported case number %d\n", test_num);
	}

	return ret;
}

static blts_cli_testcase bt_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity */

	{ "Core-Bluetooth scan", bt_run_case, 60000 },
	{ "Core-Bluetooth drivers and userspace check", bt_run_case, 10000 },
	{ "Core-Bluetooth receive L2CAP connection", bt_run_case, 35000 },
	{ "Core-Bluetooth connect with L2CAP", bt_run_case, 10000 },
	{ "Core-Bluetooth ping-pong transfer with L2CAP", bt_run_case, 10000 },
	{ "Core-Bluetooth receive RFCOMM connection", bt_run_case, 35000 },
	{ "Core-Bluetooth connect with RFCOMM", bt_run_case, 10000 },
	{ "Core-Bluetooth ping-pong transfer with RFCOMM", bt_run_case, 10000 },
	{ "Core-Bluetooth connect with HCI", bt_run_case, 10000 },
	{ "Core-Bluetooth transfer ACL data with HCI", bt_run_case, 10000 },
	{ "Core-Bluetooth receive ACL data with HCI", bt_run_case, 35000 },
	{ "Core-Bluetooth change name with HCI", bt_run_case, 35000 },
	{ "Core-Bluetooth verify name with remote HCI", bt_run_case, 10000 },
	{ "Core-Bluetooth change class with HCI", bt_run_case, 35000 },
	{ "Core-Bluetooth verify class with remote HCI", bt_run_case, 35000 },
	{ "Core-Bluetooth reset connection with HCI", bt_run_case, 35000 },
	{ "Core-Bluetooth audit incoming HCI connection", bt_run_case, 35000 },
	{ "Core-Bluetooth Read HCI controller information local", bt_run_case, 35000 },
	{ "Core-Bluetooth Read HCI controller information remote", bt_run_case, 10000 },
	{ "Core-Bluetooth Read connected link information local", bt_run_case, 35000 },
	{ "Core-Bluetooth Read connected link information remote", bt_run_case, 35000 },
	{ "Core-Bluetooth authentication with pairing as master", bt_run_case, 10000 },
	{ "Core-Bluetooth authentication with pairing as slave", bt_run_case, 35000 },
	{ "Core-Bluetooth LE scan", bt_run_case, 60000 },
	{ "Core-Bluetooth LE advertise", bt_run_case, 60000 },
	{ "Core-Bluetooth LE connect", bt_run_case, 60000 },
	{ "Core-Bluetooth LE transmit data", fute_bt_le_tx_data, 60000 },
	{ "Core-Bluetooth LE receive data", fute_bt_le_rx_data, 60000 },

	BLTS_CLI_END_OF_LIST
};

static blts_cli bt_cli =
{
	.test_cases = bt_cases,
	.log_file = "blts_bt_log.txt",
	.blts_cli_help = bt_help,
	.blts_cli_process_arguments = bt_argument_processor,
	.blts_cli_teardown = bt_teardown
};

int main(int argc, char **argv)
{
	/* You can do something here if you wish */
	return blts_cli_main(&bt_cli, argc, argv);
}
