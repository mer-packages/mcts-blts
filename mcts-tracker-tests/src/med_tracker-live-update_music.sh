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


echo "Test case: med_tracker-live-update_music"

cp /tmp/data/test_tracks/meego_m1.ogg ~/Music/

sleep 10

if tracker-search -m | grep "meego_m1.ogg">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -m | grep "meego_m1.ogg" | awk '\
  {\
  start = match($1,"meego_m1.ogg");\
  if(start>0)\
  {printf("Success: Tracker can search out new added music meego_m1.ogg\n");\
  exit 0;}\
  else\
  {printf("Fail: Tracker can Not search out new added music meego_m1.ogg\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added music meego_m1.ogg"
  exit 1
fi

rm -f ~/Music/meego_m1.ogg

sleep 15

tracker-search -m | grep -c "meego_m1.ogg" | awk '\
{\
if($1==0)\
{printf("Success: Music meego_m1.ogg is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed music meego_m1.ogg\n");\
exit 1;}\
}'

sleep 10
