#!/bin/bash
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
#       Chen, Hao  <hao.h.chen@intel.com>
#

#DESCR: test amixer

cd `dirname $0`

amixer -c0 
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 fail"
	exit 1
fi

amixer -c0 scontrols
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 scontrols fail"
	exit 1
fi

amixer -c0 scontents
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 scontents fail"
	exit 1
fi

amixer -c0 controls
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 controls fail"
	exit 1
fi

amixer -c0 contents
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 contents fail"
	exit 1
fi

control_name="Master Playback Volume"

amixer -c0 cget -c 0 iface=MIXER,name="$control_name"
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 cget fail"
	exit 1
fi

amixer -c0 cset -c 0 iface=MIXER,name="$control_name" 90%
if [ $? -ne 0 ]; then
	echo "Run amixer -c0 cset fail"
	exit 1
fi

echo "PASS"
exit 0
