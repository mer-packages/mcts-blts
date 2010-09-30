#!/usr/bin/python

# Copyright (C) 2010 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU Lesser General Public License,
# version 2.1, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
# Authors:
#              Tang, Shao-Feng <shaofeng.tang@intel.com>
#

import dbus, gobject, sys
from dbus.mainloop.glib import DBusGMainLoop
from lswdbus.lastfm_dbus_service import LastfmDbusService

reload(sys)
sys.setdefaultencoding("utf-8")

SIGNAL_NAME = "/tmp/lastfm.item.hidden"

class LastfmBanishableDbusService(LastfmDbusService):
    '''
    represent the ItemView Interface of last.fm service
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        LastfmDbusService.__init__(self)
        banishableIfaceName = "com.meego.libsocialweb.Banishable"
        self.service = dbus.Interface(self.service, banishableIfaceName)
        self.loop = None
        
    def invokeHideItem(self, itemId):
        if(None != itemId and len(itemId.strip()) > 0):
            print "Try to invoke HideItem(uuid) with uuid (%s)" % itemId
            self.service.HideItem(itemId)
            return True
        else:
            print "Try to invoke HideItem(uuid) with None or blank"
            return False
        
    def __connectToItemHiddenSignal(self):
        self.service.connect_to_signal("ItemHidden", self.hidden_signal_cb, member_keyword="member")
        
    
    def startMonitor(self):
        self.__connectToItemHiddenSignal()
        self.loop = gobject.MainLoop()
        self.loop.run()
    
        
    def hidden_signal_cb(self, uuid, member="member"):
        print "The 'ItemHidden' Signal is received. and the uuid of item is %s" % uuid
        print member
        self.saveInFile(uuid)
        if(None != self.loop and self.loop.is_running()):
            print "Going to quit from gobject.MainLoop()"
            self.loop.quit()
        
        
    def saveInFile(self, uuid):        
        f = open(SIGNAL_NAME, "w")
        f.write(uuid.encode('utf-8'))
        f.close()
        
if __name__ == '__main__':
    DBusGMainLoop(set_as_default=True)
    service = LastfmBanishableDbusService()
    print "Starting to minitor the 'ItemHidden' Signal"
    service.startMonitor()