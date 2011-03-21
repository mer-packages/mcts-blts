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


echo "Test case: med_tracker-live-update_folder"

cp -rf /tmp/data/test_tracks /home/$LOGNAME/Downloads/

sleep 10

if tracker-search -m | grep "meego_m1.ogg">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -m | grep "meego_m1.ogg" | awk '\
  {\
  start = match($0,"meego_m1.ogg");\
  if(start>0)\
  {printf("Success: Tracker can search out new added meego_m1.ogg\n");\
  }\
  else\
  {printf("Fail: Tracker can Not search out new added meego_m1.ogg\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added meego_m1.ogg"
  exit 1
fi

if tracker-search -v | grep "meego_v.ogv">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -v | grep "meego_v.ogv" | awk '\
  {\
  start = match($0,"meego_v.ogv");\
  if(start>0)\
  {printf("Success: Tracker can search out new added meego_v.ogv\n");\
  }\
  else\
  {printf("Fail: Tracker can Not search out new added meego_v.ogv\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added meego_v.ogv"
  exit 1
fi

if tracker-search -i | grep "meego_i1.jpg">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -i | grep "meego_i1.jpg" | awk '\
  {\
  start = match($0,"meego_i1.jpg");\
  if(start>0)\
  {printf("Success: Tracker can search out new added meego_i1.jpg\n");\
  exit 0;}\
  else\
  {printf("Fail: Tracker can Not search out new added meego_i1.jpg\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added meego_i1.jpg"
  exit 1
fi

sleep 10

if tracker-search -s | grep "test_tracks">/dev/null 2>&1
then
  echo "Not Null Value"
  tracker-search -s | grep "test_tracks" | awk '\
  {\
  start = match($0,"test_tracks");\
  if(start>0)\
  {printf("Success: Tracker can search out new added test_tracks\n");\
  exit 0;}\
  else\
  {printf("Fail: Tracker can Not search out new added test_tracks\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: Tracker can Not search out new added test_tracks"
  exit 1
fi

echo "Remove the folder test_tracks"

rm -rf ~/Downloads/test_tracks

sleep 15

tracker-search -i | grep -c "meego_i1.jpg" | awk '\
{\
if($1==0)\
{printf("Success: Image meego_i1.jpg is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed meego_i1.jpg\n");\
exit 1;}\
}'

tracker-search -m | grep -c "meego_m1.ogg" | awk '\
{\
if($1==0)\
{printf("Success: Music meego_m1.ogg is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed meego_m1.ogg\n");\
exit 1;}\
}'

tracker-search -v | grep -c "meego_v.ogv" | awk '\
{\
if($1==0)\
{printf("Success: Image meego_v.ogv is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed meego_v.ogv\n");\
exit 1;}\
}'

tracker-search -s | grep -c "test_tracks" | awk '\
{\
if($1==0)\
{printf("Success: Folder test_tracks is unable to display\n");\
exit 0;}\
else\
{printf("Fail: Tracker can Not update new removed test_tracks\n");\
exit 1;}\
}'

sleep 10
