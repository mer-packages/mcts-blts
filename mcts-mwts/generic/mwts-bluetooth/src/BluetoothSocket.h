/* BluetoothSocket.h -- Header file for Bluetooth socket tests
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

#ifndef BLUETOOTHSOCKET_H_
#define BLUETOOTHSOCKET_H_

#include <stable.h>
#include <QTimer>
#include <QThread>
#include <QCryptographicHash>
#include "BluetoothCommonTypes.h"


class BluetoothSocket : public QThread

{
    Q_OBJECT
public:
    BluetoothSocket(QObject* parent=0);
    virtual ~BluetoothSocket();

    int CreateClient();
    int CreateServer(TransferType type);
    int Transfer( ServiceRole role,
                  TransferType type = L2CAP,
                  char* host = NULL,
                  long int bytes =-1,
                  int buffsize = -1,
                  int time = -1 );
    void Close();
    void CreateBuffer(int buffize);
    int ConvertSocketyType(QString type);
    bool runTransfer();
    int Read();
    int Write();
    void run();
    bool CompareChecksum();

public slots:
    void Timeout();

signals:

    /**
     * Callback, when server has closed server port and is finished its work
     */
    void transferStopped();
    /**
     * A critical error occurred. Program cannot continue.
     */
    void transferError(QString error);
    /**
    *   An error occurred. Program may continue.
    */
    void transferWarning(QString warning);
    /**
     * Returns receive througput periodically about second divisions
     * @param sampleTime        time fron start of transfer
     * @param bytesPerSecond    transfer throughput, calculated from perious callback time
     */
    void periodicThroughput(double sampleTime, double bytesPerSecond);
    /**
     * Returns receive total througput
     * @param bytesPerSecond        transfer total throughput
     * @param isSuccesfull          were case succesfull
     */
    void transferFinished(double bytesPerSecond,bool isSuccesfull);

private:
    QTimer*     	m_pTimer;                ///< timeout timer

    // parameter for threads to run
    ServiceRole  	m_enumRole;              ///< the role. eg. clent, host
    TransferType	m_SocketType;            ///< socket type. eg. L2CAP, RfComm
    QString*    	m_phost;                 ///< mac adress of the host
    long int    	m_bytes;                 ///< amount of bytes to be sent
    int         	m_buffsize;              ///< the size of sending buffer
    long int    	m_sendtime;              ///< the time to send. in LOLA cases
    QTime       	m_time;                  ///< timer. for periodic samples, timeout, and LOLA sending


    long int    	m_cBytes_transfered;     ///< bytes already transferred
    long int    	m_cBytes_left;           ///< bytes to be transferred
    char*       	m_cBuffer;               ///< transfer buffer
    int         	m_iSocket;               ///< socket used in transfer

    bool            m_bTimeoutOccurred;      ///< flag telling if timeout has occurred

    QCryptographicHash* m_checksum;      	///< checksum of all bytes transferred

};

#endif /* BLUETOOTHSOCKET_H_ */
