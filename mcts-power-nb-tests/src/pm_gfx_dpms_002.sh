#!/bin/sh
#DESCR: Test if graphics DPMS feature works
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


err_exit()
{
	echo $@
	exit 1
}

export DISPLAY=:0.0
xset -q | grep "DPMS is Enabled"
[ $? -ne 0 ] && err_exit "DPMS is disabled or xset does not support DPMS"

xset dpms 10 20 30
xset -q | grep "Off: 30"
[ $? -ne 0 ] && err_exit "Fail to set DPMS off time"

sleep 30
xset -q | grep "Monitor is Off"
[ $? -ne 0 ] && err_exit "Monitor does not shut down"

xset dpms 0 0 0

xset dpms force on

exit 0
