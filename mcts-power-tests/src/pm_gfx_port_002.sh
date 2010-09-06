#!/bin/sh
#DESCR: Test if unused port should be disconnected
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


export DISPLAY=:0.0
s=`xrandr | grep connected -w | wc | awk '{print $1}'`
[ $s -ne 1 ] && { echo "Fail to run xrandr "; exit 1; }

s=`xrandr | grep connected -w | grep LVDS`
[ $? -ne 0 ] && { echo "Fail to grep LVDS "; exit 1; }

exit 0
