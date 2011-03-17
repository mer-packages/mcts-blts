/* wlan-core-utils.h -- WLAN core utility functions

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

#ifndef UTILS_H_
#define UTILS_H_

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

int nl80211_set_mode(wlan_core_data* data, int mode);

int nl80211_init(wlan_core_data* data);
void nl80211_cleanup(wlan_core_data* data);

int nl80211_finish_device_init(wlan_core_data* data, int test_num);

char* find_parent_phy_device(const char *interface);

int phy_lookup(char *name);
int send_and_recv_msgs(wlan_core_data* data, struct nl_msg *msg, int (*valid_handler)(struct nl_msg *, void *), void *valid_data);

int set_iface_flags(int sock, const char *ifname, int dev_up);

int ieee80211_frequency_to_channel(int freq);
int ieee80211_channel_to_frequency(int chan);

int mac_addr_a2n(unsigned char *mac_addr, char *arg);
int mac_addr_n2a(char *mac_addr, unsigned char *arg);

int hexstr2bin(const char *hex, u8 *buf, size_t len);

int nl80211_set_power_save(wlan_core_data* data);

int nl80211_set_key_ccmp(wlan_core_data *data, const u8 *addr,
			int key_idx, int set_tx,
			const u8 *seq, size_t seq_len,
			const u8 *key, size_t key_len);

int nl80211_set_supp_port(wlan_core_data *data, int authorized);

int nl_get_multicast_id(wlan_core_data* data, const char *family, const char *group);

int stop_processes_before_testing(void);
int restart_processes_after_testing(void);

#endif /*UTILS_H_*/
