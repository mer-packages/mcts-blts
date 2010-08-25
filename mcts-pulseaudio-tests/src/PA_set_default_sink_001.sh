#!/bin/bash
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
#       Chen, Hao  <hao.h.chen@intel.com>
#
# Set default sink

# Modifications:
#          Modificator: shuang.wan@intel.com  Date: 2010-07-09
#          Content of Modification
#           1) Supplement the comments to the script file
#           2) Move the environment variables initlization to env.sh

. $(cd `dirname $0`;pwd)/env.sh

$UTILS_PATH/pa_query_control -cm load module-null-sink sink_name=pa_test_sink >  "$TEST_RESULT"

if [ $? -ne 0 ]; then
    echo "Failed to load module"
    exit 1
fi
cat  "$TEST_RESULT"
index=`cat  "$TEST_RESULT" | awk -F= '{print $2}'`

$UTILS_PATH/pa_query_control -cd sink pa_test_sink >  "$TEST_RESULT"

if [ $? -ne 0 ]; then
    echo "Failed to set default sink"
    exit 1
fi
cat  "$TEST_RESULT"

$UTILS_PATH/pa_query_control -cm unload $index >  "$TEST_RESULT"    

if [ $? -ne 0 ]; then
    echo "Failed to unload module"
    exit 1
fi
cat  "$TEST_RESULT"

rm "$TEST_RESULT"
