#!/bin/sh
#DESCR: Test cpu tickless feature works when system is busy
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
CONFIG_SUFFIX=`uname -r`

if [ ! -f /boot/config-$CONFIG_SUFFIX ]; then
	echo "mount /dev/sda1 "
	mount /dev/sda1 /boot
fi

if [ ! -f /boot/config-$CONFIG_SUFFIX ]; then
	echo "can't find boot config"
	config_hz=1000
else
	config_hz=`grep CONFIG_HZ= /boot/config-$CONFIG_SUFFIX | awk -F= '{print $2}'`
fi

echo $config_hz
./check_tickless_busy $config_hz      
exit $?
