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
# load sample, play sample, unload sample

# Modifications:
#          Modificator: shuang.wan@intel.com  Date: 2010-07-09
#          Content of Modification
#           1) Supplement the comments to the script file
#           2) Move the environment variables initlization to env.sh

. $(cd `dirname $0`;pwd)/env.sh

pactl upload-sample $DATA_PATH/test.wav test_sample > "$TEST_RESULT"

if [ $? -ne 0 ]; then
    echo "Failed to upload-sample"
    exit 1
fi
cat "$TEST_RESULT"

pactl play-sample test_sample > "$TEST_RESULT"

if [ $? -ne 0 ]; then
    echo "Failed to play sample"
    exit 1
fi
cat "$TEST_RESULT"

pactl remove-sample test_sample > "$TEST_RESULT"

if [ $? -ne 0 ]; then
    echo "Failed to unload modules"
    exit 1
fi
cat "$TEST_RESULT"

rm "$TEST_RESULT"
