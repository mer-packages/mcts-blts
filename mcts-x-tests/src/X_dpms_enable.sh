#!/bin/sh
#DESCR: check if DPMS is enabled, through /tmp/Xorg.0.log
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
#        Zheng,Kui  <kui.zheng@intel.com>
#
#
# check if DPMS is enabled, through /tmp/Xorg.0.log
#

XPID=`pgrep Xorg`
if [ -n "$XPID" ]; then
    echo "Checking Xorg.0.log"
else
    echo "Xorg is not started"
    exit 1
fi
XUSRNAME=`ps -C Xorg -o user=`
LOGDIR1=/var/log/Xorg.0.log
LOGDIR2=/tmp/Xorg.0.$XUSRNAME.log

if [ -s "$LOGDIR2" ]; then
    LOG=$LOGDIR2
elif [ -s "$LOGDIR1" ]; then
    LOG=$LOGDIR1
else
    echo "can't find Xorg.0.log"
    exit 1
fi
DPMS_STATUS=`cat $LOG | grep "DPMS.*enabled"`
if [ -n "$DPMS_STATUS" ]; then
    echo "DPMS is Enabled, PASS!"
else
    echo "DPMS is not Enabled, FAIL"
    exit 1
fi
exit 0
	
