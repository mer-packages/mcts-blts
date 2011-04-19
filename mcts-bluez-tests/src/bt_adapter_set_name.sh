#!/bin/sh
#DESCR: Set visible name for local bluetooth adapter. 
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
# Authors:
#       Zhang Jingke  <jingke.zhang@intel.com>
# Date Created: 2011/03/24
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#

# Enter the case folder
cd `dirname $0`
. ./data/bluetooth_env

ret=0

# Get current hci0's name. Need to restore it after testing
original_name=`${TEST_ADAPTER} name`

# Here, we won't test all kinds of legal or illegal name strings. 
# For further testing, you can customize string in ./data/bluetooth_env
custom=0
if [ ${TEST_NAME_STRING} != "default" ]; then
    test_names=${TEST_NAME_STRING}
    custom=1
else
    # If no custom string (TEST_NAME_STRING=default in the file), just choose these: 
    test_names=("netbook" "meego netbook" "**netbook")
fi

# Check name set for adapter
if [ $custom -eq 1 ]; then
    ${TEST_ADAPTER} name $test_name
    name=`${TEST_ADAPTER} name`
    if [ $name = "${test_names}" ]; then
        echo "[PASS] Custom name is set successfully!"
    else
        echo "[FAIL] Custom name setting fails."
        ret=1
    fi
else
    # test 3 names by default:
    i=0
    while [ $i -lt 3 ]; 
    do
        set_name=${test_names[i]}
        ${TEST_ADAPTER} name "$set_name"
        name=`${TEST_ADAPTER} name`
        if [ "$name" = "$set_name" ]; then
            echo "[PASS] Adapter name is set to \"${set_name}\""
        else
            echo "[FAIL] Adapter is $name, not set to ${set_name}"
            ret=1
        fi

         i=`expr $i + 1`
     done
fi

# Recover original name
${TEST_ADAPTER} name ${original_name}
exit $ret
