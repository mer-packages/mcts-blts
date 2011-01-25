#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2010 Intel Corporation.
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
#       Jeff Zheng <jeff.zheng@intel.com>
#
# Description: CM_Stress_BT
#

import sys
import os
import time
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
from common import *

if (len(sys.argv) != 2):
    print "Usage: %s <Device On/Off number> ... " % (sys.argv[0])
    EXIT(False)
round = int(sys.argv[1])

wifidev = WiFiDevice()
if not wifidev :
    print "No WiFi device, case fails"
    EXIT(False)
wifidev.Enable()
wifidev.Connect('Guest')
if not wifidev.IsConnected():
    print "WiFi device is not connected, case fails"
    EXIT(False)
dev = BTDevice()

count=1

while count <= round:
    print "At the %s round" % (count)
    print "Enable device"
    dev.Enable()
    time.sleep(10)
    dev.Connect()
    if dev.IsConnected():
        print "Bluetooth is connected"
    else:
        print "Bluetooth is not connected, test fails"
	EXIT(FALSE)
    time.sleep(40)
    print "Diable device"
    dev.Disable()
    time.sleep(10)
    if dev.IsConnected():
        print "Bluetooth is still connected, test fails"
	EXIT(FALSE)
    else:
        print "Bluetooth is not connected"
    time.sleep(40)
    count += 1
    
EXIT(True)
