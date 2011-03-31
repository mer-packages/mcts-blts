#!/bin/sh
#DESCR: Pair the bluetooth server computer and check to remove it.
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
#       Zhang Jingke  <jingke.zhang@intel.com>
# Date Created: 2010/01/12
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

if [ -z $hci0 ]; then
    echo "No adaptor found!"
    exit 1
fi

# Firstly, try to pair the computer
export DISPLAY=:0.0
${TEST_DEVICE} remove $SERV_BD_ADDR > /dev/null 2>&1
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

ret=0 
# make sure computer is paired successfully
${TEST_DEVICE} list | grep "$SERV_BD_ADDR" > /dev/null
if [ $? -eq 0 ]; then
    echo "[PASS] The computer server is paired successfully!"
else
    echo "[FAIL] The computer server is not paired."
    ret=0 
fi

# Remove the paired computer, and check again.
sleep 2
${TEST_DEVICE} remove $SERV_BD_ADDR

${TEST_DEVICE} list | grep "$SERV_BD_ADDR" > /dev/null
if [ $? -ne 0 ]; then
    echo "[PASS] The computer server is removed successfully!"
else
    echo "[FAIL] The computer server is not removed."
    ret=0 
fi

exit $ret
