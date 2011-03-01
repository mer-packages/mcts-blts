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


cp -r /tmp/data/test_tracks/meego_i*.jpg ~/Pictures/

sleep 10

echo "Test case: med_tracker-tag_multiple_file"

tracker-tag -d test

sleep 5

tracker-tag -d test1

sleep 5

tracker-tag -d test2

sleep 5

tracker-tag -a test1 /home/$LOGNAME/Pictures/meego_i1.jpg

sleep 5

tracker-tag -a test2 /home/$LOGNAME/Pictures/meego_i2.jpg

sleep 5

if tracker-tag -t -s | grep "Tag">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s | grep -c "test" | awk '\
  {\
  if($2=2)\
  {printf("Success:Tracker can add multiple tags\n");\
  }\
  else\
  {printf("Fail: Tracker fail to add multiple tags\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker fail to add multiple tags"
  exit 1
fi

if tracker-tag -t -s | grep "test1">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s | grep "test1" | awk '\
  {\
  start = match($1,"test1");\
  if(start>0)\
  {printf("Success: Tracker can search out tag test1\n");\
  }\
  else\
  {printf("Fail: Tracker can Not search out tag test1\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out tag test1"
  exit 1
fi

if tracker-tag -t -s | grep "test2">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s | grep "test2" | awk '\
  {\
  start = match($1,"test2");\
  if(start>0)\
  {printf("Success: Tracker can search out tag test2\n");\
  }\
  else\
  {printf("Fail: Tracker can Not search out tag test2\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out tag test2"
  exit 1
fi

rm ~/Pictures/meego_i*.jpg
