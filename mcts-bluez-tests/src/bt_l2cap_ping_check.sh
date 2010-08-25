#!/bin/sh
#DESCR: Check l2cap socket communication using l2ping
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

# If both IUTS' adaptor support simple pairing feature, then the pairing agent is needed
# to handle the pairing request when making a l2cap connection.

${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

curr_user=`whoami`
if [ $curr_user != "root" ]; then
    ${DATA_DIR}/sudo_l2ping.sh $ROOT_PASS $SERV_BD_ADDR "l2ping.log"
else
    l2ping $SERV_BD_ADDR -c 4 >l2ping.log
fi

num_reply=`cat l2ping.log | grep "44 bytes from $SERV_BD_ADDR" -c`

if [ $curr_user != "root" ]; then
    ${DATA_DIR}/sudo_rm.sh $ROOT_PASS "l2ping.log"
else
    rm -rf l2ping.log
fi

if [ "$num_reply" -le 0 ]; then
    echo "no response recieved from adaptor $SERV_BD_ADDR"
    echo "Check l2cap socket communication using l2ping: Failed"
    exit 1
else
    echo "l2cap ping check passed, got $num_reply responses from $SERV_BD_ADDR"
    echo "Check l2cap socket communication using l2ping: Successfully"
fi

exit 0
