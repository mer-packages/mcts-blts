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
# Description: CM_SC_IPv4
#

import sys
import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
import time
import gobject
import dbus
import dbus.mainloop.glib
import SignalBase
import SignalConnman
import common
import offline

def func():
    dev = common.WiFiGuestDevice()
    svc = dev.GetService()
    svc.svc.SetProperty("IPv4.Configuration",
                         { "Method": "manual", "Address": "192.168.1.111"})
    time.sleep(1)
    svc.svc.SetProperty("IPv4.Configuration",
                         { "Method": "dhcp"})
    dev = common.WiFiGuestDevice()

sig = SignalConnman.sig
sig.property_name = 'IPv4'

trigger=SignalBase.Trigger(func)
trigger.start()
trigger.join()

sig.mainloop.run()
common.EXIT(sig.my_ret)
