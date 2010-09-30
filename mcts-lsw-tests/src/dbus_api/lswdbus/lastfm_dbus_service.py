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
#    represent last.fm service which sit on DBus
#

import dbus, gobject, sys
from lsw_dbus_core import LswCoreService
reload(sys)
sys.setdefaultencoding("utf-8")

class LastfmDbusService(LswCoreService):
    '''
    represent the service API of last.fm service
    '''


    def __init__(self):
        '''
        Constructor
        '''
        LswCoreService.__init__(self)
        servicePath = "/com/meego/libsocialweb/Service/lastfm"
        self.service = self.bus.get_object(self.bus_name, servicePath)

class LastfmItemViewService(LastfmDbusService):
    '''
    represent the ItemView Interface of last.fm service
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        LastfmDbusService.__init__(self)
        queryIfaceName = "com.meego.libsocialweb.Query"
        self.service = dbus.Interface(self.service, queryIfaceName)
        
        viewpath = self.service.OpenView("feed", {})
        print "View Path: " + viewpath
        
        #Connect the new View Object
        self.viewobj = self.bus.get_object(self.bus_name, viewpath)
        