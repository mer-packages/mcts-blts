#!/bin/sh
#DESCR: Check FTP (File Transfer Profile) file get
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
# Date Created: 2010/01/12
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

# select a random file in the root directory
file_count=`grep "file_count.*root" ${DATA_DIR}/server_info | awk '{print $3}'`
let "item_num = ($RANDOM % $file_count) + 1"
if [ $item_num -eq 0 ]; then
    item_num=1
fi

# Treat the files in the root directory and file in the subdirectory differently.

is_contain_dir=`grep "file_info.*root" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $2}' | grep "\/"`
if [ "$is_contain_dir" != "" ]; then
    get_dirname=`grep "file_info.*root" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $2}' | sed 's/\(.*\)\/\(.*\)/\1/g'`
fi

get_filename=`grep "file_info.*root" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $2}' | sed 's/\(.*\)\/\(.*\)/\2/g'`
md5string=`grep "file_info.*root" ${DATA_DIR}/server_info | sed -n "$item_num p" | awk '{print $3}'`

if [ -z $get_dirname ]; then
    ${FTP_CLIENT} -d $SERV_BD_ADDR -g $get_filename
else
    ${FTP_CLIENT} -d $SERV_BD_ADDR -c $get_dirname -g $get_filename
fi

#rm -rf ./server_info

if [ ! -f $get_filename ]; then
    echo "Test file not exist!"
    exit 1
fi

md5string_local=`md5sum $get_filename | awk '{print $1}'`

if [ $md5string != $md5string_local ]; then
    echo "md5 string mismatch, there should be some problem in file transferring!"
    echo "Check FTP get file: Failed"
    rm -rf "$get_filename"
    exit 1
fi

echo "md5 strings are same, file get checking successfully!"
echo "Check FTP get file: Successful"

rm -rf "$get_filename"
exit 0
