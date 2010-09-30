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
import time
import subprocess
import sys, string
reload(sys)
sys.setdefaultencoding("utf-8")


def checkExpectedKey(expectedKey):
    #pre-check whether the expected key already exit in the response from last.fm
    prefound = False
    if(myzone.items_size() > 0):
        for itemkey in myzone.get_items().keys():
            if(-1 != itemkey.find(expectedkey)):
                print "The expected song and associated user is in the response of last.fm"
                prefound = True;
    else:
        print "Got nothing from last.fm. at least, 1 item is needed"
        sys.exit(Constants.ERROR_RETURN)
        
    if(prefound):
        for itemkey in checkNewItemCase.items.keys():
            if(-1 != itemkey.find(expectedkey)):
                print "OK. The expected song and associated user is in the items on DBus"
                sys.exit(Constants.OK_RETURN)
    else:
        print "The expected song and associated user isn't in the response of last.fm"
        print "Please update the expected value on last.fm first"
        sys.exit(Constants.ERROR_RETURN)

class LastfmCheckNewItemCase(LastfmBasicCase):

    '''
-h     : print this help message and exit (also --help)
-w     : how long to wait for the libsocialweb update (unit: minute, for example "-w 5" mean waiting 5 mins to check the items of libsocialweb)
-e     : the expecting new user
-s     : the expecting new Song
    '''

    def __init__(self):
        LastfmBasicCase.__init__(self)
        self.expect_user = None
        self.expect_song = None
    
    
    def get_opt_strs(self):
        return "hw:u:e:s:"
    
    def get_opt_list(self):
        return ["help", "wait", "user", "expectuser", "song"]
    
    def get_expect_user(self):
        return self.expect_user
    
    def get_expect_song(self):
        return self.expect_song
    
    def save_opt_attr(self, opts):
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                sys.exit(Constants.OK_RETURN) 
            elif o in ("-w", "--wait"):
                self.waitmins = string.atoi(a)
            elif o in ("-u", "--user"):
                self.account = a
            elif o in ("-e", "--expectuser"):
                self.expect_user = a
            elif o in ("-s", "--song"):
                self.expect_song = a
                self.expect_song = self.expect_song.replace(" ", "+")
            print "Option: %s" % o
            print "Args: %s" % a 
            
            

if __name__ == '__main__':
    checkNewItemCase = LastfmCheckNewItemCase()
    checkNewItemCase.init_opt()
    
    #Invoke the LSW thread to monitor DBus
    #os.system("python dbus_lastfm_itemview.py")
    subp = subprocess.Popen(['python', 'dbus_lastfm_itemview.py'])
    
    print "Going to sleep for %d mins\n" % checkNewItemCase.get_wait_minutes()
    time.sleep(checkNewItemCase.get_wait_minutes() * 60)
    print "Wake up from sleep\n"
    
    subp.kill()
    
    myzone = MyZoneModel(checkNewItemCase.get_user_account(), Constants.TEST_API_KEY)
    response_xml = myzone.invokeRestApi()
    myzone.parsResponseXml(response_xml)
    myzone.updateFriendsRecentTrack()
    
    checkNewItemCase.read_dbus_memory_record()
    
    if(myzone.items_size() != checkNewItemCase.items_size()):
        print "The size of items is not correct!\n"
        print " %d items on DBus, and %d items are expected." % (checkNewItemCase.items_size(), myzone.items_size())
        checkNewItemCase.print_diff_items(myzone.get_items())
        sys.exit(Constants.ERROR_RETURN)
    else:
        if(None != checkNewItemCase.get_expect_user() and None != checkNewItemCase.get_expect_song()):
            expectedkey = "%s %s" % (checkNewItemCase.get_expect_song(), checkNewItemCase.get_expect_user())
            checkExpectedKey(expectedkey)
        elif(None != checkNewItemCase.get_expect_user()):
            expectedkey = checkNewItemCase.get_expect_user()
            checkExpectedKey(expectedkey)
        elif(None != checkNewItemCase.get_expect_song()):
            expectedkey = checkNewItemCase.get_expect_song()
            checkExpectedKey(expectedkey)
        else:
            print "Please provide the expecting song or user to try again"
            sys.exit(Constants.ERROR_RETURN)
            