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

import os, sys, subprocess, time, codecs, dbus
from lastfm_constant import Constants
from dbus_user_changed_signal import RecordSignal

reload(sys)
sys.setdefaultencoding("utf-8")



if __name__ == '__main__':
    
    UC_SIGNAL_FILE = "/tmp/qt.user.changed.signal"
    UC_SIGNAL = "UserChanged signal"
    DCC_SIGNAL_FILE = "/tmp/qt.dyn.caps.signal"
    DCC_SIGNAL = "DynCapsChanged signal"
    
    case = RecordSignal()
    case.init_opt()
    
    def getUserNameFromGnomeKeyRing():
        useraccount = ""
        f = os.popen("/opt/mcts-lsw-tests/dbus_api/reading_user_account -s %s" % case.get_service_name())
        for i in f.readlines():
            if("" != i):
                useraccount = i.strip()
        print "The account is %s" % useraccount
        return useraccount
    
    def getPasswordFromGnomeKeyRing():
        pwd = ""
        f = os.popen("/opt/mcts-lsw-tests/dbus_api/reading_user_account -s %s -p" % case.get_service_name())
        for i in f.readlines():
            if("" != i):
                pwd = i.strip()
        print "The password is %s" % pwd
        return pwd
    
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
            
    def readSignalUserchanged():
        signal = None
        f = open(UC_SIGNAL_FILE, "r")
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
        f = open(DCC_SIGNAL_FILE, "r")
        while(True):
            line = f.readline()
            if("" == line):
                break
            else:
                signal = line.strip()
        f.close()
        return signal
    
    print "Remove the history signals cache";
    os.system("rm -f %s" % UC_SIGNAL_FILE)
    if(case.need_record_caps_sig()):
        os.system("rm -f %s" % DCC_SIGNAL_FILE)
    
    print "Going to start the signal monitor"
    subp = subprocess.Popen(["/opt/mcts-lsw-tests/qt/common_swclientservice_userchanged_signal", '-s', case.get_service_name()]);
    
    time.sleep(1);
    
    ori_user = getUserNameFromGnomeKeyRing()
    ori_pwd = getPasswordFromGnomeKeyRing()
    
    new_user = "moblintest2"
    new_password = "otc111"
    
    os.system("/opt/mcts-lsw-tests/dbus_api/setting_user_account -s %s -u %s -p %s" % (case.get_service_name(), new_user, new_password))
    incokeCredentialsUpdated()
    
    time.sleep(2)
    signal = readSignalUserchanged()
    cap_sig = None
    if(case.need_record_caps_sig()):
        cap_sig = readCapsSignal()
    
    subp.kill()

    found = False
    if(None != signal and UC_SIGNAL == signal):
        print "OK! the UserChanged signal is received"
        found = True
        
    cap_found = False
    if(case.need_record_caps_sig()):
        if(None != cap_sig and DCC_SIGNAL == cap_sig):
            print "OK! the DynamicCapabilitiesChanged signal is received too"
            cap_found = True
            
    os.system("/opt/mcts-lsw-tests/dbus_api/setting_user_account -s %s -u %s -p %s" % (case.get_service_name(), ori_user, ori_pwd))
    incokeCredentialsUpdated()
    
    if(found and ((case.need_record_caps_sig() and cap_found)) or not ((case.need_record_caps_sig() or cap_found))):
        sys.exit(Constants.OK_RETURN)
    elif(found):
        print "Fail! to get the CapabilitiesChanged signal" 
        sys.exit(Constants.ERROR_RETURN)
    else:
        print "Fail! to get the UserChanged signal"    
        sys.exit(Constants.ERROR_RETURN)
    
    
