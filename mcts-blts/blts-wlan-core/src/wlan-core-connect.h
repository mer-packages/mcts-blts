/* wlan-core-connect.h -- WLAN core connect functions

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

#ifndef CONNECT_H_
#define CONNECT_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>			/* getopt_long() */

#include <fcntl.h>			/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>

#include "wlan-core-definitions.h"

#define WPA_PROTO_WPA BIT(0)
#define WPA_PROTO_RSN BIT(1)

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)
#define WPA_AUTH_ALG_FT BIT(2)

typedef enum { WPA_ALG_NONE, WPA_ALG_WEP, WPA_ALG_TKIP, WPA_ALG_CCMP,
               WPA_ALG_IGTK, WPA_ALG_DHV } wpa_alg;
typedef enum { CIPHER_NONE, CIPHER_WEP40, CIPHER_TKIP, CIPHER_CCMP,
               CIPHER_WEP104 } wpa_cipher;
typedef enum { KEY_MGMT_802_1X, KEY_MGMT_PSK, KEY_MGMT_NONE,
               KEY_MGMT_802_1X_NO_WPA, KEY_MGMT_WPA_NONE } wpa_key_mgmt;

struct auth_params {
        int freq;
        const u8 *bssid;
        const u8 *ssid;
        size_t ssid_len;
        int auth_alg;
        const u8 *ie;
        size_t ie_len;
        const u8 *wep_key[4];
        size_t wep_key_len[4];
        int wep_tx_keyidx;
};

struct associate_params {
        const u8 *bssid;
        const u8 *ssid;
        size_t ssid_len;
        int freq;
        const u8 *wpa_ie;
        size_t wpa_ie_len;
        wpa_cipher pairwise_suite;
        wpa_cipher group_suite;
        wpa_key_mgmt key_mgmt_suite;
        int auth_alg;
        int mode;
        const u8 *wep_key[4];
        size_t wep_key_len[4];
        int wep_tx_keyidx;
        enum {
                NO_MGMT_FRAME_PROTECTION,
                MGMT_FRAME_PROTECTION_OPTIONAL,
                MGMT_FRAME_PROTECTION_REQUIRED
        } mgmt_frame_protection;
};

enum disconnect_type
{
	DEAUTHENTICATE,
	DISASSOCIATE,
	AP_LOSS
};

int nl80211_leave_ibss(wlan_core_data *data);
int nl80211_join_ibss(wlan_core_data *data, struct associate_params *params);

int nl80211_authenticate(wlan_core_data *data, struct auth_params *params);
int nl80211_associate(wlan_core_data *data, struct associate_params *params);
int nl80211_connect(wlan_core_data *data, struct associate_params *params);
int nl80211_deauthenticate(wlan_core_data *data, const u8 *addr, int reason_code);
int nl80211_disassociate(wlan_core_data *data, const u8 *addr, int reason_code);
int nl80211_disconnect(wlan_core_data *data, const u8 *addr, int reason_code);

int nl80211_associate_oneshot(wlan_core_data *data, struct associate_params* params);
int nl80211_authenticate_oneshot(wlan_core_data *data, struct auth_params* params);
int nl80211_connect_oneshot(wlan_core_data *data, struct associate_params* params);

// ------------  test cases ----------------

int associate_with_open_ap(wlan_core_data* data);
int associate_with_wep_ap(wlan_core_data* data);

int disconnect_from_open_ap(wlan_core_data* data, int disconnect_type);
int disconnect_from_open_adhoc_network(wlan_core_data* data);
#endif /*CONNECT_H_*/
