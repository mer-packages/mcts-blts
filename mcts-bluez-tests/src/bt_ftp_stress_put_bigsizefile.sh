#!/bin/sh
#DESCR: Check to use FTP to transfer a big file (about 10MB) to OBEXD Server.
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

adaptors=`hciconfig | grep "BD Address" | awk '{print $3}'`
hci0=`echo "$adaptors" | sed -n '1p'`

if [ -z $hci0 ]; then
    echo "No adaptor found!"
    exit 1
fi

# If both IUTS' adaptor support simple pairing feature, then the pairing agent is needed
# to handle the pairing request when making a l2cap connection.
ret=0
export DISPLAY=:0.0
${DATA_DIR}/auto_accept_agent hci0 $SERV_BD_ADDR
sleep 2

# make a big file in local directory, size is 10MB, name is "10M_file"
big_file="10M_file"
cat ${TEST_TEXT_FILE} ${TEST_TEXT_FILE} > ./1M_file
cat ./1M_file ./1M_file ./1M_file ./1M_file ./1M_file > ./5M_file
cat ./5M_file ./5M_file > $big_file
rm -f ./1M_file ./5M_file

# Delete the same name file on Server
${FTP_CLIENT} -d $SERV_BD_ADDR -g "${DATA_DIR}/server_info"
subdir="subdir1"
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -r ${big_file}

# Upload this file to FTP server
time1=`date +%s`
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -p `pwd`/${big_file}
time2=`date +%s`
duration=`expr $time2 - $time1`

# Delete the big file
 
# search this file on Server
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -l | grep "${big_file}" > /dev/null
if [ $? -ne 0 ]; then
    echo "[FAIL] Failed to upload big 10M file to FTP server!"
    ret=1
else 
    echo "[PASS] succeed in uploading a 10MB file to server, using ${duration} seconds."
fi

# remove the file we just pushed
${FTP_CLIENT} -d $SERV_BD_ADDR -c $subdir -r ${big_file} 
rm -f ${big_file}

exit ${ret}
