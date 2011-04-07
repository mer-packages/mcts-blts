#!/bin/sh
#DESCR: Check if the stack packages are installed in system
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
P_Bluez=`rpm -qv bluez`
P_Obexd=`rpm -qv obexd`

if [ -z $P_Bluez ]; then
    echo "No Bluez package is found!"
    ret=1
else
    echo "[PASS] Bluez package is: $P_Bluez"
fi

if [ -z $P_Obexd ]; then
    echo "No obexd package is found!"
    ret=1
else
    echo "[PASS] Obexd(client) package is: $P_Obexd"
fi

# exit 
exit $ret
