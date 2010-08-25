#!/bin/sh
#DESCR: check GL feature GL_EXT_framebuffer_object
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
# check GL feature GL_EXT_framebuffer_object, through glxinfo
#
export DISPLAY=:0.0

FBO_STATUS=`glxinfo | grep "GL_EXT_framebuffer_object"`
if [ -n "$FBO_STATUS" ]; then
	echo "support GL_EXT_framebuffer_object, PASS!"
else
	echo "not support GL_EXT_framebuffer_object, FAIL"
	exit 1
fi

exit 0
	
