#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2010 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307 USA.
#
# Authors:
#       Jeff Zheng <jeff.zheng@intel.com>
#
# Description: common functions used in all test cases
#

import dbus
import string
import commands
import time
import re
from config import *

###################################################
# ........Common Function
###################################################


def IPIsValid(ip):
    return re.match("(\d\d?\d?\.){3}\d\d?\d?", ip) != None


def MacAddrIsValid(mac):
    return re.match("([a-fA-F0-9]{2}[:|\-]?){5}[a-fA-F0-9]{2}", mac) \
        != None


def EXIT(ret):
    if ret == True:
        print 'Test Case PASS'
    else:
        print 'Test Case FAIL'
    exit(ret != True)

def EXIT1(ret, op):
    if ret == True:
        op()
        print 'Test Case PASS'
    else:
        op()
        print 'Test Case FAIL'
    exit(ret != True)

def ASSERT(ret, msg, op):
    if ret == False:
        op()
        print msg
        print 'Test Case FAIL'
        exit(ret != True)

###################################################
# ........Class Manager
###################################################


class Manager:

    def __init__(self):
        self.bus = dbus.SystemBus()
        if self.bus == None:
            print 'There is no dbus, please check'
            EXIT(False)
        self.manager = \
            dbus.Interface(self.bus.get_object('net.connman', '/'
                           ), 'net.connman.Manager')
        if self.manager == None:
            print 'There is no connmand...do you startup connmand?'
            EXIT(False)
        print 'Attached to a connman manager'
        self.properties = self.manager.GetProperties()
        self.LatestSvc = None

    def GetProperties(self):
        return self.manager.GetProperties()

    def SetProperty(self, name, value):
        self.manager.SetProperty(name, value)
        print 'Set Manager property %s = %s' % (name, value)

    def GetState(self):
        return self.manager.GetState()

    def CreateProfile(self, name):
        print 'Create profile %s' % name
        return self.manager.CreateProfile(name)

    def RemoveProfile(self, path):
        print 'Removing profile %s ...' % path
        self.manager.RemoveProfile(path)
        print 'Removed'

    def RequestScan(self, type):
        if type == '':
            print 'Scanning for all technologies...'
        else:
            print 'Scanning for %s technology...' % type
        self.manager.RequestScan(type)
        print 'Done'

    def EnableTechnology(self, type):
        self.Online()
        if type == '':
            print 'EnableTechnology for all technologies...'
            props = self.manager.GetProperties()
            for tech in props['AvailableTechnologies']:
                print 'Enabling %s...' % tech
                try:
                    self.manager.EnableTechnology(tech)
                except dbus.DBusException, e:
                    pass
        else:
            print 'EnableTechnology for %s technology...' % type
            try:
                self.manager.EnableTechnology(type)
            except dbus.DBusException, e:
                pass
        time.sleep(5)
        print 'Done'

    def DisableTechnology(self, type):
        if type == '':
            print 'DisableTechnology for all technologies...'
            props = self.manager.GetProperties()
            for tech in props['EnabledTechnologies']:
                print 'Disabling %s...' % tech
                try:
                    self.manager.DisableTechnology(tech)
                except dbus.DBusException, e:
                    pass
        else:
            print 'DisableTechnology for %s technology...' % type
            try:
                self.manager.DisableTechnology(type)
            except dbus.DBusException, e:
                pass
        time.sleep(5)
        print 'Done'

    def Startup(self):
        self.DisableTechnology('')
        self.EnableTechnology('ethernet')
        eth = EthDevice()
        time.sleep(10)
        self.EnableTechnology('wifi')
        time.sleep(10)

    def StartupWiFi(self):
        self.Startup()

    def StartupEthernet(self):
        self.DisableTechnology('')
        self.EnableTechnology('ethernet')
        eth = EthDevice()
        time.sleep(10)

    def Startup3G(self):
        self.DisableTechnology('')
        self.EnableTechnology('cellular')
        time.sleep(10)

    def StartupBT(self):
        self.DisableTechnology('')
        self.EnableTechnology('bluetooth')
        time.sleep(10)

    def Cleanup(self):
        self.DisableTechnology('')
        self.EnableTechnology('ethernet')
        eth = EthDevice()
        time.sleep(10)
        self.EnableTechnology('wifi')
        time.sleep(5)

    def RegisterAgent(self, path):
        print 'RegisterAgent for path'
        self.manager.RegisterAgent(path)

    def UnregisterAgent(self, path):
        print 'UnregisterAgent for path'
        self.manager.UnregisterAgent(path)

    def RegisterCounter(self, path, interval):
        print 'RegisterCounter for path'
        self.manager.RegisterCounter(path, interval)

    def UnregisterCounter(self, path):
        print 'UnregisterCounter for path'
        self.manager.UnregisterCounter(path)

    def RequestSession(self, bearer):
        print 'Request a networking session for %s' % bearer
        return self.manager.RequestSession(bearer)

    def ReleaseSession(self):
        print 'Release a networking session'
        return self.manager.ReleaseSession()

    def Offline(self):
        self.SetProperty('OfflineMode', dbus.Boolean(1))

    def Online(self):
        self.SetProperty('OfflineMode', dbus.Boolean(0))

    def GetSubObject(self, path, name):
        interface = 'net.connman.' + name
        self.subobj = \
            dbus.Interface(self.bus.get_object('net.connman',
                           path), interface)
        return self.subobj

    def ConnectService(
        self,
        ssid,
        passphrase='',
        security='none',
        ):

        print 'Connect to Service %s with passphrase = %s security = %s...' \
            % (ssid, passphrase, security)
        try:
            path = self.manager.ConnectService({
                'Type': 'wifi',
                'Mode': 'managed',
                'SSID': ssid,
                'Security': security,
                'Passphrase': passphrase,
                })
        except dbus.DBusException, e:
            return None
        print 'Connected to %s' % path
        return path


    # ConnectWiFi: Connect to an WiFi AP with relative params
    # passphrase: passphrase string
    # security: wep64/wep128/psk/psk1/psk2/wpa/rsn/wep64s/wep128s
    # open: 0: shared 1: open
    # hidden: 0: broadcast 1: hidden
    # channel: 1 - 11 
    def ConnectWiFi(
        self,
        passphrase='',
        security='psk',
        open=1,
        hidden=0,
        channel=1,
        ):
        if self.Apset(passphrase, security, open, hidden, channel):
            return self.ConnectAP(cm_apset_ap_essid, passphrase, security, 
                                  open, hidden, channel)
        return False
    def TurnAP(self, state):
        apset_cmd = 'ssh ' + cm_apset_server + ' ' + cm_apset_server_path 
        if state == 'on':
            apset_cmd = apset_cmd + ' activate'
        else:
            apset_cmd = apset_cmd + ' shutdown'

        ret = commands.getstatusoutput(apset_cmd)
        time.sleep(5)
        if ret[0] != 0:
            print 'Command %s return fail' % apset_cmd
            return False
        print 'apset done'
        return True

    # Apset: Setting AP with relative params
    # Parameter values are same as ConnectWiFi
    def Apset(
        self,
        passphrase='',
        security='psk',
        open=1,
        hidden=0,
        channel=1,
        ):
        print 'Setting AP...'
        apstr = ' MODE N2.4-ONLY '
        apstr += ' CHANNEL ' + `channel`
        if open == 1:
            apstr += ' AUTH open '
        else:
            apstr += ' AUTH shared '
        if hidden == 0:
            apstr += ' BCAST enable '
        else:
            apstr += ' BCAST disable '
        if security == 'wep64':
            apstr += ' WEP enable ENCRYPT 64 KEY1 ' + `passphrase`
        elif security == 'wep128':
            apstr += ' AUTH WEP128 ENCRYPT 128 KEY1 ' + `passphrase`
        if security == 'wep64s':
            apstr += ' WEP enable ENCRYPT 64 KEY1 ' + `4141414141`
        elif security == 'wep128s':
            apstr += ' AUTH WEP128 ENCRYPT 128 KEY1 ' \
                + `41414141414141414141414141`
        elif security == 'psk':
            apstr += ' WEP disable AUTH psk WPA-CRYPTO TKIP '
        elif security == 'psk1':
            apstr += ' WEP disable AUTH psk WPA-CRYPTO TKIP '
        elif security == 'psk2':
            apstr += ' WEP disable AUTH psk2 WPA-CRYPTO AES '
        elif security == 'wpa':
            apstr += ' WEP disable AUTH psk WPA-CRYPTO TKIP '
        elif security == 'rsn':
            apstr += ' WEP disable AUTH psk2 WPA-CRYPTO AES '
        print 'apset string is %s' % apstr
        apset_cmd = 'ssh ' + cm_apset_server + ' ' + cm_apset_server_path \
            + apstr
        ret = commands.getstatusoutput(apset_cmd)
        if ret[0] != 0:
            print 'Command %s return fail' % apset_cmd
            return False
        print 'apset done'
        return True

    # Refer to ConnectWiFi
    def ConnectAP(
        self,
        essid=cm_apset_ap_essid,
        passphrase='',
        security='psk',
        open=1,
        hidden=0,
        channel=1,
        ):
        print 'Connecting to AP...'
        if security == 'wep64':
            sec = 'wep'
        elif security == 'wep128':
            sec = 'wep'
        if security == 'wep64s':
            sec = 'wep'
            passphrase = 's:AAAAA'
        elif security == 'wep128s':
            sec = 'wep'
            passphrase = 's:AAAAAAAAAAAAA'
        elif security == 'psk':
            sec = 'psk'
        elif security == 'psk1':
            sec = 'psk'
        elif security == 'psk2':
            sec = 'psk'
        elif security == 'wpa':
            sec = 'wpa'
        elif security == 'rsn':
            sec = 'rsn'
        self.manager.RequestScan('wifi')
        time.sleep(2)
        path = self.ConnectService(essid, passphrase,
                                   sec)
        print 'essid is %s passphrase is %s sec is %s' % (essid, passphrase, sec)
        time.sleep(5)
        print 'path is %s' % path
        if path == None:
            print "ConnectService failed"
            return None
        i = 1
        while i < 3:
            svc = self.GetService(path)
            if svc == None:
                continue
            if svc.IsReady():
                self.LatestSvc = svc
                break
            else:
                svc.Connect()
                time.sleep(5)
            i += 1
        time.sleep(5)
        return svc.BroadcastPing()

    def GetService(self, path=None):
        if path == None:
            return self.LatestSvc
        service = None
        self.LatestSvc = None
        try:
            service = \
                dbus.Interface(self.bus.get_object('net.connman'
                               , path), 'net.connman.Service')
        except dbus.DBusException, e:
            return None
        time.sleep(1)
        svc = Service(service)
        print 'Service %s found' % path
        self.LatestSvc = svc
        return svc

    def GetServiceByName(self, Name):
        props = self.GetProperties()
        for path in props['Services']:
            service = \
                dbus.Interface(self.bus.get_object('net.connman'
                               , path), 'net.connman.Service')
            properties = service.GetProperties()
            if properties['Name'] == Name:
                svc = Service(service)
                return svc
        return None


def ManagerObjectIsReady():
    if manager.bus == None:
        print 'There is no dbus'
        return False
    if manager.manager == None:
        print 'There is no connman, please check if there is connmand'
        return False
    return True


def ManagerSubObjectIsReady(name):
    SubObjectNameMap = {'Service': 'Services',
                        'Technology': 'Technologies',
                        'Profile': 'Profiles'}
    name1 = SubObjectNameMap[name]
    for path in manager.properties[name1]:
        print path
        service = manager.GetSubObject(path, name)
        if service != None:
            print 'There is at least one %s, the testing success!' \
                % name
            return True
    print 'We cannot find any %s object, the testing fails!' % name
    return False


def ServiceObjectIsReady():
    return ManagerSubObjectIsReady('Service')


def TechnologyObjectIsReady():
    return ManagerSubObjectIsReady('Technology')


def ProfileObjectIsReady():
    return ManagerSubObjectIsReady('Profile')


###################################################
# ........Class Service
###################################################

class Service:

    def __init__(self, svc):
        self.svc = svc

    def GetProperties(self):
        try:
            self.properties = self.svc.GetProperties()
        except dbus.DBusException, e:
            return None
        return self.properties

    def GetProperty(self, name):
        props = self.GetProperties()
        if name in props.keys():
            return props[name]
        return None

    def SetProperty(self, name, value):
        print 'Set Service property %s = %s' % (name, value)
        self.svc.SetProperty(name, value)

    def ClearProperty(self, name):
        self.svc.ClearProperty(name)

    def Connect(self):
        if self.svc == None:
            print 'No such service'
            return False
        print 'Connecting...'
        loop = 1
        while loop < 3:
            try:
                self.svc.Connect()
            except dbus.DBusException, e:
                if e._dbus_error_name \
                    == 'net.connman.Error.AlreadyConnected':
                    print 'The service is already connected'
                    return True
                elif e._dbus_error_name \
                    == 'net.connman.Error.InProgress':
                    loop = loop + 1
                    print 'Still in connecting'
                    time.sleep(10)
                    continue
                else:
                    print e
                    return False
            time.sleep(20)
        if self.IsReady():
            print 'Connected'
            time.sleep(20)
            return True
        else:
            print 'Not connected'
            return False

    def Disconnect(self):
        self.svc.Disconnect()

    def Remove(self):
        self.svc.Remove()

    def MoveBefore(self, service):
        self.svc.MoveBefore(service)

    def MoveAfter(self, service):
        self.svc.MoveAfter(service)

    def IsReady(self):
        try:
            self.properties = self.svc.GetProperties()
        except dbus.DBusException, e:
            return False
        state = self.properties['State']
        return state == 'ready' or state == 'online' or state == 'login'

    # Basically we can get response through broadcast
    def BroadcastPing(self, size=0):
        count = 0
        while count < 10:
            ip = self.GetIP()
            if ip != None:
                break
            time.sleep(5)
            if count == 5:
                print "Try to connect again"
                try:
                    self.svc.Connect()
                except dbus.DBusException, e:
                    pass
                time.sleep(5)
            count += 1
        if ip == None:
            return False
        ip1 = ip.split('.')
        bip = ip1[0] + '.' + ip1[1] + '.' + ip1[2] + '.255'
        if size != 0:
            ping_string = 'ping -c 5 -s %s -b %s' % (size, bip)
        else:
            ping_string = 'ping -c 5 -b %s' % bip
        ret = commands.getstatusoutput(ping_string)
        print '%s  return %s' % (ping_string, ret[0])
        if ret[0] == 0:
            return True
        # Guess if there is a machine xxx.xxx.xxx.1
        bip = ip1[0] + '.' + ip1[1] + '.' + ip1[2] + '.1'
        if size != 0:
            ping_string = 'ping -c 5 -s %s -b %s' % (size, bip)
        else:
            ping_string = 'ping -c 5 -b %s' % bip
        ret = commands.getstatusoutput(ping_string)
        print '%s  return %s' % (ping_string, ret[0])
        return ret[0] == 0

    # If connected to Internet, we can always connect to www.intel.com
    def Ping(self, size=0, hostname='www.intel.com'):
        if size != 0:
            ping_string = 'ping -c 5 -s %s %s' % (size, hostname)
        else:
            ping_string = 'ping -c 5 %s' % hostname
        ret = commands.getstatusoutput(ping_string)
        print '%s  return %s' % (ping_string, ret[0])
        return ret[0] == 0

    def GetIP(self):
        self.properties = self.GetProperties()
        if self.properties == None:
            return None
        if len(self.properties['IPv4']) == 0:
            print 'Cannot get IPv4 for this device'
            return None
        return self.properties['IPv4']['Address']

    def HasIP(self):
        ip = self.GetIP()
        if ip == None:
            return False
        print 'IP address is %s' % ip
        return IPIsValid(ip)

    def IsDHCP(self):
        self.properties = self.GetProperties()
        if self.properties == None:
            return False
        method = self.properties['IPv4']['Method']
        print 'IPv4 Method of the service is %s' % method
        return method == 'dhcp'

    def IsFixed(self):
        self.properties = self.GetProperties()
        if self.properties == None:
            return False
        method = self.properties['IPv4']['Method']
        print 'IPv4 Method of the service is %s' % method
        return method == 'fixed'


###################################################
# ........Class Device
# The base class for Device, real device will
# derived from this class
###################################################


class Device:

    def __init__(self, name, nth):
        self.device = None
        self.intf = None
        for path in manager.properties['Technologies']:
            technology = manager.GetSubObject(path, 'Technology')
            properties = technology.GetProperties()
            if "Name" in properties.keys() and properties['Name'] != name:
                continue
            i = 1
            self.manager = manager
            self.tech = technology
            self.name = name
            self.type = properties['Type']
            print 'Got it: %s' % path
            manager.EnableTechnology(self.type)
            return
        print 'No such device!'

    def GetProperties(self):
        return self.tech.GetProperties()

    def SetProperty(self, name, value):
        print 'Set Device property %s = %s' % (name, value)
        self.tech.SetProperty(name, value)

    def GetService(self, servicename=None):
        properties = self.manager.GetProperties()
        for path in properties['Services']:
            service = dbus.Interface(self.manager.bus.get_object('net.connman'
                                   , path), 'net.connman.Service'
                                   )
            properties = service.GetProperties()
            if "Name" in properties.keys() and servicename!=None:
                if servicename != properties["Name"]:
                    continue
            else:
                if self.type != properties["Type"]:
                    continue
            svc = Service(service)
            print 'Service %s found' % path
            return svc
        
        print 'Service not found'
        return None

    def GetTechnology(self):
        return self.tech

    def GetInterface(self):
        if self.intf:
            return self.intf
        svc = self.GetService()
        props1 = svc.GetProperties()
        props = props1['Ethernet']
        print props
        key = 'Interface'
        if key in props.keys():
            self.intf = props['Interface']
            print 'There is the key Interface'
            return self.intf
        return None

    def ifconfigUP(self):
        intf = self.GetInterface()
        if intf == None:
            print 'The device interface name is empty, cannot ifconfig'
            return False
        cmd = 'ifconfig ' + intf + ' up'
        ret = commands.getstatusoutput(cmd)
        print '%s returns %s' % (cmd, ret[0])
        if ret[0] != 0:
            return False
        return True

    def ifconfigDown(self):
        intf = self.GetInterface()
        if intf == None:
            print 'The device interface name is empty, cannot ifconfig'
            return False
        cmd = 'ifconfig ' + intf + ' down'
        ret = commands.getstatusoutput(cmd)
        print '%s return %s' % (cmd, ret[0])
        if ret[0] != 0:
            return False
        return True

    def PoweredOff(self):
        '''Powered off device'''
        props = self.GetProperties()
        type = props['Type']
        print "Disable %s" % type
        manager.DisableTechnology(type)
        time.sleep(1)

    def Disable(self):
        self.PoweredOff()

    def PoweredOn(self):
        '''Powered on device'''

        props = self.GetProperties()
        type = props['Type']
        print "Enable %s" % type
        manager.EnableTechnology(type)
        time.sleep(1)

    def Enable(self):
        self.PoweredOn()

    def IsPoweredOff(self):
        properties = self.tech.GetProperties()
        print 'Device Powered state is %s' % properties['State']
        return properties['State'] in ["offline", "available", "blocked"]

    def IsDisabled(self):
        return self.IsPoweredOff()

    def IsExist(self):
        return self.tech != None

    def IsPoweredOn(self):
        properties = self.tech.GetProperties()
        print 'Device Powered state is %s' % properties['State']
        return properties['State'] in ["enabled", "connected"]

    def IsEnabled(self):
        return self.IsPoweredOn()

    def IsConnected(self):
        service = self.GetService()
        return service != None and service.BroadcastPing()

    def IsSCPWork(self):
        print 'scp to xfzheng'
        ret = \
            commands.getstatusoutput('scp test@xfzheng.sh.intel.com:1M /tmp/1M.1'
                )
        if ret[0] != 0:
            print 'scp failed'
            return False
        print 'scp success'
        print 'scp from xfzheng'
        ret = \
            commands.getstatusoutput('scp /tmp/1M.1 test@xfzheng.sh.intel.com:/tmp/1M.2'
                )
        if ret[0] != 0:
            print 'scp failed'
            return False
        print 'scp success'
        commands.getstatusoutput('ssh test@xfzheng.sh.intel.com rm /tmp/1M.2'
                                 )
        commands.getstatusoutput('rm /tmp/1M.1')
        return True

    def IsReEnableWorks(self):
        if self.IsEnabled():
            self.Disable()
        self.Enable()
        return self.IsEnabled()

    def Upload(self, cmd):
        print "Uploading file with %s" % (cmd)
        ret = commands.getstatusoutput(cmd)
        if ret[0] == 0:
            print "Upload success!"
        else:
            print "Upload failed!"
            print ret
        return ret[0] == 0

    def Download(self, cmd):
        print "Downloading file with %s" % (cmd)
        ret = commands.getstatusoutput(cmd)
        if ret[0] == 0:
            print "Download success!"
        else:
            print "Download failed!"
            print ret
        return ret[0] == 0

class EthDevice(Device):

    def __init__(self):
        Device.__init__(self, 'Wired', 1)
        svc = self.GetService()
        if svc == None:
            return
        svc.Connect()

    def Upload(self):
        return (Device.Upload(self, cm_eth_upload))

    def Download(self):
        return (Device.Download(self, cm_eth_download))


class WiFiDevice(Device):

    def __init__(self):
        Device.__init__(self, 'WiFi', 1)
        self.ssid=None

    def GetService(self):
        service = Device.GetService(self, self.ssid)
        if service == None:
            return service
        properties = service.GetProperties()
        if properties['Name']:
            print 'WiFi essid is %s' % properties['Name']
        else:
            print 'There is no essid'
        return service

    def Connect(
        self,
        ssid,
        passphrase='',
        security='none',
        ):
        self.ssid = ssid
        service = self.GetService()
        if service:
            properties = service.GetProperties()
            if ssid == properties['Name']:
                if properties['State'] in ['ready', 'online', 'login']:
                    print 'The service is already in state %s' \
                        % properties['State']
                    return True
        device_path = manager.ConnectService(ssid)
        time.sleep(10)
        #service = Device.GetService(self, ssid)
        service = self.GetService()
        properties = service.GetProperties()
        if properties['State'] in ['ready', 'online', 'login']:
            print 'state is %s' % properties['State']
            return True
        service.Connect()

        count = 0
        while count < 10:
            properties = service.GetProperties()
            if properties['State'] in ['ready', 'online', 'login']:
                print 'state is %s' % properties['State']
                return True
            count = count + 1
            time.sleep(5)
        print 'Current state is not ready'
        return False

    def Upload(self):
        return (Device.Upload(self, cm_wifi_upload))

    def Download(self):
        return (Device.Download(self, cm_wifi_download))


class C3GDevice(Device):

    def __init__(self):
        Device.__init__(self, '3G', 1)

    def GetService(self):
        props = manager.GetProperties()
        for path in props['Services']:
            service = \
                dbus.Interface(manager.bus.get_object('net.connman'
                               , path), 'net.connman.Service')

            properties = service.GetProperties()
            for key in properties.keys():
                if key == 'Mode' and properties['Mode'] == 'gprs':
                    svc = Service(service)
                    return svc
        return None

    def Connect(self):
        service = Device.GetService(self)
        if service == None:
            return False
        service.Connect()
        count = 0
        while count < 10:
            properties = service.GetProperties()
            if properties['State'] == 'online':
                print 'state is %s' % properties['State']
                return True
            count = count + 1
            time.sleep(5)
        return False

    def BroadcastPing(self, size=0):
        service = self.GetService()
        if service == None:
            return True
        loop = 0
        while loop < 5:
            state = service.GetProperty('state')
            if state == 'online':
                break
            time.sleep(10)
            loop = loop + 1
        return service.Ping()

    def IsConnected(self):
        return self.BroadcastPing()

    def Upload(self):
        return (Device.Upload(self, cm_3g_upload))

    def Download(self):
        return (Device.Download(self, cm_3g_download))


class BTDevice(Device):

    def __init__(self):
        Device.__init__(self, 'Bluetooth', 1)

    def Connect(self):
        service = Device.GetService(self)
        service.Connect()

    def Upload(self):
        return (Device.Upload(self, cm_bt_upload))

    def Download(self):
        return (Device.Download(self, cm_bt_download))

class WiFiGuestDevice(WiFiDevice):

    def __init__(self):
        WiFiDevice.__init__(self)
        print 'Now connect...'
        self.Enable()
        WiFiDevice.Connect(self, 'Guest')


key64 = '1111000000'
key128 = '11112222333344445555666611'
psk_key = 'sharedsecret'
manager = Manager()

