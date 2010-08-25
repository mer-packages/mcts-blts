#!/bin/sh
#DESCR: check GL feature GLX_EXT_texture_from_pixmap
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
#        Zheng,Kui  <kui.zheng@intel.com>
#
#
# check GL feature GLX_EXT_texture_from_pixmap., through glxinfo
#
export DISPLAY=:0.0

TFP_STATUS=`glxinfo | grep "GLX_EXT_texture_from_pixmap"`
if [ -n "$TFP_STATUS" ]; then
	echo "support GLX_EXT_texture_from_pixmap, PASS!"
else
	echo "not support GLX_EXT_texture_from_pixmap, FAIL"
	exit 1
fi

exit 0
	
