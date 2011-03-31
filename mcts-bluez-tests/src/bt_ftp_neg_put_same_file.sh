#!/bin/sh
#DESCR: Check FTP (File Transfer Profile) put the same file for three times.
# Copyright (C) 2010 Intel Corporation
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
# Date Created: 2011/03/24
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

# If both IUTS' adaptor support simple pairing feature, then the pairing agent is needed
# to handle the pairing request when making a l2cap connection.

${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

# Get the server info file.

export DISPLAY=:0.0

${FTP_CLIENT} -d $SERV_BD_ADDR -g "${DATA_DIR}/server_info"

if [ ! -f ${DATA_DIR}/server_info ]; then
    echo "Server info not exist!"
    exit 1
fi

# select a random sub-directory
subdir_count=`grep "file_count.*subdir" ${DATA_DIR}/server_info -c`

let "item_num = ($RANDOM % $subdir_count) + 1"
if [ $item_num -eq 0 ]; then
    item_num=1
fi

# Do file put for 3 times
i=0

while [ $i -lt 3 ];
do 
    i=`expr $i + 1`
    subdir=`grep "file_count.*subdir" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $2}'`
    oldfilecount=`grep "file_count.*subdir" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $3}'`

    # Create a temporary file
    echo "This is a temporary file used to test FTP file put function.">tmp_ftp_put.log

    # Upload this file to FTP server
    ${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -p `pwd`/tmp_ftp_put.log

    rm -rf tmp_ftp_put.log
 
    # Try to get the file count of the folder we just pushed.
    newfilecount=`${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -l | wc -l`

    if [ $newfilecount -ne `expr $oldfilecount + 1` ]; then
        echo "Failed to upload file tmp_ftp_put.log to FTP server!"
        echo "Check FTP file put: Failed"
        exit 1
    else
        echo "[PASS] Send file to server, for the $i time." 
    fi
done

# remove the file we just pushed
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -r tmp_ftp_put.log

echo "[PASS] Check FTP same file put: successfully"
exit 0
