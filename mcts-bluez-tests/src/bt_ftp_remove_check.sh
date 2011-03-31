#!/bin/sh
#DESCR: Check FTP (File Transfer Profile) folder manipulation includs create, remove
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

# Make a device pairing to FTP server
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

export DISPLAY=:0.0
tmp_folder_name="tmp_folder_name1"
tmp_file_name="tmp_ftp"

# Create a folder on server's root directory
${FTP_CLIENT} -d $SERV_BD_ADDR -C $tmp_folder_name

# List the server's root directory and check if 
# the new created sub-folder listed in the folder browse output
${FTP_CLIENT} -d $SERV_BD_ADDR -l > tmp_ftp_folder.log
created=`grep "$tmp_folder_name" ./tmp_ftp_folder.log -c`
if [ $created -le 0 ]; then
    echo "The folder $tmp_folder_name is not created!"
    echo "Check FTP folder create: Failed"
    rm -rf ./tmp_ftp_folder.log
    exit 1
fi

# Create a temporary file
echo "This is a temporary file used to test FTP file remove under new folder."> $tmp_file_name

# Upload this file to FTP server
${FTP_CLIENT} -d $SERV_BD_ADDR -c $tmp_folder_name -p `pwd`/$tmp_file_name

# List the root folder again and check if the folder really got removed
${FTP_CLIENT} -d $SERV_BD_ADDR -c $tmp_folder_name -l > tmp_ftp_folder.log
exist=`grep "$tmp_file_name" ./tmp_ftp_folder.log -c`
if [ $exist -ne 1 ]; then
    echo "Failed to create the file $tmp_file_name under folder $tmp_folder_name!"
    rm -rf ./tmp_ftp_folder.log
    exit 1
fi

# Remove the folder we just created
${FTP_CLIENT} -d $SERV_BD_ADDR -c $tmp_folder_name -r $tmp_file_name

# List the root folder again and check if the folder really got removed
${FTP_CLIENT} -d $SERV_BD_ADDR -c $tmp_folder_name -l > tmp_ftp_folder.log
exist=`grep "$tmp_file_name" ./tmp_ftp_folder.log -c`
if [ $exist -gt 0 ]; then
    echo "Failed to delete the file $tmp_file_name under folder $tmp_folder_name!"
    echo "Check FTP new folder file remove: Failed"
    rm -rf ./tmp_ftp_folder.log
    exit 1
fi

rm -rf ./tmp_ftp_folder.log
rm -f $tmp_file_name

echo "Check FTP new folder file create & remove: successfully"
exit 0
