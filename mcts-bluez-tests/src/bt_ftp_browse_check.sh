#!/bin/sh
#DESCR: Check FTP (File transfer profile) folder navigation from FTP client.
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
#       Zhang Jingke  <jingke.zhang@intel.com>
# Date Created: 2010/01/18
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#

# Enter the case folder
cd `dirname $0`
. ./data/bluetooth_env

adaptors=`hciconfig  | grep "BD Address" | awk '{print $3}'`
hci0=`echo "$adaptors" | sed -n '1p'`

if [ -z $hci0 ]; then
    echo "No adaptor found!"
    exit 1
fi

# Make a pairing with the server
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

# Get the server info file.
export DISPLAY=:0.0

${FTP_CLIENT} -d $SERV_BD_ADDR -g "${DATA_DIR}/server_info"

if [ ! -f ${DATA_DIR}/server_info ]; then
    echo "Server info not exist!"
    exit 1
fi

# Select a sub directory in the root directory randomly
file_count=`grep "file_count.*subdir" ${DATA_DIR}/server_info -c`
let "item_num = ($RANDOM % $file_count) + 1"
if [ $item_num -eq 0 ]; then
    item_num=1
fi

# Compose the sub folder name we want to browse
subfoldername="subdir$item_num"

# Change the current directory to the sub directory just selected and browse the content of it
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subfoldername -l >tmp_folder_nav.log

# Check file count property of the sub directory
row_count=`wc -l ./tmp_folder_nav.log | awk '{print $1}'`
expected_count=`grep "file_count.*subdir$item_num" ${DATA_DIR}/server_info | awk '{print $3}'`

# Delete the temporary files
rm -rf ./tmp_folder_nav.log

# Compare the server info file and folder count we get
if [ $row_count -ne $expected_count ]; then
    echo "The file count in folder:$subfoldername is not equal server info file!"
    echo "Check folder navigation and browse: Failed"
    exit 1
fi

# Do further check like comparing file list in sub directory with server info file
# But I think file count check is good enough.

echo "Check folder navigation and browse: Successful"
exit 0
