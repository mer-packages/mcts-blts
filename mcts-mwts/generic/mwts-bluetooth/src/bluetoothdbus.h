/* bluetoothdbus.h -- Header file for Bluetooth Dbus test class
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

#ifndef BLUETOOTHDBUS_H
#define BLUETOOTHDBUS_H

#include "BluetoothApiInterface.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

class Agent;

class BluetoothDbus: public BluetoothApiInterface
{
    Q_OBJECT
public:
    BluetoothDbus(QObject* parent=0);
    ~BluetoothDbus();

    bool Init();
    bool Uninit();

    virtual bool Connect();
    virtual bool Disconnect();
    virtual bool Remove();

    virtual bool SetScanMode(ScanMode mode);
    virtual bool SetPowerMode(bool isOn);
    virtual bool Scan();

    virtual QString& GetAddress();

public slots:
    void PropertyChanged(QString,QDBusVariant);
    void DeviceFound(QString,QVariantMap);
    void Timeout();

private:
    QString pair_devices();
    QString has_device();
    QDBusInterface* create_interface(const QString& type, const QString& path);
    bool    is_connected();

    QDBusInterface*     m_pManager;         ///< interface instance of bt manager
    QDBusInterface*     m_pAdapter;         ///< interface instance of bt adapter
    QDBusInterface*     m_pDevice;          ///< interface instance of bt device
    QDBusInterface*     m_pAudio;           ///< interface instance of bt audio
    Agent*              m_pAgent;           ///< agent pointer

    QEventLoop*         m_pEventLoop;       ///< instance of a Qt EventLoop
};

#endif // BLUETOOTHDBUS_H
