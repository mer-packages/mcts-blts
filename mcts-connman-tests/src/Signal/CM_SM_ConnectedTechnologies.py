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
# Description: CM_SM_ConnectedTechnologies
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
import SignalManager
import common

def func():
    dev = common.WiFiGuestDevice()
    time.sleep(2)
    print 'Disabling...'
    dev.Disable()
    time.sleep(2)
    dev.Enable()


sig = SignalManager.sig
sig.property_name = 'ConnectedTechnologies'

trigger=SignalBase.Trigger(func)
trigger.start()
trigger.join()

sig.mainloop.run()
common.EXIT(sig.my_ret)

