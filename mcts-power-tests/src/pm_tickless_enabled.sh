#!/bin/sh
#DESCR: Test cpu tickless feature is enabled
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


if [ ! -f /boot/config-`uname -r` ]; then
	echo "mount /dev/sda1 "
	mount /dev/sda1 /boot
fi

if [ ! -f /boot/config-`uname -r` ]; then
	echo "can't find boot config"
	exit 2
fi

cat /boot/config-`uname -r` | grep "CONFIG_NO_HZ=y"
if [ $? -ne 0 ]; then
	echo "Tickless kernel configuration is unset"
	exit 1
fi
echo "Tickless kernel configuration is set"
exit 0
