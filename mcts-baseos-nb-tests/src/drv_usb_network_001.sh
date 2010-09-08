#!/bin/sh
#DESCR: Verify if the cpu is supported by Moblin
# Copyright (C) 2009 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
# 
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.  
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307 USA.
# 
#
# Authors:
#       Gong, Zhipeng <zhipeng.gong@intel.com>
cd `dirname $0`
found="no"
for i in /sys/bus/usb/drivers/*; do
        [ ! -e $i/*/net ] && continue
        echo "found USB-Ethernet converter at $i/*/net"
        hwaddress=`cat $i/*/net/*/address`
        echo "HW address is $hwaddress"
        found="yes"
done


if [ $found == "yes" ]; then
        ifconfig -a | grep $hwaddress -i -A 2 | grep "inet addr"
        if [ $? == "0" ]; then
                echo "the device has got IP address"
        else
                echo "the device has not got IP address"
                exit 1
        fi
else
	echo "can't fount USB-Ethernet converter"
	exit 1
fi

interface=`ifconfig -a | grep $hwaddress -i | awk '{print $1}'`
echo $interface

#ifconfig $interface down
#sleep 3
#ifconfig $interface up
#echo "wait for the ethernet controller to get ip"
#sleep 10

ifconfig -a | grep $hwaddress -i -A 2 | grep "inet addr"
[ $? -eq 0 ] || { echo "cannot find the ip address for ethernet controller after down/up"; exit 1; }

exit 0

