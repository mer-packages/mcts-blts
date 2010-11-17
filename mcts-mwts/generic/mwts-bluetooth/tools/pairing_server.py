#!/usr/bin/python

# This is a script for putting up a pairing server.
# This is compatible with mwts-bluetooth asset,
# and it accepts all pairing attempts made by
# mwts-bluetooth asset.


import gobject

import sys
import dbus
import dbus.service
import dbus.mainloop.glib

class Agent(dbus.service.Object):
    
    @dbus.service.method("org.bluez.Agent",
                    in_signature="", out_signature="")
    def Release(self):
        print "Release"

    @dbus.service.method("org.bluez.Agent",
                    in_signature="os", out_signature="")
    def Authorize(self, device, uuid):
        print "Authorize (%s, %s)" % (device, uuid)

    @dbus.service.method("org.bluez.Agent",
                    in_signature="o", out_signature="s")
    def RequestPinCode(self, device):
        print "RequestPinCode (%s)" % (device)
        return "0000"

    @dbus.service.method("org.bluez.Agent",
                    in_signature="o", out_signature="u")
    def RequestPasskey(self, device):
        print "RequestPasskey (%s)" % (device)
        return dbus.UInt32("0000")

    @dbus.service.method("org.bluez.Agent",
                    in_signature="ou", out_signature="")
    def DisplayPasskey(self, device, passkey):
        print "DisplayPasskey (%s, %d)" % (device, passkey)

    @dbus.service.method("org.bluez.Agent",
                    in_signature="ou", out_signature="")
    def RequestConfirmation(self, device, passkey):
        print "RequestConfirmation (%s, %d)" % (device, passkey)

    @dbus.service.method("org.bluez.Agent",
                    in_signature="s", out_signature="")
    def ConfirmModeChange(self, mode):
        print "ConfirmModeChange (%s)" % (mode)

    @dbus.service.method("org.bluez.Agent",
                    in_signature="", out_signature="")
    def Cancel(self):
        print "Cancel"

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SystemBus()
    manager = dbus.Interface(bus.get_object("org.bluez", "/"),
                            "org.bluez.Manager")

    path = manager.DefaultAdapter()

    adapter = dbus.Interface(bus.get_object("org.bluez", path),
                            "org.bluez.Adapter")

    path = "/pairing/agent"
    agent = Agent(bus, path)

    mainloop = gobject.MainLoop()

    adapter.RegisterAgent(path, "DisplayYesNo")
    print "Agent registered"
    print "Server started"

    try:
        mainloop.run()
    except:
        agent.Cancel()
