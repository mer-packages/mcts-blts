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
#

BASE_DIR=`dirname $0`
cd ${BASE_DIR}

./test-simple-voicecall dial 4321&

sleep 5

./test-simple-voicecall dial 199&

sleep 3

./test-simple-voicecall hold

sleep 5

./test-multi-voicecall holdandanswer

sleep 2

./test-multi-voicecall getproperties |grep "4321\|1234567"

if [ $? -ne 0 ]; then
	echo "error when hold and answer call"
	exit 1
else
	./test-simple-voicecall hangupall
	exit 0
fi
