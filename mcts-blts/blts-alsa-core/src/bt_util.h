/* bt_util.h -- Bluetooth connectivity helper

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

#ifndef BLTS_ALSA_CORE_BT_UTIL_H
#define BLTS_ALSA_CORE_BT_UTIL_H

int bt_util_connect_btaudio(char *remote_mac, char *pin);
int bt_util_disconnect_btaudio();

#endif /* BLTS_ALSA_CORE_BT_UTIL_H */
