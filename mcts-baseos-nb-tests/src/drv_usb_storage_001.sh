#!/bin/sh
#DESCR: Check USB Storage in usbfs 
# Copyright (C) 2007, Intel Corporation.
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
# Authors:
#       Wang,Jing  <jing.j.wang@intel.com>
#
cd `dirname $0`
cat /proc/bus/usb/devices |grep "Cls=08(stor.) Sub=06"|grep "Driver=usb-storage"
if [ $? -ne 0 ]; then
	echo "Not find USB storage"
	cat /proc/bus/usb/devices
	exit 1
fi
exit 0

	


