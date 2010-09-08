#!/bin/sh
#DESCR: Show disk partition of SDMMC Card reader 
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
sddev=`find_sd_device`
if [ $? -ne 0 ]; then
	echo "Not find SD device"
	exit 1
fi

/sbin/fdisk -l /dev/$sddev|grep "Disk identifier:"
if [ $? -ne 0 ]; then
	echo "fail to show fdisk info of /dev/$sddev"
	exit 1
fi
exit 0

	


