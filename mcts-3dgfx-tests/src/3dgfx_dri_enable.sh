#!/bin/bash

# Description - Check PulseAudio daemon
# Copyright (C) 2010 Intel Corporation
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# Authors:
#         Zheng Kui<kui.zheng@intel.com>
#
#
# check whether DRI is enabled, through glxinfo
# if glxinfo is not installed, skip this test case
# make sure the X has started before this test
#
#
export DISPLAY=:0.0

GLXINFO=`which glxinfo`
if [ -z $GLXINFO ]; then
    echo "WARNING: No glxinfo found, ignore DRI cheching!"
else
    $GLXINFO >temp.glxinfo.$$ 2>&1
    if [ $? -ne 0 ]; then
        echo "glxinfo exit exception, FAIL!"
    	exit 1
    # glxinfo output will include line like:
    #   direct rendering: YES  or
    #   direct rendering: NO
    else
        DRI_STATUS=$(grep "direct rendering" temp.glxinfo.$$ | awk '{print $3}')
        if [ Y"$DRI_STATUS" = "YYes" ] ; then
            echo "direct rendering is enabled! PASS"
            grep -i error temp.glxinfo.$$
            if [ $? -eq 0 ]; then
                echo "glxinfo run with ERROR!"
		exit 1
            fi
        else
            	echo "direct rendering is disabled! FAIL"
	    	exit 1
        fi
    fi
    rm -f temp.glxinfo.$$
fi

echo

