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

from lastfm_constant import Constants
from lastfm_main import LastfmBasicCase
from model.myzone import MyZoneModel
from lswdbus.lastfm_dbus_service import LastfmDbusService
import time
import subprocess
import sys, string, os
import dbus
import codecs
from dbus_lastfm_banishable import LastfmBanishableDbusService, SIGNAL_NAME

reload(sys)
sys.setdefaultencoding("utf-8")


class LastfmBanishableIfaceCase(LastfmBasicCase):

    '''
-h     : print this help message and exit (also --help)
-w     : how long to wait for the libsocialweb update (unit: minute, for example "-w 5" mean waiting 5 mins to check the items of libsocialweb)
-e     : the user, if no itemId is specified, whose last status will be banished from the friends. If the itemId is specified, ignore this option
-i     : the item's ID, which is consis of the url of the last track and the user's name (for instance, "http://www.last.fm/music/Robert+Pattinson/_/Never+Think tinaknows")
-s     : check whether the 'ItemHidden' signal with the corresponding itemkey is received or not. defaultly, no need to check the signal.
    '''

    def __init__(self):
        LastfmBasicCase.__init__(self)
        self.banishing_user = None
        self.banishing_item_key = None
        self.checkSignal = False
        self.cmd = None
    
    
    def get_opt_strs(self):
        return "hsw:u:e:i:"
    
    def get_opt_list(self):
        return ["help", "signal", "wait", "user", "expectuser", "itemkey"]
    
    def get_banished_user(self):
        return self.banishing_user
    
    def get_banished_item_key(self):
        return self.banishing_item_key
    
    def needCheckSignal(self):
        return self.checkSignal
    
    def save_opt_attr(self, opts):
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                self.quit(Constants.ERROR_RETURN)
            elif o in ("-s", "--signal"):
                self.checkSignal = True
            elif o in ("-w", "--wait"):
                self.waitmins = string.atoi(a)
            elif o in ("-u", "--user"):
                self.account = a
            elif o in ("-e", "--expectuser"):
                self.banishing_user = a
            elif o in ("-i", "--itemkey"):
                self.banishing_item_key = a
                #self.banishing_item_key = self.banishing_item_key.replace(" ", "+")
            print "Option: %s" % o
            print "Args: %s" % a 
            
    def start_to_monitor_hidden_signal(self):
        command = "rm -f %s" % SIGNAL_NAME
        print "Runing command: %s" % command
        self.cmd = subprocess.Popen(['rm', '-f', SIGNAL_NAME])
        self.cmd = subprocess.Popen(['python', '%s/dbus_api/dbus_lastfm_banishable.py' % Constants.INSTALL_PATH])
        
    def quit(self, return_value): 
        if(None != self.cmd):
            self.cmd.kill()
            self.cmd = None
        sys.exit(return_value)

#Invoke the LSW thread to monitor DBus
def start_monitor_dbus(wmin):
    subp = subprocess.Popen(['python', '%s/dbus_api/dbus_lastfm_itemview.py' % Constants.INSTALL_PATH])
    
    print "Going to sleep for %d min to get items which is in the current itemview\n" % wmin
    time.sleep(wmin * 60)
    print "Waking up\n"
    
    subp.kill()
    


if __name__ == '__main__':
    banishableCase = LastfmBanishableIfaceCase()
    banishableCase.init_opt()
    
    banishedKey = None
    if(None == banishableCase.get_banished_item_key()):
        
        start_monitor_dbus(1)
        banishableCase.read_dbus_memory_record()
        
        # No item and user is specified, Banish the first item
        if(None == banishableCase.get_banished_user()):
            if(banishableCase.items_size() > 0):
                banishedKey = banishableCase.items.keys()[0]
            else:
                print "No items on DBus."
                banishableCase.quit(Constants.ERROR_RETURN)
        else:
            for itemKey in banishableCase.items.keys():
                if(itemKey.endswith(banishableCase.get_banished_user())):
                    banishedKey = itemKey
                    break
    else:
        banishedKey = banishableCase.get_banished_item_key()
    
    
    dbus_service = LastfmBanishableDbusService()
    if(banishableCase.needCheckSignal()):
        banishableCase.start_to_monitor_hidden_signal()
        time.sleep(30)
    
    if(dbus_service.invokeHideItem(banishedKey)):
        start_monitor_dbus(banishableCase.get_wait_minutes())
        banishableCase.read_dbus_memory_record()
        
        for itemkey in banishableCase.items:
            if(itemkey == banishedKey):
                print "Fail to banish the item, the item is still included in this itemview"
                banishableCase.quit(Constants.ERROR_RETURN)
        
        print "OK. The banished item isn't included in this itemview any more."
        
        if(banishableCase.needCheckSignal()):
            receive_signal = False
            print "Sleep 30 seconds to check the signal is emitted or not \n" 
            time.sleep(30)
            print "Waking up\n"
            
            receive_uuid = None 
            try:
                f = codecs.open(SIGNAL_NAME,encoding='utf-8', mode="r")
                line = ""
                while(True):
                    line = f.readline()
                    line = line.strip()
                    print line
                    if("" == line):
                        break
                    elif(banishedKey == line):
                        receive_signal = True
                        receive_uuid = line
                f.close()
            except IOError, msg:
                print "Fail to read %s, [ERROR]:%s" % (SIGNAL_NAME, msg)
            if(not receive_signal):
                print "Fail to receive the 'ItemHidden' signal with the corresponding itemkey (%s), the received hidden itemkey is '%s'" % (banishedKey, receive_uuid)
                banishableCase.quit(Constants.ERROR_RETURN)
            else:
                print "OK! The 'ItemHidden' signal with the corresponding itemkey is received"
        banishableCase.quit(Constants.OK_RETURN)
    else:
        banishableCase.quit(Constants.ERROR_RETURN)
    
