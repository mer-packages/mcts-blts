#!/bin/sh
#DESCR: Stressly test to get adapter power status to On/Off for 50 times. 
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
i=0

while [ $i -lt 50 -a $ret -ne 1 ];
do
    # check power status, if not ON, exit test.
    [ `${TEST_ADAPTER} powered` -eq 0 ] && "[FAIL] initial power is off" && ret=1

    # Power off adapter and check
    ${TEST_ADAPTER} powered off
    power=`${TEST_ADAPTER} powered`

    if [ ${power} -eq 1 ]; then
        echo "[FAIL] Adapter power is still ON, $i trial FAIILs!"
        ret=1
    else
        trials=`expr $i + 1`
        if [ `expr $trials % 10` -eq 0 ]; then
            echo "[PASS] Finish ${trials} trial."
        fi
    fi

    i=`expr $i + 1`
    
    # power on the adapter and prepare for next off
    ${TEST_ADAPTER} powered on
done

${TEST_ADAPTER} powered on
exit $ret
