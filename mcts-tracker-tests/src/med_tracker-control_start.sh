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


echo "Test case: TRACKER-CONTROL_001"

tracker-control -s

if tracker-control | grep "tracker-miner-fs">/dev/null 2>&1
then
  echo "Not Null Value for grep output"
  tracker-control | grep "tracker-miner-fs" | awk '\
  {\
  start = match($0,"tracker-miner-fs");\
  if(start>0)\
  {printf("Success: Tracker daemon tracker-miner-fs is started\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker daemon tracker-miner-fs\n");\
  exit 1;}\
  }'

else
  echo "Null Value for grep output"
  echo "Fail: can NOT find tracker daemon tracker-miner-fs"
  exit 1
fi

if tracker-control | grep "tracker-store">/dev/null 2>&1
then
  echo "Not Null Value for grep output"
  tracker-control | grep "tracker-store" | awk '\
  {\
  start = match($0,"tracker-store");\
  if(start>0)\
  {printf("Success: Tracker daemon tracker-store is started\n");\
  exit 0;}\
  else\
  {printf("Fail: can NOT find tracker daemon tracker-store\n");\
  exit 1;}\
  }'

else
  echo "Null Value for grep output"
  echo "Fail: can NOT find tracker daemon tracker-store"
  exit 1
fi
