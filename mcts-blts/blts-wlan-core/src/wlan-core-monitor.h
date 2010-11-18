/* wlan-core-monitor.h -- Monitor interface handling

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

#ifndef WLAN_CORE_MONITOR_H
#define WLAN_CORE_MONITOR_H

#include "wlan-core-definitions.h"

int nl80211_create_monitor_iface(wlan_core_data *data);
void nl80211_remove_monitor_iface(wlan_core_data *data);

int monitor_extract_frame(wlan_core_data *data, u8 *buf, unsigned buf_sz, u8 **frame,
			unsigned *frame_len);
int monitor_send_raw_frame(wlan_core_data *data, void *frame, size_t frame_sz, int encrypt);

int monitor_sock_setup(wlan_core_data *data);
void monitor_sock_cleanup(wlan_core_data *data);

int monitor_send_l2_packet(wlan_core_data *data, u16 proto, void *buf, size_t len);

#endif /* WLAN_CORE_MONITOR_H */
