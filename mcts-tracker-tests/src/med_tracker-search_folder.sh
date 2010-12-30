#!/bin/sh
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
#       Huang Rui  <rui.r.huang@intel.com>
# Date Created: 2010/12/25
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#


echo "Test case: med_tracker-search_folder"

cp -rf /tmp/data/test_tracks ~/

sleep 15

if tracker-search -s | grep "test_tracks">/dev/null 2>&1
then
  echo "Not Null Value for grep output"
  tracker-search -s test_tracks | grep -c "test_tracks" | awk '\
  {if($1=1)\
  {print("Success: Tracker find test_tracks folder\n");\
  exit 0;}\
  else\
  {print("Fail: Tracker can not find test_tracks folder\n");\
  exit 1;}\
  }' 

else
  echo "Null Value for grep output"
  echo "Fail: Tracker can not find test_tracks folder"
  exit 1
fi

rm -rf ~/test_tracks
