#!/bin/bash
# 
# Copyright (C) 2007, Intel Corporation.
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
#       Chen, Hao  <hao.h.chen@intel.com>
#
# DESCR: alsa test - record and play audio file

cd `dirname $0`

arecord -D plughw:0,0  -d 5 -f S16 data/input.wav

sleep 2 

aplay -D plughw:0,0 -f dat data/input.wav
if [ $? -ne 0 ]; then
	echo "aplay wav file recorded fail"
        [ -f data/input.wav ] && rm -f data/input.wav
	exit 1
fi
[ -f data/input.wav ] && rm -f data/input.wav
exit 0
