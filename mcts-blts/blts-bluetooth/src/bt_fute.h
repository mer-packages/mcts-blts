/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* bt_fute.c -- Bluez functional tests

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

#ifndef BT_FUTE_H
#define BT_FUTE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/* Test case numbering */
enum {
	CORE_BT_SCAN = 1,
	CORE_BT_CHECK,
	CORE_BT_RECEIVE_L2CAP,
	CORE_BT_CONNECT_L2CAP,
	CORE_BT_PING_PONG_L2CAP,
	CORE_BT_RECEIVE_RFCOMM,
	CORE_BT_CONNECT_RFCOMM,
	CORE_BT_PING_PONG_RFCOMM,
	CORE_BT_CONNECT_HCI,
	CORE_BT_TRANSFER_ACL_DATA_WITH_HCI,
	CORE_BT_RECEIVE_ACL_DATA_WITH_HCI,
	CORE_BT_CHANGE_NAME_WITH_HCI,
	CORE_BT_VERIFY_NAME_WITH_HCI,
	CORE_BT_CHANGE_CLASS_WITH_HCI,
	CORE_BT_VERIFY_CLASS_WITH_HCI,
	CORE_BT_RESET_CONNECTION_WITH_HCI,
	CORE_BT_AUDIT_INCOMING_HCI_CONNECTION,
	CORE_BT_READ_HCI_CONTROLLER_INFO_LOCAL,
	CORE_BT_READ_HCI_CONTROLLER_INFO_REMOTE,
	CORE_BT_READ_CONNECTED_LINK_INFO_LOCAL,
	CORE_BT_READ_CONNECTED_LINK_INFO_REMOTE,
	CORE_BT_AUTHENTICATION_WITH_PAIRING_AS_MASTER,
	CORE_BT_AUTHENTICATION_WITH_PAIRING_AS_SLAVE,
	CORE_BT_SIMPLE_PAIRING_AS_MASTER,
	CORE_BT_RECEIVE_SECURE_L2CAP,
	CORE_BT_SIMPLE_PAIRING_OOB_MASTER,
	CORE_BT_LE_SCAN,
	CORE_BT_LE_ADVERTISE,
	CORE_BT_LE_CONNECT
};

struct bt_data {
	char *mac_address;
	int dev_id;
	int agent_running;
};

int fute_bt_drivers_depcheck();
int fute_bt_scan(int);
int fute_bt_l2cap_echo_server(int);
int fute_bt_l2cap_echo_client(char *server_mac, int want_transfer_test, int);
int fute_bt_rfcomm_echo_server(int);
int fute_bt_rfcomm_echo_client(char *server_mac, int want_transfer_test, int);
int fute_bt_hci_connect_disconnect(char *remote_mac, int);
int fute_bt_hci_transfer_acl_data(char *remote_mac, int);
int fute_bt_hci_receive_acl_data(int);
int fute_bt_hci_change_name(int);
int fute_bt_hci_verify_name(char *remote_mac, int);
int fute_bt_hci_change_class(int);
int fute_bt_hci_verify_class(char *remote_mac, int);
int fute_bt_hci_reset_connection(char *remote_mac, int);
int fute_bt_hci_audit_incoming_connect(int);
int fute_bt_hci_ctrl_info_local(int);
int fute_bt_hci_ctrl_info_remote(char *remote_mac, int);
int fute_bt_hci_link_info_local(int);
int fute_bt_hci_link_info_remote(char *remote_mac,int);
int fute_bt_hci_ll_pairing(char* remote_mac, int master, int);
int fute_bt_hci_simple_pairing (bt_data *user_data, int master);
int fute_bt_hci_secure_l2cap_server (bt_data *user_data);
int fute_bt_hci_simple_pairing_oob (bt_data *user_data, int master);

int fute_bt_le_scan();
int fute_bt_le_advertise();
int fute_bt_le_connect_disconnect(char *remote_mac);
int fute_bt_le_tx_data(void*, int);
int fute_bt_le_rx_data(void*, int);


#endif /* BT_FUTE_H */
