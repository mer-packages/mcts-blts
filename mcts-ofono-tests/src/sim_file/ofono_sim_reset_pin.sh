#!/bin/sh
# Copyright (C) 2008-2010, Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.  
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307 USA.
#
# Authors:
#       Li,Zhigang  <zhigang.li@intel.com>
#
 
BASE_DIR=`dirname $0`
cd ${BASE_DIR}

export DISPLAY=:0.0

pathto=/usr/share/phonesim/default.xml
pkill "^phonesim"
grep -n "PINNAME" $pathto | head -n 1 |awk -F: '{print $1}' \
| xargs -I{} sed -i {}s/"READY"/"SIM PUK"/ $pathto

sleep 2

cmd="phonesim -p 12345 $pathto"

ionice -n7 $cmd &

sleep 3

./test-modem poweron

sleep 9

./test-sim resetpin 13243546 2468

sleep 3

./test-modem online

if [ $? -ne 0 ]; then
	#this only for test automation
	grep -n "PINNAME" $pathto | head -n 1 |awk -F: '{print $1}' \
	| xargs -I{} sed -i {}s/"SIM PUK"/"READY"/ $pathto
	echo "error in inputing puk code"
	exit 1
else
	#this only for test automation
	grep -n "PINNAME" $pathto | head -n 1 |awk -F: '{print $1}' \
	| xargs -I{} sed -i {}s/"SIM PUK"/"READY"/ $pathto
	exit 0
fi
