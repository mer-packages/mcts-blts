#!/bin/bash
#DESCR: Test intel high definition audio codec power management feature
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
if [ ! -d /sys/module/snd_hda_intel ]; then
	echo "can't find snd_hda_intel. skip"
	exit 0
fi


POWER_SAVE=`cat /sys/module/snd_hda_intel/parameters/power_save`

echo power_save value is $POWER_SAVE

[ $POWER_SAVE -eq 0 ] && { echo "error!!! power_save is zero"; exit 1; }

POWER_SAVE_CONTROLLER=`cat /sys/module/snd_hda_intel/parameters/power_save_controller`

echo power_saver_controller value is $POWER_SAVE_CONTROLLER

[ "$POWER_SAVE_CONTROLLER" != "Y" ] && { echo "error!!! power_save_controller is not Y"; exit 1; }

exit 0
