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


cp -r /tmp/data/test_tracks/meego_i1.jpg ~/Pictures/

echo "Test case: med_tracker-tag_single_file"

tracker-tag -d test

sleep 5

tracker-tag -d test1

sleep 5

tracker-tag -d test2

sleep 5

tracker-tag -a test1 /home/$LOGNAME/Pictures/meego_i1.jpg

sleep 5

tracker-tag -a test2 /home/$LOGNAME/Pictures/meego_i1.jpg

sleep 5

if tracker-tag -t -s | grep "Tag">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-tag -t -s | grep -c "meego_i1" | awk '\
  {\
  if($1=2)\
	  {printf("Success:Tracker can add multiple tags on one file\n");\
  }\
  else\
	  {printf("Fail: Tracker fail to add multiple tags on one file\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker fail to add multiple tags on one file"
  exit 1
fi

#tracker-tag -t -s | grep "meego_i1" | awk '\
#{\
#for (x=0;;x++)\
#if($x<0){print NR; break}\
#if(NR==2)\
#{printf("Success: Tracker can search out video with multiple tags");\
#exit 0;}\
#else\
#{printf("Fail: Tracker can NOT search out video with multiple tags");\
#exit 1;}\
#}'

rm ~/Pictures/meego_i*.jpg
