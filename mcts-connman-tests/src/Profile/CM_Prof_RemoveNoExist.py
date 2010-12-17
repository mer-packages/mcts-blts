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
# Description: CM_Prof_RemoveNoExist
#

import sys
import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
import time
import gobject

import dbus
from common import *

manager = manager.manager

try:
    manager.RemoveProfile('/profile/TestProfile')
except dbus.DBusException, e:
    pass

properties = manager.GetProperties()
profiles = properties['Profiles']

for path in profiles:
    id = path[path.rfind('/') + 1:]
    if id == 'TestProfile':
        print 'profile TestProfile still exists!'
        EXIT(False)

print 'Remove no exist profile TestProfile'
try:
    manager.RemoveProfile('/profile/TestProfile')
except dbus.DBusException, e:
    if e._dbus_error_name == 'net.connman.Error.NotFound':
        print 'Remove none exist profile trigger error NotFound'
        EXIT(True)
    else:
        print e
        EXIT(True)
print 'No error happens when remove none exist profile'
EXIT(False)

