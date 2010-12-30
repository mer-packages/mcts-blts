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


cp -r /tmp/data/test_tracks/meego_v.ogv ~/Videos/

echo "Test case: med_tracker-tag_add"

tracker-tag -d test

sleep 5

tracker-tag -d test1

sleep 5

tracker-tag -d test2

sleep 5

tracker-tag -a test /home/$LOGNAME/Videos/meego_v.ogv

sleep 5

if tracker-tag -t -s test | grep "Tag">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s test | grep -c "test" | awk '\
  {\
  if($1>0)\
  {printf("Success:Tracker can add tag\n");\
  }\
  else\
  {printf("Fail: Tracker fail to add tag\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker fail to add tag"
  exit 1
fi

if tracker-tag -t -s test | grep "meego_v.ogv">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s test | grep "meego_v.ogv" | awk '\
  {\
  start = match($1,"meego_v.ogv");\
  if(start>0)\
  {printf("Success: Tracker can search out tagged video meego_v\n");\
  exit 0;}\
  else\
  {printf("Fail: Tracker can Not find tagged video meego_v\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not find tagged video meego_v"
  exit 1
fi

tracker-tag -d test
rm ~/Videos/meego_v.ogv
