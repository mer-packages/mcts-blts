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

import os, sys, subprocess, time, codecs, dbus, getopt
from lastfm_constant import Constants

class RecordItemHiddenSignal(object):
    '''
-h     : print this help message and exit (also --help)
-s     : the monitoring service, possible value: lastfm, twitter, myspace, and viemo (also --service)
-i     : the uuid of the hiding item
    '''
    
    def __init__(self):
        self.service = None
        self.p_uuid = None
        
    def get_service_name(self):
        return self.service
    
    def get_uuid(self):
        return self.p_uuid
    
    def quit_no_service(self):
        print "the monitoring service should be provided. for help use --help"
        sys.exit(Constants.ERROR_RETURN)
        
    def init_opt(self):
        try:
            opts, args = getopt.getopt(sys.argv[1:], "hi:s:", ["help", "uuid", "service"])
        except getopt.error, msg:
            print msg
            print "for help use --help"
            sys.exit(1)
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                sys.exit(0) 
            elif o in ("-s", "--service"):
                self.service = a
            elif o in ("-i", "--uuid"):
                self.p_uuid = a
        if(None == self.service):
            self.quit_no_service()
        if(None == self.p_uuid):
            print "the uuid of the hidding item should be provided. for help use --help"
            sys.exit(Constants.ERROR_RETURN)
        return self.service
    

if __name__ == '__main__':
    ITEM_HIDDEN_SIGNAL_FILE = "/tmp/qt.item.hidden.signal"
    ITEM_HIDDEN_SIGNAL = "ItemHidden signal"
    
    case = RecordItemHiddenSignal()
    case.init_opt()
    
    def readSignalItemHidden():
        signal = None
        f = open(ITEM_HIDDEN_SIGNAL_FILE, "r")
        while(True):
            line = f.readline()
            if("" == line):
                break
            else:
                signal = line.strip()
        f.close()
        return signal
    
    print "Remove the history signals cache";
    os.system("rm -f %s" % ITEM_HIDDEN_SIGNAL_FILE)
    
    print "Going to start the signal monitor"
    subp = subprocess.Popen(['/opt/mcts-lsw-tests/qt/common_swclientservice_itemhidden_signal', '-s', case.get_service_name()]);
    
    time.sleep(1);
    
    subq = subprocess.Popen(['/opt/mcts-lsw-tests/qt/common_swclientservice_hide_item', '-s', case.get_service_name(), '-i', case.get_uuid()]);
    
    time.sleep(2);
    
    subq.kill()
    subp.kill()
    
    signal = readSignalItemHidden();
    if(None != signal and ITEM_HIDDEN_SIGNAL == signal):
        print "OK! the ItemHidden signal is received, and it is successful invoking the method 'hideItem'"
        sys.exit(Constants.OK_RETURN)
    else:
        print "Fail! to get the ItemHidden signal"    
        sys.exit(Constants.ERROR_RETURN)
