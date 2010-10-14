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


import os, sys
import time
from model.myzone import MyZoneModel
from lswdbus.lastfm_itemview_iface import CACHE_NAME
import string
import codecs
import shlex, subprocess
import getopt
from lastfm_constant import Constants
reload(sys)
sys.setdefaultencoding("utf-8")


def diffItemIds(oneDict, anotherDict):
    ids = []
    for litem in oneDict.keys():
        if(not litem in anotherDict.keys()):
            ids.append(litem)
    return ids;

def readTheUserAccount():
    useraccount = ""
    f=os.popen("/opt/mcts-lsw-tests/dbus_api/reading_user_account -s lastfm")
    for i in f.readlines():
        if("" != i):
            useraccount = i.strip()
    print "The account is %s" % useraccount
    return useraccount

class LastfmItem (object):
    
    def __init__(self, uuid):
        self.uuid = uuid;
        self.album = ""
        self.title = ""
        self.url = ""
        self.author = ""
        self.authoricon = ""
        self.authorid = ""
        self.date = ""
        self.thumbnail = "" 
        
    def getUuid(self):
        return self.uuid
        
    def toString(self):
        str = ""
        for prop in LastfmItem.__dict__.keys():
            if(prop != "toString" and (not (prop.startswith("get") or prop.startswith("__")))):
                str += "%s=%s" % (prop, self.__getattribute__(prop))
        return str
    
    def compareTo(self, webItem):
        result = True
        if(self.album != webItem.get_track_album()):
            result = False
        elif(self.author != webItem.get_user()):
            result = False
        elif(self.date != webItem.get_date()):
            result = False
        return result
    
class LastfmBasicCase(object):
    
    '''
-h     : print this help message and exit (also --help)
-w     : how long to wait for the libsocialweb update (unit: minute, for example "-w 5" mean waiting 5 mins to check the items of libsocialweb)
    '''
    
    def __init__(self):
        self.items = {}
        self.account = None
        self.waitmins = 1
        
    
    def read_dbus_memory_record(self):      
        self.items = {}
        f = codecs.open(CACHE_NAME,encoding='utf-8', mode="r")
        line = f.readline()
        print "Preparing to read the memory_cache\n"
        
        while True:
            #print line
            if("" == line):
                break
            elif ("[" == line[0] and "]" == line[-2]):
                uuid = line[1 : -2]
                print "Item UUID: %s is loaded from DBus" % uuid 
                item = LastfmItem(uuid)
                line = f.readline()
                while ("" != line and "\n" != line):
                    for prop in item.__dict__.keys():
                        if(0 == line.find(prop + "=")):
                            item.__setattr__(prop, line[len(prop + "="):])
                    line = f.readline()
                self.items[uuid.encode('utf-8')] = item
            else:
                line = f.readline()
        f.close()
        
    def items_size(self):
        return len(self.items.keys())
    
    
    def print_diff_items(self, webItemDicts):
        print "Unwanted items: " + str(diffItemIds(self.items, webItemDicts))
        print "Missed items: "+ str(diffItemIds(webItemDicts, self.items))
        
    def get_user_account(self):
        return self.account
    
    def get_wait_minutes(self):
        return self.waitmins
    
    def get_opt_strs(self):
        return "hw:u:"
    
    def get_opt_list(self):
        return ["help", "wait", "user"]
    
    def save_opt_attr(self, opts):
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                sys.exit(Constants.OK_RETURN) 
            elif o in ("-w", "--wait"):
                self.waitmins = string.atoi(a)
            elif o in ("-u", "--user"):
                self.account = a
            print "Option: %s" % o
            print "Args: %s" % a 
        
    def init_opt(self):
        try:
            opts, args = getopt.getopt(sys.argv[1:], self.get_opt_strs(), self.get_opt_list())
        except getopt.error, msg:
            print msg
            print "for help use --help"
            sys.exit(Constants.ERROR_RETURN)
        # process options
        self.save_opt_attr(opts)
        
        if(None == self.account):
            self.account = readTheUserAccount()
        if(None == self.account or "" == self.account):
            print "Fail to extract the user account. please set the account on MeeGo first"
            sys.exit(Constants.ERROR_RETURN)

if __name__ == '__main__':
    
    basicCase = LastfmBasicCase()
    basicCase.init_opt()
    
    #Invoke the LSW thread to monitor DBus
    subp = subprocess.Popen(['python', '%s/dbus_api/dbus_lastfm_itemview.py' % Constants.INSTALL_PATH ])
    
    print "Going to sleep for %d mins\n" % basicCase.get_wait_minutes()
    time.sleep(basicCase.get_wait_minutes() * 60)
    print "Wake up from sleep\n"
    
    subp.kill()
    
    
    myzone = MyZoneModel(basicCase.get_user_account(), Constants.TEST_API_KEY)
    response_xml = myzone.invokeRestApi()
    myzone.parsResponseXml(response_xml)
    myzone.updateFriendsRecentTrack()
    
    basicCase.read_dbus_memory_record()
    
    if(myzone.items_size() != basicCase.items_size()):
        print "The size of items is not correct!\n"
        print " %d items on DBus, and %d items are expected." % (basicCase.items_size(), myzone.items_size())
        basicCase.print_diff_items(myzone.get_items())
        sys.exit(Constants.ERROR_RETURN)
    else:
        unwantedIds = diffItemIds(basicCase.items, myzone.get_items())
        missedIds = diffItemIds(myzone.get_items(), basicCase.items)
        
        if(0 == len(unwantedIds) and 0 == len(missedIds)):
            for dbusitem in basicCase.items.values():
                webItem = myzone.get_items()[dbusitem.getUuid()]
                if(dbusitem.compareTo (webItem)):
                    print "Below item is different in DBus\n"
                    print "Web item: \n" + webItem.toString()
                    print "DBus item: \n" + dbusitem.toString()
                    sys.exit(Constants.ERROR_RETURN)
            sys.exit(Constants.OK_RETURN) 
        else:
            if(0 != len(unwantedIds)):
                print "Some items are unwanted !\n"
                print unwantedIds
                
            if(0 != len(missedIds)):
                print "Some items are missed !\n"
                print missedIds
                
            sys.exit(Constants.ERROR_RETURN)
