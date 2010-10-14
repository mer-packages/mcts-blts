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


import sys, time
import dbus, gobject
from dbus.mainloop.glib import DBusGMainLoop
reload(sys)
sys.setdefaultencoding("utf-8")

ONLINE_SIGNAL = "/tmp/lsw_online_signal"

if __name__ == '__main__':
    DBusGMainLoop(set_as_default=True)
    
    bus = dbus.SessionBus()
    #bus.start_service_by_name("com.meego.libsocialweb")
    
    sw = bus.get_object("com.meego.libsocialweb", "/com/meego/libsocialweb")
    sw = dbus.Interface(sw, "com.meego.libsocialweb")
    
    def online(state):
        status_str = None
        if state:
            print "Currently online"
            status_str = "On"
        else:
            print "Currently offline"
            status_str = "Off"
        f = open(ONLINE_SIGNAL, "w")
        f.write(status_str.encode('utf-8'))
        f.close()
    
    sw.connect_to_signal("OnlineChanged", online)
    
    loop = gobject.MainLoop()
    loop.run()
