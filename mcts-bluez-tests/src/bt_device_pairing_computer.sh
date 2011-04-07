#!/bin/sh
#DESCR: Pair the bluetooth server computer.
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

# Firstly, try to remove the pairing, what ever it is really paired or not
${TEST_DEVICE} remove $SERV_BD_ADDR > /dev/null 2>&1

export DISPLAY=:0.0
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

ret=0 
# Get the paired computer.
${TEST_DEVICE} list | grep "$SERV_BD_ADDR" > /dev/null
if [ $? -eq 0 ]; then
    echo "[PASS] The computer server is paired successfully!"
else
    echo "[FAIL] The computer server is not paired."
    ret=1 
fi

exit $ret
