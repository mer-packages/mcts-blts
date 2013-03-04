#!/bin/bash
#
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
#
# Author: 
#        Wei, Zhang <wei.z.zhang@intel.com>
#

# sanity parameter check
if ([ x"$1" != x"mem" ] && [ x"$1" != x"cpu" ]) || [ $# -lt 2 ]; then
        echo "$0 [mem|cpu] <Percentage Threshold>"
        exit 0
fi

# CPU or Memory
TYPE=$1

# failure threshold per each process, 0~100
THRESHOLD=$2

# tried times
TIMES=10

# sleep time per try (ms)
SLEEP=1

for((i=0; i<$TIMES; i++));do

    top=`ps -A -o p${TYPE},comm | sort -g -k 1 -r | head -1`
    top_percent=`echo $top | awk '{print $1}'`
    top_command=`echo $top | awk '{print $2}'`

    echo -ne "\r$(($i+1))/$TIMES... $top"

    if [ "$(echo "$top_percent" "$THRESHOLD" | awk '{if($1 < $2) print "ok"}')" != "ok" ]
    then
        echo -e "\nFAIL: Program <$top_command> takes ${top_percent}% $TYPE which is exceeded to threshold ${THRESHOLD}%"
        exit 1
    fi

    sleep $SLEEP

done

echo -e "\nPASS: No program takes more than ${THRESHOLD}% $TYPE"
