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


echo "Test case: med_tracker-control_reset"

tracker-control -r

sleep 10

cd /home/$LOGNAME/.cache/tracker

ls | awk '\
{\
start = match($0,"meta.db");\
if(start>0)\
{printf("Fail: meta.db does NOT be removed\n");\
exit 1;}\
else\
{printf("Success: meta.db is removed successfully\n");\
}\
}'

ls | awk '\
{\
start = match($0,"contents.db");\
if(start>0)\
{printf("Fail: contents.db does NOT be removed\n");\
exit 1;}\
else\
{printf("Success: contents.db is removed successfully\n");\
}\
}'

ls | awk '\
{\
start = match($0,"fulltext.db");\
if(start>0)\
{printf("Fail: fulltext.db does NOT be removed\n");\
exit 1;}\
else\
{printf("Success: fulltext.db is removed successfully\n");\
}\
}'

tracker-control -k

sleep 15

tracker-control -s

sleep 20

exit 0
