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
# Description: CM_BT_IfconfigUp
#

import sys
import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
import time
from common import *
dev = BTDevice()
ret = dev.ifconfigDown()
time.sleep(2)
ret = dev.ifconfigUP()
if ret == False:
    print 'ifconfig up failure is not connman issue'
    EXIT(True)
time.sleep(2)
dev = BTDevice()
dev.Connect()
svc = dev.GetService()
if svc.BroadcastPing() != True:
    print 'Cannot ping'
    EXIT(False)
EXIT(True)
