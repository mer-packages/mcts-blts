/* BluetoothTest.h -- Header file for Bluetooth test class
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

#ifndef _INCLUDED_BLUETOOTH_TEST_H
#define _INCLUDED_BLUETOOTH_TEST_H

#include <stable.h>
#include "BluetoothSocket.h"
#include "BluetoothObex.h"
#include "BluetoothApiInterface.h"
#include "BluetoothCommonTypes.h"

/**
 * Bluetooth test class
 * Inherited from MwtsTest class
 */
class BluetoothTest : public MwtsTest
{
    Q_OBJECT
public:
    BluetoothTest(QObject* parent = 0);
    virtual ~BluetoothTest();

    void OnInitialize();
    void OnUninitialize();

    int SetApi(int api);
    bool SetScanMode(BluetoothApiInterface::ScanMode mode);
    bool SetPowerMode(bool isOn);
    bool Scan();
    int Transfer(ServiceRole role,
                TransferType type = L2CAP,
                char* host = NULL,
                long int bytes =-1,
                int buffsize = -1,
                int time = -1 );
    int Device(int command, char* host = 0, char* pin = 0, bool isClient=true,bool isHeadset=false);
    int FileTransfer(char* filename, char* host = 0, ServiceRole role = Client);
    const QString& GetAddress() const;

    /**
    * @brief overwritten function from MwtsTest class. To stop printing.
    */
    inline virtual void OnIdle() {};

public slots:

    void transferStopped();
    void periodicThroughput(double sampleTime, double bytesPerSecond);
    void transferFinished(double bytesPerSecond,bool isSuccessfull);
    void transferError(QString error);
    void transferWarning(QString warning);

private:

    // Api flag
    bool    m_bIsDbus;

    // Sockets/API's
    BluetoothSocket*         m_pSocket;
    BluetoothObex*			 m_pObex;
    BluetoothApiInterface*   m_pBtApi;
};

#endif // _INCLUDED_BLUETOOTH_TEST_H
