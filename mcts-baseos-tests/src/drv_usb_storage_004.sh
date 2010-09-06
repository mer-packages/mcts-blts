#!/bin/sh
#DESCR: Delete partition 
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
. ./helper
device=`get_usb_storage_device`
if [ $? -ne 0 ]; then
	echo "ERR: can not get storage device file"
	exit 1
fi
echo "Find expected USB storage at $device"

if [ ! -e /dev/$device ]; then
	echo "ERR: Can not find device file /dev/$device"
	exit 1
fi
clean_partition $device
sleep 2
#dd if=/dev/zero of=/dev/$device bs=1024 count=1
/sbin/fdisk /dev/${device} <<EOF
n
p
1

+64M
n
p
2

+128M
w
EOF

/sbin/fdisk /dev/${device} <<EOF
d
2
w
EOF

sleep 5
if [ -e /dev/${device}2 ]; then
	echo "Delete partiton 2 fail"
	exit 1
fi
clean_partition $device
#dd if=/dev/zero of=/dev/$device bs=1024 count=1
echo "INFO: Succeed to delete partitions"
echo "PASS"
exit 0
	
