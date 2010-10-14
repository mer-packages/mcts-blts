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

import sys, os, time, subprocess, getopt, dbus
from dbus_user_changed_signal import USER_CHANGED_SIGNAL, CAP_CHANGED_SIGNAL, RecordSignal
from lastfm_constant import Constants

reload(sys)
sys.setdefaultencoding("utf-8")

def incokeCredentialsUpdated():
    try:
        bus = dbus.SessionBus()
        serviceObjPath = ("/com/meego/libsocialweb/Service/%s" % case.get_service_name())
        serviceIface = "com.meego.libsocialweb.Service"
        sw = bus.get_object("com.meego.libsocialweb", serviceObjPath)
        sw = dbus.Interface(sw, serviceIface)
        sw.CredentialsUpdated()
    except dbus.exceptions.DBusException, msg:
        print "[ErrorMessage] %s" % msg
        sys.exit(1)


if __name__ == '__main__':
    
    case = RecordSignal()
    case.init_opt()
    
    #os.system("rm -f %s" % USER_CHANGED_SIGNAL)
    subp = subprocess.Popen(['rm', '-f', 'USER_CHANGED_SIGNAL'])
    
    if(case.need_record_caps_sig()):
        os.system("rm -f %s" % CAP_CHANGED_SIGNAL)
        #Invoke the LSW thread to monitor DBus
        subp = subprocess.Popen(['%s/dbus_api/dbus_user_changed_signal.py' % Constants.INSTALL_PATH, '-s', case.get_service_name(), '-c'])
    else:
        #Invoke the LSW thread to monitor DBus
        subp = subprocess.Popen(['%s/dbus_api/dbus_user_changed_signal.py' % Constants.INSTALL_PATH, '-s', case.get_service_name()])
    
    time.sleep(10)
    
    def getUserNameFromGnomeKeyRing():
        useraccount = ""
        f = os.popen("%s/dbus_api/reading_user_account -s %s" % (Constants.INSTALL_PATH, case.get_service_name()))
        for i in f.readlines():
            if("" != i):
                useraccount = i.strip()
        print "The account is %s" % useraccount
        return useraccount
    
    def getPasswordFromGnomeKeyRing():
        pwd = ""
        f = os.popen("%s/dbus_api/reading_user_account -s %s -p" % (Constants.INSTALL_PATH, case.get_service_name()))
        for i in f.readlines():
            if("" != i):
                pwd = i.strip()
        print "The password is %s" % pwd
        return pwd
            
    def readSignalUserchanged():
        signal = None
        f = open(USER_CHANGED_SIGNAL, "r")
        while(True):
            line = f.readline()
            if("" == line):
                break
            else:
                signal = line.strip()
        f.close()
        return signal
    
    def readCapsSignal():
        signal = None
        f = open(CAP_CHANGED_SIGNAL, "r")
        while(True):
            line = f.readline()
            if("" == line):
                break
            else:
                signal = line.strip()
        f.close()
        return signal
    
    ori_user = getUserNameFromGnomeKeyRing()
    ori_password = getPasswordFromGnomeKeyRing()
    
    new_user = "moblintest2"
    new_password = "otc111"
    
    os.system("%s/dbus_api/setting_user_account -s %s -u %s -p %s" % (Constants.INSTALL_PATH, case.get_service_name(), new_user, new_password))
    
    incokeCredentialsUpdated()
    time.sleep(10)
    signal = readSignalUserchanged()
    cap_sig = None
    if(case.need_record_caps_sig()):
        cap_sig = readCapsSignal()
    
    subp.kill()
    
    found = False
    if(None != signal and "user_changed" == signal):
        print "OK! the UserChanged signal is received"
        found = True
    
    cap_found = False
    if(case.need_record_caps_sig()):
        if(None != cap_sig and "capabilities_changed" == cap_sig):
            print "OK! the CapabilitiesChanged signal is received too"
            cap_found = True
            
    os.system("%s/dbus_api/setting_user_account -s %s -u %s -p %s" % (Constants.INSTALL_PATH, case.get_service_name(), ori_user, ori_password))
    incokeCredentialsUpdated()
    
    if(found and ((case.need_record_caps_sig() and cap_found)) or not ((case.need_record_caps_sig() or cap_found))):
        sys.exit(Constants.OK_RETURN)
    elif(found):
        print "Fail! to get the CapabilitiesChanged signal" 
        sys.exit(Constants.ERROR_RETURN)
    else:
        print "Fail! to get the UserChanged signal"    
        sys.exit(Constants.ERROR_RETURN)
    
    
