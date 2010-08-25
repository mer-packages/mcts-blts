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
# Description: common code to check the value of a property 
#

import sys
import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
import time
import gobject

import dbus
import dbus.mainloop.glib

from common import *


def InListWiFiGuest(key, list):
    dev = WiFiGuestDevice()
    svc = dev.GetService()
    properties = svc.GetProperties()
    print properties
    if key in properties.keys():
        item = properties[key]
    else:
        print 'Property %s does not exist' % key
        return False
    if item in list:
        print '%s %s is valid' % (key, item)
        return True
    else:
        print '%s %s is invalid' % (key, item)
        return False


def InList3G(key, list):
    dev = C3GDevice()
    svc = dev.GetService()
    properties = svc.GetProperties()
    if key in properties.keys():
        item = properties[key]
    else:
        item = ''
    if item in list:
        print '%s %s is valid' % (key, item)
        return True
    else:
        print '%s %s is invalid' % (key, item)
        return False


def InListManager(key, list):
    properties = manager.GetProperties()
    print properties
    if key in properties.keys():
        item = properties[key]
    else:
        print 'Property %s does not exist' % key
        return False
    if item in list:
        print '%s %s is valid' % (key, item)
        return True
    else:
        print '%s %s is invalid' % (key, item)
        return False


def InListManagerM(key, list):
    properties = manager.GetProperties()
    print properties
    if key in properties.keys():
        items = properties[key]
    else:
        print 'Property %s does not exist' % key
        return False
    print 'items is %s' % items
    for item in items:
        print 'item is %s' % item
        if item in list:
            print '%s %s is valid' % (key, item)
        else:
            print '%s %s is invalid' % (key, item)
            return False

    return True


