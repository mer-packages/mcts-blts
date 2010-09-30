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

import sys, os, time, subprocess
from dbus_core_signal import ONLINE_SIGNAL
from core_online_method_main import LswCoreIfaceService
from lastfm_constant import Constants
reload(sys)
sys.setdefaultencoding("utf-8")

def readSignalOnLine():
    signal = None
    f = None
    while (None == f):
        try:
            f = open(ONLINE_SIGNAL, "r")
        except IOError, msg:
            print "wait 2 seconds for the signal"
            time.sleep(2)
    while(True):
        line = f.readline()
        if("" == line):
            break
        else:
            signal = line.strip()
    f.close()
    return signal

def isSignalOnLine():
    signal = readSignalOnLine()
    while (None == signal):
        print "wait 2 seconds for the signal"
        time.sleep(2)
        signal = readSignalOnLine()
    os.system("rm -f %s" % ONLINE_SIGNAL)
    if("On" == signal):
        return True
    else:
        return False

if __name__ == '__main__':
    service = LswCoreIfaceService()
    
    if(service.isOnline()):
        #Invoke the LSW thread to monitor DBus
        subp = subprocess.Popen(['python', 'dbus_core_signal.py'])
        
        time.sleep(10)
        
        os.system("ifconfig eth0 down")
        if(isSignalOnLine()):
            print "Fail! The current status is offline, and different with the status from DBus"
            subp.kill()
            os.system("ifconfig eth0 up")
            sys.exit(Constants.ERROR_RETURN)
            
        os.system("ifconfig eth0 up")

        if(not isSignalOnLine()):
            print "Fail! The current status is online, and different with the status from DBus"
            subp.kill()
            sys.exit(Constants.ERROR_RETURN)
        
        print "OK, the status from DBus is correct"
        subp.kill()
        sys.exit(Constants.OK_RETURN)
    else:
        print "Fail, the netbook is offline now. please make it online, then try this script again"
        sys.exit(Constants.ERROR_RETURN)