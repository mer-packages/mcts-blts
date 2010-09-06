#!/bin/sh
#DESCR: Test if usb internal hub auto suspend feature works when hub is idle
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
. ./functions

mount -t usbfs none /proc/bus/usb &> /dev/null
echo "checking internal hubs for auto suspend when the hub is idle"
gen_internal_usbhub_file
devfile=internal_usbhub
python check_usb_autosusp.py $devfile

if [ $? -ne 0 ]; then
	echo "FAIL"
	exit 1
fi
echo "PASS"
exit 0
