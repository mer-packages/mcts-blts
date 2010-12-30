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


echo "Test case: med_tracker-package_installation"

if rpm -qa | grep "tracker">/dev/null 2>&1
then
  echo "Not Null Value"
  rpm -qa | grep "tracker" | awk '\
  {\
  start = match($1,"tracker");\
  if(start>0)\
  {printf("Success: Tracker package is installed successfully\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker package\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker package"
  exit 1
fi

if rpm -ql tracker | grep "/usr/bin/">/dev/null 2>&1
then
  echo "Not Null Value"
  rpm -ql tracker | grep "/usr/bin/" | awk '\
  {\
  start = match($1,"/usr/bin/tracker");\
  if(start>0)\
  {printf("Success: Tracker is installed in /usr/bin\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker in /usr/bin\n");\
  exit 1;}\
}'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker in /usr/bin"
  exit 1
fi

if rpm -ql tracker | grep "/usr/lib/">/dev/null 2>&1
then
  echo "Not Null Value"
  rpm -ql tracker | grep "/usr/lib/" | awk '\
  {\
  start = match($1,"/usr/lib/");\
  if(start>0)\
  {printf("Success: Tracker is installed in /usr/lib\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker in /usr/lib\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker in /usr/lib"
  exit 1
fi

if rpm -ql tracker | grep "/usr/libexec/">/dev/null 2>&1
then
  echo "Not Null Value"
  rpm -ql tracker | grep "/usr/libexec/" | awk '\
  {\
  start = match($1,"/usr/libexec/tracker");\
  if(start>0)\
  {printf("Success: Tracker is installed in /usr/libexec\n");\
  }\
  else\
  {printf("Fail: can NOT find tracker in /usr/libexec\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker in /usr/libexec"
  exit 1
fi

if rpm -ql tracker | grep "/usr/share/">/dev/null 2>&1
then
  echo "Not Null Value"
  rpm -ql tracker | grep "/usr/share/" | awk '\
  {\
  start = match($1,"/usr/share/");\
  if(start>0)\
  {printf("Success: Tracker is installed in /usr/share\n");\
  exit 0;}\
  else\
  {printf("Fail: can NOT find tracker in /usr/share\n");\
  exit 1;}\
  }'

else
  echo "Null Value"
  echo "Fail: can NOT find tracker in /usr/share"
  exit 1
fi
