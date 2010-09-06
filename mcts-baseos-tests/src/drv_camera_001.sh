#!/bin/sh
#DESCR: Check video device 
# Copyright (C) 2008, Intel Corporation.
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
#       Chen,Hao  <hao.h.chen@intel.com>
#

#camera device
cd `dirname $0`
DEV_VIDEO=/dev/video0

#check video device
if [ ! -e $DEV_VIDEO ]; then
	echo "USB Camera not found"
	exit 1
fi


	


