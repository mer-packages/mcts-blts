/* agentrelay.h This class relays all bluez dbus messages to our  test asset.
 *
 * This file is part of MCTS
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com;
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef _AGENTRELAY_H_
#define _AGENTRELAY_H_

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

class AgentRelay: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.bluez.Agent\" >\n"
"    <method name=\"Authorize\" >\n"
"       <arg type=\"o\" direction=\"in\" name=\"device\" >\n"
"       <arg type=\"s\" direction=\"in\" name=\"uuid\" >\n"
"    </method >\n"
"    <method name=\"Cancel\" />\n"
"    <method name=\"ConfirmationModeChange\" >\n"
"       <arg type=\"s\" direction=\"in\" name=\"mode\" >\n"
"    </method >\n"
"    <signal name=\"DisplayPasskey\" >\n"
"       <arg type=\"o\" direction=\"in\" name=\"device\" >\n"
"       <arg type=\"u\" direction=\"in\" name=\"passkey\" >\n"
"       <arg type=\"u\" direction=\"in\" name=\"entered\" >\n"
"    </method >\n"
"    <method name=\"Release\" />\n"
"    <signal name=\"RequestConfirmation\" >\n"
"       <arg type=\"o\" direction=\"in\" name=\"device\" >\n"
"       <arg type=\"u\" direction=\"in\" name=\"passkey\" >\n"
"    </method >\n"
"    <method name=\"RequestPasskey\" >\n"
"       <arg type=\"o\" direction=\"in\" name=\"device\" >\n"
"       <arg type=\"u\" direction=\"out\" >\n"
"    </method >\n"
"    <method name=\"RequestPinCode\" >\n"
"       <arg type=\"o\" direction=\"in\" name=\"device\" >\n"
"       <arg type=\"s\" direction=\"out\" >\n"
"    </method >\n"
"  </interface>\n"
        "")


public:
    AgentRelay(QObject *parent);
    virtual ~AgentRelay();

public slots:
    void Authorize(const QDBusObjectPath &device, const QString &uuid);
    void Cancel();
    void ConfirmModeChange(const QString &mode);
    void DisplayPasskey(const QDBusObjectPath &device, uint passkey, uint entered);
    void Release();
    void RequestConfirmation(const QDBusObjectPath &device, uint passkey);
    uint RequestPasskey(const QDBusObjectPath &device);
    QString RequestPinCode(const QDBusObjectPath &device);
};

#endif // _AGENTRELAY_H_
