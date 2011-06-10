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


echo "Test case: med_tracker-search_image"

cp -rf /tmp/data/test_tracks/meego_i*.jpg ~/Pictures/

sleep 10

if tracker-search -i | grep "file">/dev/null 2>&1
then
  echo "Not Null Value for grep output"
  tracker-search -i | grep -c "file" | awk '\
  {if($1>0)\
  {printf("Success: Tracker find %s image\n",$1);\
  exit 0;}\
  else\
  {printf("Fail: Tracker can not find any image\n");\
  exit 1;}\
  }' 

else
  echo "Null Value for grep output"
  echo "Fail: Tracker can not find any image"
  exit 1
fi

rm -rf ~/Pictures/meego_i*.jpg
