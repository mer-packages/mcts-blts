#!/bin/sh
#DESCR: Check if bluetoothd has been started
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
#       Jingke Zhang  <jignke.zhang@intel.com>
# Date Created: 2010/01/12
#
# Modifications:
#          Modificator  Date
#          Content of Modification

# Enter the case folder
cd `dirname $0`
. ./data/bluetooth_env


bt_daemon_pid=`ps aux | grep bluetoothd | grep -v grep | awk '{print $2}'`

if [ -z $bt_daemon_pid ]; then
    echo "bluetoothd daemon isn't started, this is a crital failure"
    echo "Check if bluetoothd daemon started: Failed"
    exit 1
else
    echo "Check if bluetoothd daemon started: Successfully"
    echo "bluetoothd daemon has been started, the pid is: $bt_daemon_pid"
    exit 0
fi
