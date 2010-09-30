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

import getopt, sys, os, time
from lswdbus.lsw_dbus_core import LswCoreService
from lastfm_constant import Constants
reload(sys)
sys.setdefaultencoding("utf-8")

class LswCoreIfaceService(LswCoreService):
    
    def __init__(self):
        '''
        Constructor
        '''
        LswCoreService.__init__(self)
        servicePath = "/com/meego/libsocialweb"
        self.service = self.bus.get_object(self.bus_name, servicePath)
        
    def isOnline(self):
        return self.service.IsOnline()
    
    def getServices(self):
        return self.service.GetServices()
    
class lswCoreIfaceCase (object):
    '''
-h     : print this help message and exit (also --help)
-s     : the expecting status, valid list: on or off
    '''
    def __init__(self):
        self.expect_status = None
        
    def only_chcek_expected(self):
        return None != self.expect_status
    
    def expect_online(self):
        return "On" == self.expect_status or "on" == self.expect_status
    
    def expect_offline(self):
        return "Off" == self.expect_status or "off" == self.expect_status
    
    def init_opt(self):
        try:
            opts, args = getopt.getopt(sys.argv[1:], "hs:", ["help", "status"])
        except getopt.error, msg:
            print msg
            print "for help use --help"
            sys.exit(Constants.ERROR_RETURN)
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                sys.exit(Constants.OK_RETURN) 
            elif o in ("-s", "--status"):
                if("on" == a or "On" == a):
                    self.expect_status = "On"
                elif("off" == a or "Off" == a):
                    self.expect_status = "Off"
            print "Option: %s" % o
            print "Args: %s" % a 

if __name__ == '__main__':
    case = lswCoreIfaceCase()
    case.init_opt()
    
    service = LswCoreIfaceService()
    if(case.only_chcek_expected()):
        currentStaus = service.isOnline()
        if(currentStaus and case.expect_online()):
            print "OK! The current status is online, and same with the expecting status"
            sys.exit(Constants.OK_RETURN)
        elif((not currentStaus) and case.expect_offline()):
            print "OK! The current status is offline, and same with the expecting status"
            sys.exit(Constants.OK_RETURN)
        else:
            if currentStaus:
                print "Fail! The current status is online, and different with the expecting status"
            else:
                print "Fail! The current status is offline, and different with the expecting status"
            sys.exit(Constants.ERROR_RETURN)
    else:
        if(service.isOnline()):
            os.system("ifconfig eth0 down")
            if(service.isOnline()):
                print "Fail! The current status is offline, and different with the status from DBus"
                sys.exit(Constants.ERROR_RETURN)
            os.system("ifconfig eth0 up")
            
            time.sleep(10)
            if(not service.isOnline()):
                print "Fail! The current status is online, and different with the status from DBus"
                sys.exit(Constants.ERROR_RETURN)
            
            print "OK, the status from DBus is correct"
            sys.exit(Constants.OK_RETURN)
        else:
            print "Fail, the netbook is offline now. please make it online, then try this script again"
            sys.exit(Constants.ERROR_RETURN)
