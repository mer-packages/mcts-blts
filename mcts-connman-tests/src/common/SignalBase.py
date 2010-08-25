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
# Description: Functions used all signal test cases
#

import time
import gobject

import dbus
import dbus.mainloop.glib


def f(
    name,
    value,
    path,
    interface,
    ):

    print 'In f'


func = f


class SignalBase:

    def __init__(self):
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        bus = dbus.SystemBus()
        print 'Add signal receiver'
        self.Add_Signal_Receiver(bus)
        print 'Add signal receiver done'
        self.mainloop = gobject.MainLoop()
        self.my_ret = False

    def Add_Signal_Receiver(self, bus):
        pass

    def property_changed(
        self,
        name,
        value,
        path=None,
        interface=None,
        ):

#        print 'Changed property name = %s path = %s interface = %s' \
#            % (name, path, interface)

        if name == self.property_name:
            print 'Changed property name = %s path = %s interface = %s' \
                % (name, path, interface)
            self.my_ret = True
            self.mainloop.quit()


