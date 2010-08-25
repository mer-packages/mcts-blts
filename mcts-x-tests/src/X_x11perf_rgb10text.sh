#!/bin/sh
#DESCR: x11perf -rgb10text
#TIMEOUT: 600
# Copyright (C) 2009, Intel Corporation.
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
#       Zheng,Kui  <kui.Zheng@intel.com>
#
#
# x11perf  for rgb10text test 
#
#
export DISPLAY=:0.0

x11perf -rgb10text
if [ $? -eq 0 ]; then
     echo "x11perf -- rgb10text, PASS!"
else
     echo "x11perf -- rgb10text, FAIL!"
     exit 1
fi

exit 0

