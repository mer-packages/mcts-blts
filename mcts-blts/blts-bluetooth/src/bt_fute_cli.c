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

#include <blts_log.h>

#include "bt_fute.h"

void show_help(char* arg0)
{
	fprintf(stderr, "Usage: %s <logfile> <test> <test-args>\n", arg0);
	fprintf(stderr, "Tests:                                                  Args:\n");
	fprintf(stderr, "Core-Bluetooth scan\n");
	fprintf(stderr, "Core-Bluetooth drivers and userspace check\n");
	fprintf(stderr, "Core-Bluetooth receive L2CAP connection\n");
	fprintf(stderr, "Core-Bluetooth connect with L2CAP                       <server-mac>\n");
	fprintf(stderr, "Core-Bluetooth ping-pong transfer with L2CAP            <server-mac>\n");
	fprintf(stderr, "Core-Bluetooth receive RFCOMM connection\n");
	fprintf(stderr, "Core-Bluetooth connect with RFCOMM                      <server-mac>\n");
	fprintf(stderr, "Core-Bluetooth ping-pong transfer with RFCOMM           <server-mac>\n");
	fprintf(stderr, "Core-Bluetooth connect with HCI                         <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth transfer ACL data with HCI               <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth receive ACL data with HCI\n");
	fprintf(stderr, "Core-Bluetooth change name with HCI\n");
	fprintf(stderr, "Core-Bluetooth verify name with remote HCI              <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth change class with HCI\n");
	fprintf(stderr, "Core-Bluetooth verify class with remote HCI             <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth reset connection with HCI                <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth audit incoming HCI connection\n");
	fprintf(stderr, "Core-Bluetooth Read HCI controller information local\n");
	fprintf(stderr, "Core-Bluetooth Read HCI controller information remote   <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth Read connected link information local\n");
	fprintf(stderr, "Core-Bluetooth Read connected link information remote   <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth authentication with pairing as master    <remote-mac>\n");
	fprintf(stderr, "Core-Bluetooth authentication with pairing as slave\n");
}

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		show_help(argv[0]);
		exit(1);
	}


	int retval = -1;
	if(!strcmp(argv[2],"Core-Bluetooth scan"))
	{
		log_open(argv[1],1);
		retval = fute_bt_scan();
		log_close();
	}
	else if(!strcmp(argv[2],"Core-Bluetooth drivers and userspace check"))
	{
		log_open(argv[1],1);
		retval = fute_bt_drivers_depcheck();
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth receive L2CAP connection"))
	{
		log_open(argv[1],1);
		retval = fute_bt_l2cap_echo_server();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth connect with L2CAP")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_l2cap_echo_client(argv[3],0);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth ping-pong transfer with L2CAP")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_l2cap_echo_client(argv[3],1);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth receive RFCOMM connection"))
	{
		log_open(argv[1],1);
		retval = fute_bt_rfcomm_echo_server();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth connect with RFCOMM")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_rfcomm_echo_client(argv[3],0);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth ping-pong transfer with RFCOMM")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_rfcomm_echo_client(argv[3],1);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth connect with HCI")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_connect_disconnect(argv[3]);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth transfer ACL data with HCI")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_transfer_acl_data(argv[3]);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth receive ACL data with HCI"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_receive_acl_data();
		log_close();
	}

	else if (!strcmp(argv[2],"Core-Bluetooth change name with HCI"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_change_name();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth verify name with remote HCI")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_verify_name(argv[3]);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth change class with HCI"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_change_class();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth verify class with remote HCI")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_verify_class(argv[3]);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth reset connection with HCI")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_reset_connection(argv[3]);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth audit incoming HCI connection"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_audit_incoming_connect();
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth Read HCI controller information local"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_ctrl_info_local();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth Read HCI controller information remote")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_ctrl_info_remote(argv[3]);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth Read connected link information local"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_link_info_local();
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth Read connected link information remote")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_link_info_remote(argv[3]);
		log_close();
	}
	else if ((!strcmp(argv[2],"Core-Bluetooth authentication with pairing as master")) && argc == 4)
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_ll_pairing(argv[3], 1);
		log_close();
	}
	else if (!strcmp(argv[2],"Core-Bluetooth authentication with pairing as slave"))
	{
		log_open(argv[1],1);
		retval = fute_bt_hci_ll_pairing("00:00:00:00:00:00", 0);
		log_close();
	}
	else
	{
		show_help(argv[0]);
		exit(1);
	}

	printf("Test %s.\n",retval?"FAILED":"PASSED");

	return retval;
}

