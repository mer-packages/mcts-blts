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
# Description: CM_WA_ForgotPassword
#

import sys
import time
import dbus

import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
from common import *

if manager.Apset('1111000000', 'wep64', 1, 0, 1) == False:
    print 'Cannot access WiFi'
    EXIT(False)

bus = dbus.SystemBus()

manager = dbus.Interface(bus.get_object('net.connman', '/'),
                         'net.connman.Manager')

properties = manager.GetProperties()

for path in properties['Services']:
    service = dbus.Interface(bus.get_object('net.connman',
                             path), 'net.connman.Service')

    properties = service.GetProperties()
    if properties['Name'] != 'shz13-otc-cisco-cm':
        continue
    print '%s Found with path=%s' % (properties['Name'], path)
    svc = dbus.Interface(bus.get_object('net.connman', path),
                         'net.connman.Service')
    svc.SetProperty('Passphrase', '1111000001')
    try:
        svc.Connect()
    except dbus.DBusException, error:
        pass
    time.sleep(5)
    count=1
    while count < 5:
        try:
            properties = svc.GetProperties()
            if properties['State'] == 'failure':
                EXIT(True)
        except dbus.DBusException, error:
            pass
        time.sleep(5)
        count = count + 1
EXIT(False)
