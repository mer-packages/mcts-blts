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

int fute_bt_drivers_depcheck();
int fute_bt_scan();
int fute_bt_l2cap_echo_server();
int fute_bt_l2cap_echo_client(char *server_mac, int want_transfer_test);
int fute_bt_rfcomm_echo_server();
int fute_bt_rfcomm_echo_client(char *server_mac, int want_transfer_test);
int fute_bt_hci_connect_disconnect(char *remote_mac);
int fute_bt_hci_transfer_acl_data(char *remote_mac);
int fute_bt_hci_receive_acl_data();
int fute_bt_hci_change_name();
int fute_bt_hci_verify_name(char *remote_mac);
int fute_bt_hci_change_class();
int fute_bt_hci_verify_class(char *remote_mac);
int fute_bt_hci_reset_connection(char *remote_mac);
int fute_bt_hci_audit_incoming_connect();
int fute_bt_hci_ctrl_info_local();
int fute_bt_hci_ctrl_info_remote(char *remote_mac);
int fute_bt_hci_link_info_local();
int fute_bt_hci_link_info_remote(char *remote_mac);
int fute_bt_hci_ll_pairing(char* remote_mac, int master);

#endif /* BT_FUTE_H */
