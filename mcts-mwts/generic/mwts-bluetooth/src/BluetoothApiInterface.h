/* BluetoothApiInterface.h -- Header file for Bluetooth Api Interface
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

#ifndef BLUETOOTHAPIINTERFACE_H
#define BLUETOOTHAPIINTERFACE_H

#include <stable.h>

class BluetoothApiInterface: public QObject
{
    Q_OBJECT
public:
    // Scan mode
    enum ScanMode {
        ScanModeInvisible = SCAN_DISABLED,
        ScanModeInquiry = SCAN_INQUIRY,
        ScanModePage = SCAN_PAGE,
        ScanModeBoth = SCAN_PAGE | SCAN_INQUIRY
    };

    explicit BluetoothApiInterface(QObject* parent =0);
    virtual ~BluetoothApiInterface() {}

    virtual bool Init();
    virtual bool Uninit();

    virtual QString& GetAddress();

    virtual void SetSettings(const QString& host, const QString& pin, bool isClient, bool isHeadset);
    virtual bool Connect() =0;
    virtual bool Disconnect() =0;
    virtual bool Remove() =0;

    virtual bool SetScanMode(ScanMode mode) =0;
    virtual bool SetPowerMode(bool isOn) =0;
    virtual bool Scan()=0;

protected:    
    QString     m_sPin;             ///< pin code for pairing
    QString     m_sHost;            ///< host mac
    bool        m_bIsClient;        ///< pairing role
    bool        m_bIsHeadset;       ///< is device a headset
    QString     m_sAddress;         ///< our mac
};

#endif // BLUETOOTHAPIINTERFACE_H
