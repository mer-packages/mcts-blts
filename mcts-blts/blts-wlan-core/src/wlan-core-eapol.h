/* wlan-core-eapol.h -- EAPOL handling

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

#ifndef WLAN_CORE_EAPOL_H
#define WLAN_CORE_EAPOL_H

#include "wlan-core-definitions.h"

enum {
	AUTH_STATE_PREAUTH,
	AUTH_STATE_4WAY_1_RCVD,
	AUTH_STATE_4WAY_2_SENT,
	AUTH_STATE_4WAY_3_RCVD,
	AUTH_STATE_4WAY_COMPLETE,
	AUTH_STATE_GROUP_HS_READY,
	AUTH_STATE_GROUP_HS_1_RCVD,
	AUTH_STATE_GROUP_HS_COMPLETE,
	AUTH_STATE_AUTHENTICATED,
	AUTH_STATE_DEAUTHENTICATED,
};

int eapol_register_callback(int (*on_change_cb)(int auth_status, void *user_data), void *data);
int eapol_unregister_callback(int (*on_change_cb)(int auth_status, void *user_data));

int eapol_loop_run(wlan_core_data *data, void *user_data);
void eapol_loop_unref(wlan_core_data *data);

int wpa_gen_wpa_ie_rsn(u8 *rsn_ie, size_t rsn_ie_len,
		int pairwise_cipher, int group_cipher,
		int key_mgmt, int mgmt_group_cipher);

const char *eapol_auth_state_as_string(int state);

#endif
