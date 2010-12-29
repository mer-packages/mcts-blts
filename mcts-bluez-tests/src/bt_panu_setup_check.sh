#!/bin/sh
#DESCR: Set up PAN User and ping the PAN Server's IP (client gateway).
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
#       Guo, Zhaojuan  <zhaojuan.guo@intel.com>
# Date Created: 2010/12/02
#
# Modifications:
#          Modificator  Date
#          Content of Modification

cd `dirname $0`
. ./data/bluetooth_env

#make paring with your server device first 
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR

#run the script to connect the server
killall test_network > /dev/null
${DATA_DIR}/test_network $SERV_BD_ADDR nap >/dev/null & 

# Sleep for some seconds to let test_network finish.
sleep 5

#you will have bnep0 if the connection successful
ifconfig | grep "bnep0" > /dev/null
if [ $? -ne 0 ]; then
    echo "Have not build connection to server!"
    exit 1
else
    echo "Step1: Build connection to server successfully!" 
fi
 
#Assign ip for bnep0, server has been set to 192.168.0.1
ifconfig bnep0 192.168.0.2

#Add default gw
route add default gw 192.168.0.1
echo "Wait for 10 seconds."
sleep 10

#ping your server address
ping 192.168.0.1 -c 1 > /dev/null
if [ $? -ne 0 ]; then
    echo "[FAIL] no response recieved from adaptor $SERV_BD_ADDR"
    exit 1
fi

echo "[PASS] Step2: Ping gateway successfully. PANU profile passed"
exit 0
