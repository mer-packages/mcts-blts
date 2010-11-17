/* BluetoothApiInterface.h -- Header file for Bluetooth Hci test class
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

#ifndef __BLUETOOTHHCI_H__
#define __BLUETOOTHHCI_H__

#include "BluetoothApiInterface.h"
#include "stable.h"

class BluetoothHci : public BluetoothApiInterface
{
Q_OBJECT
public:
    explicit BluetoothHci(QObject *parent = 0);
    ~BluetoothHci();

    virtual bool Init();
    virtual bool Uninit();

    bool SetScanMode(ScanMode mode);
    bool SetPowerMode(bool isOn);
    int Inquiry(inquiry_info** inq_info);
    bool Scan();

    virtual void SetSettings(const QString& host, const QString& pin, bool isClient,bool isHeadset);
    virtual bool Connect();
    virtual bool Disconnect();
    virtual bool Remove();

private:
    int         m_iDevID;           ///< device id
    int         m_iDevSock;         ///< device socket
    bdaddr_t*   m_pBdaddr;          ///< bluetooth address struct pointer
};

#endif // __BLUETOOTHHCI_H__
