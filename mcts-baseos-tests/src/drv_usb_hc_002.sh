#!/bin/sh
#DESCR: Check Intel EHCI Controller 
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
found="no"
for devicecode in "808627c8" "80862658"; do
	cat /proc/bus/pci/devices |grep "$devicecode"
	if [ $? -eq 0 ]; then
		found="yes"
		break
	fi
done
if [ $found == "no" ]; then
	echo "Not find Intel UHCI with code $devicecode"
	exit 1
fi
echo "INFO: find UHCI with PCI ID $devicecode"
exit 0

	


