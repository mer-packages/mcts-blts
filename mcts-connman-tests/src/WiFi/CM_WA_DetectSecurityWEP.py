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
# Description: CM_WA_DetectSecurityWEP
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
manager.RequestScan('wifi')
time.sleep(10)
manager.RequestScan('wifi')
time.sleep(10)
svc = manager.GetServiceByName('shz13-otc-cisco-cm')
if svc == None:
    print 'There is no AP named shz13-otc-cisco-cm'
    EXIT(False)

props = svc.GetProperties()
security = props['Security'][0]
print 'Current security mothod is %s' % security
if security != 'wep':
    EXIT(False)

EXIT(True)

