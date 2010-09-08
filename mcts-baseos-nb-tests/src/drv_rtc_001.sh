#!/bin/bash
#DESCR: Check RTC function 
# Copyright (C) 2009, Intel Corporation.
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
#       Gong,Zhipeng  <zhipeng.gong@intel.com>
#

set -x
date0=`date`

/sbin/hwclock --set --date="11/20/2003 12:48:00"
/sbin/hwclock | grep "12:48:"
if [ $? != 0 ]; then
	echo "Failed to set hwclock"
	exit 1
fi

/sbin/hwclock --hctosys
date | grep "Nov 20 12:48:.. ... 2003"

if [ $? != 0 ]; then
        echo "Failed to sync hwclock to sys datetime"
        exit 1
fi
date -s "$date0"
exit 0
