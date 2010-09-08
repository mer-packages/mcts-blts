#!/bin/sh
#DESCR: Test proc ACPI adpater feature
# Copyright (C) 2009 Intel Corporation.
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
#
# Authors:
#       Gong, Zhipeng <zhipeng.gong@intel.com>

cd `dirname $0`
. ./functions

#Check ACPI option in configuration
grep_bootconfig CONFIG_ACPI_AC

#Make sure AC is on-line
grep on-line /proc/acpi/ac_adapter/*/state
[ $? -ne 0 ] && { echo "ac_adapter is not on-line "; exit 1; }
exit 0
