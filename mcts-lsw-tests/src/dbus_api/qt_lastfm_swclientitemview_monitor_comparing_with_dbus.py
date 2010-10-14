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

import os, sys, subprocess, time, codecs
from lastfm_main import LastfmBasicCase, LastfmItem, diffItemIds
from lastfm_constant import Constants

reload(sys)
sys.setdefaultencoding("utf-8")

QT_CACHE_NAME = "/tmp/qt.lastfm.items.cache"


class lastfmQtItem(object):
    
    def __init__(self, uuid):
        self.title = ""
        self.thumbnail = ""
        self.authorid = ""
        self.url = ""
        self.album = ""
        self.author = ""
        self.id = uuid
        self.date = ""
        
    def getUuid(self):
        return self.id
    
    def get_track_album(self):
        return self.album;
    
    def get_user(self):
        return 
        
    def toString(self):
        str = ""
        for prop in self.__dict__.keys():
            if(prop != "toString" and (not (prop.startswith("get") or prop.startswith("__")))):
                str += "%s=%s" % (prop, self.__getattribute__(prop))
        return str

class lastfmQtMainCase (LastfmBasicCase):
    '''
-h     : print this help message and exit (also --help)
-w     : how long to wait for the libsocialweb update (unit: minute, for example "-w 5" mean waiting 5 mins to check the items of libsocialweb)
    '''
    
    def __init__(self):
        LastfmBasicCase.__init__(self)
        self.qt_items = {}
        
    def read_qt_memory_record(self):
        self.qt_items = {}
        f = codecs.open(QT_CACHE_NAME, encoding='utf-8', mode="r")
        line = f.readline()
        print "Preparing to read the memory_cache\n"
        
        while True:
            #print line
            if("" == line):
                break
            elif ("[" == line[0] and "]" == line[-2]):
                uuid = line[1 :-2]
                print "Item UUID: %s is loaded from Qt_API" % uuid 
                item = lastfmQtItem(uuid)
                line = f.readline()
                while ("" != line and "\n" != line):
                    for prop in item.__dict__.keys():
                        if(0 == line.find(prop + "=")):
                            item.__setattr__(prop, line[len(prop + "="):])
                    line = f.readline()
                self.qt_items[uuid.encode('utf-8')] = item
                print item.toString()
            else:
                line = f.readline()
        f.close()
        
    def qt_items_size(self):
        return len(self.qt_items.keys())
    
    
if __name__ == '__main__':
    
    basicCase = lastfmQtMainCase()
    basicCase.init_opt()
    
    subp = subprocess.Popen(["python", "/opt/mcts-lsw-tests/dbus_api/dbus_lastfm_itemview.py"])
    subq = subprocess.Popen(["/opt/mcts-lsw-tests/qt/lastfm_swclientitemview_monitor_ma"]);
    
    print "Going to sleep for %d mins\n" % basicCase.get_wait_minutes()
    time.sleep(basicCase.get_wait_minutes() * 60)
    print "Wake up from sleep\n"

    subp.kill()
    subq.kill()
    
    basicCase.read_dbus_memory_record()
    basicCase.read_qt_memory_record()
    
    if(basicCase.qt_items_size() != basicCase.items_size()):
        print "The size of items is not correct!\n"
        print " %d items on DBus, and %d items on Qt_API." % (basicCase.items_size(), basicCase.qt_items_size())
        basicCase.print_diff_items(basicCase.qt_items_size())
        sys.exit(Constants.ERROR_RETURN)
    else:
        unwantedIds = diffItemIds(basicCase.items, basicCase.qt_items)
        missedIds = diffItemIds(basicCase.qt_items, basicCase.items)
        
        if(0 == len(unwantedIds) and 0 == len(missedIds)):
            for dbusitem in basicCase.items.values():
                webItem = basicCase.qt_items[dbusitem.getUuid()]
                if(dbusitem.compareTo (webItem)):
                    print "Below item is different in DBus\n"
                    print "Qt_API item: \n" + webItem.toString()
                    print "DBus item: \n" + dbusitem.toString()
                    sys.exit(Constants.ERROR_RETURN)
            print "OK! the items are same between DBus and Qt_API"
            sys.exit(Constants.OK_RETURN) 
        else:
            if(0 != len(unwantedIds)):
                print "Some items are unwanted !\n"
                print unwantedIds
                
            if(0 != len(missedIds)):
                print "Some items are missed !\n"
                print missedIds
                
            sys.exit(Constants.ERROR_RETURN)
