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


echo "Test case: med_tracker-package_process"

if ps -ef | grep "tracker-miner-fs">/dev/null 2>&1
then
  echo "Not Null Value"
  ps -ef | grep "tracker-miner-fs" | awk '\
  {\
  start = match($0,"tracker-miner-fs");\
  if(start>0)\
  {printf("Success: tracker-miner-fs daemon is started\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker-miner-fs daemon\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker-miner-fs daemon"
  exit 1
fi

if ps -ef | grep "tracker-store">/dev/null 2>&1
then
  echo "Not Null Value"
  ps -ef | grep "tracker-store" | awk '\
  {\
  start = match($0,"tracker-store");\
  if(start>0)\
  {printf("Success: tracker-store daemon is started\n");\
  exit 0;}\
  else\
  {printf("Fail: can NOT find tracker-store daemon\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker-store daemon"
  exit 1
fi

