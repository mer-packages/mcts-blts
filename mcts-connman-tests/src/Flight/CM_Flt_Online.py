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
# Description: CM_Flt_Online
#

import sys
import os
dir=os.path.dirname(sys.argv[0])+"/common"
sys.path.append(dir)
import time
from common import *

print 'Online all device...'
dev = WiFiDevice()
dev.PoweredOn()
time.sleep(10)
dev = EthDevice()
dev.PoweredOn()
time.sleep(10)
print 'Set to Online mode...'
manager.Online()
time.sleep(5)
state = manager.GetState()
if state == 'online':
    print 'Manager state changed to online'
    EXIT(True)

print 'Manager state is %s' % state
EXIT(False)
