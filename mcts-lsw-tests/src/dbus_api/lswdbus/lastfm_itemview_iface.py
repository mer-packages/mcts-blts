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
#    this class represent the lastfm dbus service. 
#    and take the charge in receiveing the items info from dbus 
#

import sys, time
import dbus, gobject
from dbus.mainloop.glib import DBusGMainLoop
from lastfm_dbus_service import LastfmItemViewService
reload(sys)
sys.setdefaultencoding("utf-8")


def now():
    return time.strftime("%T", time.localtime())

CACHE_NAME = "/tmp/lastfm.items.cache"


class LastfmItemViewInterface(LastfmItemViewService):
    '''
    this class represent the lastfm dbus service. 
    and take the charge in receiveing the items info from dbus 
    '''

    def __init__(self):
        LastfmItemViewService.__init__(self)
        self.__lastfmitems = {}
        ifacepath = "com.meego.libsocialweb.ItemView"
        self.__view = dbus.Interface(self.viewobj, ifacepath)
        
    def get_lastfm_items(self):
        return self.__lastfmitems
    

    def queryItemByUuid(self, key):
        return self.__lastfmitems[key]


    def insertlastfmitems(self, key, value):
        self.__lastfmitems[key] = value
        
    
    def removeFromlastfmitems(self, key):
        del self.__lastfmitems[key]
        
        
    def connect_to_signals (self):
        '''
        Connect all 3 signals to DBus
        '''
        self.__view.connect_to_signal("ItemsAdded", self.added, member_keyword="member")
        self.__view.connect_to_signal("ItemsChanged",self.changed, member_keyword="member")
        self.__view.connect_to_signal("ItemsRemoved", self.removed)
        
        
    def startMonitoring(self):
        if(None != self.__view):
            self.connect_to_signals()
            self.__view.Start()
            return True
        else:
            return False
    
    def saveInFile(self):
        text = ""
        for itemkey in self.__lastfmitems.keys():
            text += "[%s]\n" % itemkey
            item = self.__lastfmitems[itemkey]
            for key in item:
                text +=  "%s=%s\n" % (key, item[key])
            text += "\n"
        f = open(CACHE_NAME, "w")
        f.write(text.encode('utf-8'))
        f.close()

    def added(self, items, member=None):
        for (service, uuid, date, item) in items:
            self.insertlastfmitems(uuid, item)
            print "[%s] %s: ItemAdded %s from %s" % (now(), member, uuid, service)
            print "Timestamp: %s" % time.strftime("%c", time.gmtime(date))
            for key in item:
                print "  %s: %s" % (key, item[key])
        print
        self.saveInFile()
        

    def changed(self, items, member=None):
        for (service, uuid, date, item) in items:
            self.insertlastfmitems(uuid, item)
            print "[%s] %s: ItemChanged %s from %s" % (now(), member, uuid, service)
            print "Timestamp: %s" % time.strftime("%c", time.gmtime(date))
            for key in item:
                print "  %s: %s" % (key, item[key])
        print
        self.saveInFile()

    def removed(self, items):
        for (service, uuid) in items:
            self.removeFromlastfmitems(uuid)
            print "[%s] Item %s removed from %s" % (now(), uuid, service)
            print
        self.saveInFile()
