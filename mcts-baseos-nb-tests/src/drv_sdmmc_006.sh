#!/bin/sh
#DESCR: copy file to sd card
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
#TIMEOUT: 600
cd `dirname $0`
. ./helper

device=`find_sd_device`
if [ $? -ne 0 ]; then
	echo "Fail to find SD device"
	exit 1
fi

if [ ! -e /dev/$device ]; then
	echo "Can not find device /dev/$device"
	exit 1
fi
par1=`get_sd_par $device 1`
umount /dev/$par1
dd if=/dev/zero of=/dev/$device bs=1024 count=1
sync
sleep 3
/sbin/fdisk /dev/${device} <<EOF
n
p
1

+128M
w
EOF
sleep 3
 
if [ ! -e /dev/$par1 ]; then
	echo "Create partiton 1 fail"
	exit 1
fi
umount /dev/$par1
/sbin/mkfs.ext3 /dev/$par1
if [ $? -ne 0 ]; then
	echo "Fail to setup filesystem on SD card"
	exit 1
fi

[ -d tmpmnt ] || mkdir tmpmnt
umount tmpmnt
mount /dev/$par1 tmpmnt
if [ $? -ne 0 ]; then
        echo "Fail to mount filesystem on SD card"
        exit 1
fi
rm -fr tmpmnt/*

mkdir -p tmpmnt/tmpdata

dd if=/dev/urandom of=data1 bs=1k count=10000
dd if=/dev/urandom of=data2 bs=1k count=20000
dd if=/dev/urandom of=data3 bs=1k count=30000

cp -a data* tmpmnt/tmpdata
umount tmpmnt
sleep 1 
mount /dev/$par1 tmpmnt
if [ $? -ne 0 ]; then
        echo "Fail to mount filesystem on SD card"
        exit 1
fi


for i in 'data1' 'data2' 'data3'; do
	diff $i tmpmnt/tmpdata/$i
	if [ $? -ne 0 ]; then
		echo "cp $i fail"
		umount tmpmnt
		exit 1

	fi
done
umount tmpmnt
rm -rf data1 data2 data3
sleep 8
umount /dev/$par1
dd if=/dev/zero of=/dev/$device bs=1024 count=1
echo "PASS"
exit 0
