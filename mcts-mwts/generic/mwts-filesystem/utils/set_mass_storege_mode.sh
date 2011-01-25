#!/bin/sh
#
# set_mass_storage_mode.sh
#
# This file is part of MCTS 
# 
# Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). 
# 
# Contact: Tommi Toropainen; tommi.toropainen@nokia.com; 
# 
# This library is free software; you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public License 
# version 2.1 as published by the Free Software Foundation. 
# 
# This library is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# Lesser General Public License for more details. 
# 
# You should have received a copy of the GNU Lesser General Public 
# License along with this library; if not, write to the Free Software 
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
# 02110-1301 USA 
# 
#


# make first file for mass-storage filesystem

# valiables
DIRECTORY="/root/data"
FILE=$DIRECTORY"/backing_file"

echo "rmmod g_ether"
echo "So if you are in usb-connection, this is last message your see"
echo "But in tests script run to the end,"
echo "so you should see mounted mass storage file system soon."

rmmod g_ether
echo "modprobe g_file_storage file="$FILE

modprobe g_file_storage file=$FILE

echo "Ready and DONE"

