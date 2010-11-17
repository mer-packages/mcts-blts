#!/usr/bin/python

# This is a script for putting up a obex server.
# This is compatible with mwts-bluetooth asset,
# and it accepts all file transfer attempts made by
# mwts-bluetooth asset.
#
# Parameter: arg1 is the folder which to use as 
#            file storage. (ensure permissions)

import gobject

import sys
import os
import dbus
import dbus.service
import dbus.mainloop.glib

class Agent(dbus.service.Object):
    def __init__(self, conn=None, obj_path=None):
        dbus.service.Object.__init__(self, conn, obj_path)
        self.index = 0

    @dbus.service.method("org.openobex.Agent",
                    in_signature="osssii", out_signature="s")
    def Authorize(self, dpath, device, filename, ftype, length, time):
        global transfers
        global folder_path
        global files
        self.index = self.index + 1
        print "Authorize (%s, %s, %s)" % (path, device, filename)
        transfers.append(Transfer(dpath, filename, 0, length))
        filepath = folder_path + "/" +filename + str(self.index)
        print "creating file %s" % filepath
        files.append(filepath)
        return filepath

    @dbus.service.method("org.openobex.Agent",
                    in_signature="", out_signature="")
    def Cancel(self):
        print "Authorization Canceled"
        self.pending_auth = False


class Transfer(object):
    def __init__(self, dpath, filename=None, transfered=-1, size=-1):
        self.dpath = dpath
        self.filename = filename
        self.transfered = transfered
        self.size = size

    def update(self, filename=None, transfered=-1, total=-1):
        if filename:
            self.filename = filename
        self.transfered = transfered
        self.size = total

    def cancel(self):
        transfer_iface = dbus.Interface(bus.get_object("org.openobex",
                                                        self.dpath),
                                        "org.openobex.Transfer")
        transfer_iface.Cancel()


if __name__ == '__main__':
    global transfers
    global folder_path
    global files

    def transfer_completed(dpath, success):
        global transfers
        print "\ntransfer completed => %s" % (success and "Success" or "Fail")
        transfers = [t for t in transfers if t.dpath != dpath]
        
    def remove_files():
        global files
        for f in files:
            os.remove(f)
        files = []

    if(len(sys.argv) < 1):
        print "no folder path defined"
        sys.exit(1)
    else:
        folder_path = os.path.realpath(sys.argv[1])
        print "using folder %s" % (folder_path)

    transfers = []
    files = []

    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    manager = dbus.Interface(bus.get_object("org.openobex", "/"),
                            "org.openobex.Manager")

    bus.add_signal_receiver(transfer_completed,
                            dbus_interface="org.openobex.Manager",
                            signal_name="TransferCompleted")

    path = "/obex/agent"
    agent = Agent(bus, path)

    mainloop = gobject.MainLoop()

    manager.RegisterAgent(path)
    print "Agent registered"
    print "Server started"

    try:
        mainloop.run()
    except:
        print "cleaning up"
        if len(transfers) > 0:
            for a in transfers:
                a.cancel()
        if len(files) > 0:            
            remove_files()

