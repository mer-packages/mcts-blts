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
# Description: Signal functions used when property in Manager changes
#

import time
import gobject

import dbus
import dbus.mainloop.glib
from SignalBase import *


def property_changed(name, value):
    func(name, value)


class SignalManager(SignalBase):

    def __init__(self):
        SignalBase.__init__(self)

    def Add_Signal_Receiver(self, bus):
        bus.add_signal_receiver(property_changed,
                                bus_name='net.connman',
                                signal_name='PropertyChanged',
                                dbus_interface='net.connman.Manager'
                                )

    def property_changed(self, name, value):
        SignalBase.property_changed(self, name, value)


sig = SignalManager()
from common import *


def signal_manager(name, value):
    sig.property_changed(name, value)


func = signal_manager
