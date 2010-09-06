#!/bin/sh
#DESCR: Test cpu qos feature by cpuidle latency
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
# Authors:
#       Gong, Zhipeng <zhipeng.gong@intel.com>

cd `dirname $0`
if [ ! -d /sys/devices/system/cpu/cpu0/cpuidle ]; then
	echo "fail to find cpuidle directory"
	exit 2
fi

count=0
for file in /sys/devices/system/cpu/cpu0/cpuidle/*/latency; do	
	tmp=`cat $file`
	if [ $tmp -ne 0 ];then 
		count=`expr $count + 1`
		latency[$count]=$tmp
	fi
done

echo ${latency[*]}

for i in ${latency[*]}; do
	sleep 1
	echo "Check latency " $i 
	./check_pm_qos $i 3
	if [ $? -eq 0 ]; then
		echo "Pass check"		
	else
		echo "Fail check"
		exit 1
	fi
done

exit 0
