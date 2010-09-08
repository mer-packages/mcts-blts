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
. ./helper

interface_name=`get_internal_network_interface`
[ $? -eq 0 ] || { echo "cannot find ethernet controller"; exit 1; }
/sbin/ifconfig $interface_name 
[ $? -eq 0 ] || { echo "cannot ifconfig ethernet"; exit 1; }

#ifconfig eth0 down
#sleep 3
#ifconfig eth0 up
#echo "wait for the ethernet controller to get ip"
#sleep 10

#ifconfig eth0 | grep 'inet addr:'
#[ $? -eq 0 ] || { echo "cannot find the ip address for ethernet controller after down/up"; exit 1; }

exit 0

