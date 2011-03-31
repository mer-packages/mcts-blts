#!/bin/sh
#DESCR: Check to get pairable status to On for the local adapter. 
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

# Set pairable to Off
${TEST_ADAPTER} pairable off
pair=`${TEST_ADAPTER} pairable`
[ $pair -eq 1 ] && echo "[FAIL] pairable cannot be initialized to Off" && exit 1

# Set pairable scan for the adapter 
${TEST_ADAPTER} pairable on

# get power status
pair=`${TEST_ADAPTER} pairable`

if [ ${pair} -eq 1 ]; then
    echo "[PASS] Adapter pairable is ON"
else
    echo "[FAIL] Adapter pairable is still OFF"
    ret=1
fi

exit $ret
