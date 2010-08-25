#!/bin/sh
#DESCR: Run glxgears sample application
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
#       Zheng Kui <kui.zheng@intel.com>
#
#
# check whether glxgears can run
# if glxgears not installed, skip
#
#
export DISPLAY=:0.0

GLXGEARS=`which glxgears`
if [ -z "$GLXGEARS" ]; then
    echo " No glxgears found, skip..."
else
    echo "start glxgears: $GLXGEARS............"
    $GLXGEARS 2>&1 &
    sleep 6
    XPID=`pgrep glxgears`
    if [ ! -z "$XPID" ]; then
        kill -9 $XPID >/dev/null 2>&1
        echo "glxgears can run, PASS!"
    else
        echo "glxgears fail to run, FAIL!"
	exit 1
    fi
fi

echo

