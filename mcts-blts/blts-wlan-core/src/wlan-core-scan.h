/* wlan-core-enumerate.h -- WLAN core scan functions

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

#ifndef SCAN_H_
#define SCAN_H_

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

struct scan_res
{
        unsigned int flags;
        u8 bssid[ETH_ALEN];
        int freq;
        u16 beacon_int;
        u16 caps;
        int qual;
        int noise;
        int level;
        u64 tsf;
        unsigned int age;
        size_t ie_len;
        /* followed by ie_len octets of IEs */
};

struct scan_results
{
        struct scan_res **res;
        size_t num;
};

#define SCAN_AUTHENTICATED          BIT(0)
#define SCAN_ASSOCIATED             BIT(1)
#define SCAN_JOINED             	BIT(2)

const u8* scan_get_ie(struct scan_res *res, u8 ie);
int nl80211_scan_oneshot(wlan_core_data* data, const u8 *ssid, size_t ssid_len);
struct scan_res* get_bss_by_ssid(wlan_core_data *data, u8 *ssid, size_t ssid_len);
void scan_results_free(struct scan_results *res);
int nl80211_check_bss_connection_status(wlan_core_data* data, struct scan_results *res);
int nl80211_scan_for_link_data(wlan_core_data *data);
// ---------- test cases -------------

int scan_for_specific_ap(wlan_core_data* data);

#endif /*SCAN_H_*/
