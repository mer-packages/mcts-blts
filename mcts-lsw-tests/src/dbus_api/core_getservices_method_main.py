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

import getopt, sys, dbus, os
from lastfm_constant import Constants
from core_online_method_main import LswCoreIfaceService
reload(sys)
sys.setdefaultencoding("utf-8")


class lswCoreServiceIfaceCase (object):
    '''
-h     : print this help message and exit (also --help)
-e     : the expecting service, possible value: lastfm, twitter, myspace, and viemo
-k     : will check whether the available services are same with the "/usr/share/libsocialweb/keys" 
    '''
    def __init__(self):
        self.expect_service = None
        self.check_keys = False
    
    def init_opt(self):
        try:
            opts, args = getopt.getopt(sys.argv[1:], "hke:", ["help", "keys", "expectservice"])
        except getopt.error, msg:
            print msg
            print "for help use --help"
            sys.exit(Constants.ERROR_RETURN)
        for o, a in opts:
            if o in ("-h", "--help"):
                print self.__doc__
                sys.exit(Constants.OK_RETURN) 
            elif o in ("-e", "--expectservice"):
                self.expect_service = a
            elif o in ("-k", "--keys"):
                self.check_keys = True
            print "Option: %s" % o
            print "Args: %s" % a 
            
    def onlyCheckExpectService(self):
        return None != self.expect_service
    
    def get_expect_service(self):
        return self.expect_service
    
def check_service_invokable(serviceName):
    try:
        bus = dbus.SessionBus()
        path = "/com/meego/libsocialweb/Service/"+ serviceName
        serviceObj =  bus.get_object("com.meego.libsocialweb", path)
        print "Service Path: " + serviceObj.object_path + "\n"
        serviceObj = dbus.Interface(serviceObj, "com.meego.libsocialweb.Service")
        caps = serviceObj.GetStaticCapabilities()
    except dbus.exceptions.DBusException, msg:
        print "Fail to invoke the '%s' service" % serviceName
        print "[ErrorMessage] %s" % msg
        return False
    
    if(None != caps):
        print "The expecting service '%s' is in the available service list, and is invokable." % serviceName
        return True
    else:
        print "Fail! The expecting service %s isn't available." % serviceName
        return False
    
if __name__ == '__main__':
    case = lswCoreServiceIfaceCase()
    case.init_opt()
    
    core_service = LswCoreIfaceService()
    services = core_service.getServices()
    if(case.onlyCheckExpectService()):
        if( 0 == services.count(case.get_expect_service())):
            print "Fail! The expecting service %s isn't in the return of method 'GetService'" % case.get_expect_service()
            print "All available services: %s" % str(services)
            sys.exit(Constants.ERROR_RETURN)
        #the expecting service is included in the service list, check whether this service is invokable
        if (check_service_invokable(case.get_expect_service())):
            print "OK!"
            sys.exit(Constants.OK_RETURN)
        else:
            sys.exit(Constants.ERROR_RETURN)
            
    else:
        keys = os.listdir("/usr/share/libsocialweb/keys")
        if(len(keys) != len(services)):
            print "Fail! the size of available services isn't equal with the number of keys"
            print "All available services are %s" % str(services)
            print "All keys are %s" % str(keys)
            
            sys.exit(Constants.ERROR_RETURN)
        else:
            for key in keys:
                if(0 == services.count(key)):
                    print "Fail! The expecting service %s isn't in the return of method 'GetService'" % key
                    print "All available services: %s" % str(services)
                    sys.exit(Constants.ERROR_RETURN)
                if (not check_service_invokable(key)):
                    sys.exit(Constants.ERROR_RETURN)
            print "OK! All expecting service are available."
            sys.exit(Constants.OK_RETURN)
            