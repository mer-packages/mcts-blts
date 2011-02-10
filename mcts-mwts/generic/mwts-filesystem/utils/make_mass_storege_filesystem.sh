#!/bin/sh
#
# make_mass_storage_filesystem.sh
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
#filesystem size in Megabytes, so 1024 = 1 GB
SIZE=2048
# NOTE could use bc and allow filsystem size as parameter (CYLINDERS=MB * 16)
# but not bc present in tiny MeeGo images
CYLINDERS=32768

# 2 GB filesysten
echo
echo "Making 2 GB file for filesystem, please wait"
mkdir -p $DIRECTORY
dd bs=1M count=$SIZE if=/dev/zero of=$FILE

# make partion on a file

echo "Making vfat 32 filesystem in 2 GB file, please wait"
{
echo x
echo s
echo 8
echo h
echo 16
echo c
echo $CYLINDERS
echo r
echo n
echo p
echo 1
echo 
echo 
echo t
echo b
echo p
echo w
}| fdisk $FILE

# set loop device

echo "Setting loop device"
losetup -o 4096 /dev/loop0 $FILE

# Format as vfat

echo "Formatting as vfat32"
mkdosfs -n "MeeGo" /dev/loop0

# check by mounting/unmounting

echo "Mount"
mkdir -p /mnt/loop
mount -t vfat /dev/loop0 /mnt/loop

echo "Disk Free"
df -h /dev/loop0

echo "Umount"
umount /dev/loop0

# set namually top mass storage mode
# first try unload g_ether
# then load g_file_storage file
# so usb0 networking will be closed
# But as tested scrip can be run in usb-connection,
# even it seems to hang.

echo "Ready to set mass storage mode"
echo "rmmod g_ether"
echo "So if you are in usb-connection, this is last message your see"
echo "But in tests script run to the end,"
echo "so you should see mounted mass storage file system soon."

rmmod g_ether
echo "modprobe g_file_storage file="$FILE

modprobe g_file_storage file=$FILE

echo "Ready and DONE"

