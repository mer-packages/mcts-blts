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


import dbus, gobject, os, sys, getopt
from dbus.mainloop.glib import DBusGMainLoop
reload(sys)
sys.setdefaultencoding("utf-8")

USER_CHANGED_SIGNAL = "/tmp/lsw_user_changed_signal"
CAP_CHANGED_SIGNAL = "/tmp/lsw_capabilities_changed_signal"

class RecordSignal(object):
    '''
-h     : print this help message and exit (also --help)
-s     : the monitoring service, possible value: lastfm, twitter, myspace, and viemo (also --service)
-c     : also monitor the 'CapabilitiesChanged' signal
    '''
    
    def __init__(self):
        self.service =  None
        self.record_cap_sig = False
        
    def get_service_name(self):
        return self.service
    
    def need_record_caps_sig(self):
        return self.record_cap_sig
    
    def quit_no_service(self):
        print "the monitoring service should be provided. for help use --help"
        sys.exit(1)
        
    def init_opt(self):
        try:
            opts, args = getopt.getopt(sys.argv[1:], "hcs:", ["help","capabilities", "service"])
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
            elif o in ("-c", "--capabilities"):
                self.record_cap_sig = True
        if(None == self.service):
            self.quit_no_service()
        return self.service
    
if __name__ == '__main__':
    
    def user_changed():
        print "user_changed"
        f = open(USER_CHANGED_SIGNAL, "w")
        f.write("user_changed".encode('utf-8'))
        f.close()
    
    def caps_changed(caps):
        print caps
        for cap in caps:
            print "Cap: %s \n" % cap
        f = open(CAP_CHANGED_SIGNAL, "w")
        f.write("capabilities_changed".encode('utf-8'))
        f.close()
    
    case = RecordSignal()
    case.init_opt()
    
    if(None == case.get_service_name()):
        case.quit_no_service()
    
    DBusGMainLoop(set_as_default=True)
    
    try:
        bus = dbus.SessionBus()
        #bus.start_service_by_name("com.meego.libsocialweb")
        serviceObjPath = ("/com/meego/libsocialweb/Service/%s" % case.get_service_name())
        serviceIface = "com.meego.libsocialweb.Service"
        sw = bus.get_object("com.meego.libsocialweb", serviceObjPath)
        sw = dbus.Interface(sw, serviceIface)
        
        sw.connect_to_signal("UserChanged", user_changed)
        sw.connect_to_signal("CapabilitiesChanged", caps_changed)
    except dbus.exceptions.DBusException, msg:
        print "[ErrorMessage] %s" % msg
        sys.exit(1)
    
    loop = gobject.MainLoop()
    loop.run()