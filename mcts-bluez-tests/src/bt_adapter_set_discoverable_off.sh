#!/bin/sh
#DESCR: Check to get discoverable status to Off for the local adapter. 
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
# Date Created: 2011/03/24
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#

# Enter the case folder
cd `dirname $0`
. ./data/bluetooth_env

ret=0

# By hciconfig, set to no-pscan and no-iscan
${TEST_ADAPTER} discoverable on
adapter=`hciconfig | head -n 1 | cut -d':' -f1`
hciconfig $adapter iscan
sleep 2

iscan=`${TEST_ADAPTER} discoverable`
[ $iscan -eq 0 ] && echo "[FAIL] hciconfig setting fail" && exit 1

# Set discoverable scan for the adapter 
${TEST_ADAPTER} discoverable off

# get power status
iscan=`${TEST_ADAPTER} discoverable`

if [ ${iscan} -eq 0 ]; then
    echo "[PASS] Adapter discoverable is Off now"
else
    echo "[FAIL] Adapter discoverable is still On"
    ret=1
fi

exit $ret
