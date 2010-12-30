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


echo "Test case: med_tracker-live-update_video"

cp -rf /tmp/data/test_tracks/meego_v.ogv ~/Videos/

sleep 15

if tracker-search -v | grep "meego_v.ogv">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -v | grep "meego_v.ogv" | awk '\
  {\
  start = match($1,"meego_v.ogv");\
  if(start>0)\
  {printf("Success: Tracker can search out new added video meego_v.ogv\n");\
  exit 0;}\
  else\
  {printf("Fail: Tracker can Not search out new added video meego_v.ogv\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added video meego_v.ogv"
  exit 1
fi

rm -fr ~/Videos/meego_v.ogv

sleep 15

tracker-search -v | grep -c "meego_v.ogv" | awk '\
{\
if($1==0)\
{printf("Success: Video meego_v.ogv is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed video meego_v.ogv\n");\
exit 1;}\
}'
