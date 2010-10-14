#!/usr/bin/python
#
# Copyright (C) 2010, Intel Corporation.
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
#              Tang, Shao-Feng <shaofeng.tang@intel.com>
#
# Description:
#    represent the Libsocialweb core service
#

import dbus, gobject, sys
reload(sys)
sys.setdefaultencoding("utf-8")


class LswCoreService(object):
    '''
    represent the core API of lsw on DBus
    '''


    def __init__(self):
        '''
        Constructor
        '''
        self.bus_name = "com.meego.libsocialweb"
        self.bus = dbus.SessionBus()
        #self.bus.start_service_by_name(bus_name)