#!/bin/sh
#DESCR: Check pbap profile
# Copyright (C) 2010 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Authors:
#       Guo Zhaojuan  <zhaojuan.guo@intel.com>
# Date Created: 2010/12/09
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#

# Enter the case folder
cd `dirname $0`
. ./data/bluetooth_env

adaptors=`hciconfig  | grep "BD Address" | awk '{print $3}'`
hci0=`echo "$adaptors" | sed -n '1p'`
export DISPLAY=:0.0

if [ -z $hci0 ]; then
    echo "No adaptor found!"
    exit 1
fi

# If both IUTS' adaptor support simple pairing feature, then the pairing agent is needed
# to handle the pairing request when making a l2cap connection.

${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

contacts=`${PBAP_CLIENT} $SERV_BD_ADDR | grep "BEGIN:VCARD" | wc -l`

if [ $contacts -eq 0 ]; then
    echo "[FAIL] Fail to get the phonebook from remote"
    exit 1
fi

echo "[PASS] Successfully get the phonebook from remote"
exit 0
