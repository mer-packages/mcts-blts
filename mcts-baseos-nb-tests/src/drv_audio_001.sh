#!/bin/sh
#DESCR: Check Intel Audio Card ID 
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
#       Wang,Jing  <jing.j.wang@intel.com>
#

# TV Ibex Peak HD audio :80863b56
# Netbook Diamondvilla 945GME HD audio :808627d8 

cd `dirname $0`

AUDIO_PCIIDS="808627d8"
for devid in $AUDIO_PCIIDS 
do
	cat /proc/bus/pci/devices | grep $devid
	if [ $? -eq 0 ]; then
		echo "Find Intel audio device by $devid in $PLATFORM platform"
		exit 0
	fi
done
echo "Not find Audio device"
/sbin/lspci -nn
exit 1
	


