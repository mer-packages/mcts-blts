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
cd `dirname $0`
date0=`date`
date -s "09/10/2004 07:08:11"
/sbin/hwclock --systohc
/sbin/hwclock | grep "07:08:"
if [ $? != 0 ]; then
        echo "Failed to sync sys datetime to hwclock"
        exit 1
fi
date -s "$date0"
exit 0
