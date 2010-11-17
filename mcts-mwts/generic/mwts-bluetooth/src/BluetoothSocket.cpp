/* BluetoothSocket.cpp -- Source file for Bluetooth socket tests
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

#include "BluetoothSocket.h"
// Tests pipes need
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PERIODIC_SAMPLE_TIME 1000        //milliseconds
#define MAC_KEYWORD "MAC"

/**
 * Constructor, initialize variables
 */
BluetoothSocket::BluetoothSocket(QObject* parent)
: QThread(parent),
  m_cBuffer(NULL),
  m_phost(NULL),
  m_iSocket(0)
{
    MWTS_ENTER;
    m_pTimer = new QTimer(this);
    bool tmp = this->connect(m_pTimer, SIGNAL(timeout()), this, SLOT(Timeout()));
    qDebug() << "Timeout connected: " << tmp;

    m_cBuffer = NULL;
    m_checksum = new QCryptographicHash(QCryptographicHash::Md5);

    MWTS_LEAVE;
}

/**
 * Destructor, uninitialize variables
 */
BluetoothSocket::~BluetoothSocket()
{
    MWTS_ENTER;

    qDebug() << "Terminate thread to be nice";
    terminate();

    qDebug() << "Wait for main thread to complete";
    wait();
    
    qDebug() << "BluetoothSocket::~BluetoothSocket this->Close()";
    this->Close();

    if(m_cBuffer)
    {
        qDebug() << "BluetoothSocket::~BluetoothSocket delete m_cBuffer";
        delete m_cBuffer;
        m_cBuffer = NULL;
    }

    if (m_phost)
    {
        qDebug() << "BluetoothSocket::~BluetoothSocket delete m_phost";
        delete m_phost;
        m_phost = NULL;
    }

    m_pTimer->stop();
    delete m_pTimer;
    m_pTimer = 0;

    delete m_checksum;

    MWTS_LEAVE;
}

/**
 * Create Bluetooth client socket for given type
 * @return      Socket descriptor, or -1 if error
 */
int BluetoothSocket::CreateClient()
{
    MWTS_ENTER;

    m_enumRole = Client;

    QByteArray ba = m_phost->toLatin1();
    const char *host = ba.data();

      struct sockaddr local_address = {0};
      int ret = 0, btproto = 0, sock_type = 0;

      qDebug() << "Socket type == " << m_SocketType;

      // Set protocol settings
      switch(m_SocketType)
      {
          case L2CAP:
          {
              qDebug() << "Using L2CAP protocol";

              struct sockaddr_l2 l2cap_address = {0};
              local_address = *((struct sockaddr *) &l2cap_address);
              btproto = BTPROTO_L2CAP;
              qDebug() << "config L2CAP/sock_type" << g_pConfig->value("L2CAP/sock_type").toString();
              sock_type = ConvertSocketyType(g_pConfig->value("L2CAP/sock_type").toString());
              l2cap_address.l2_family = AF_BLUETOOTH;
              qDebug() << "config L2CAP/psm" << g_pConfig->value("L2CAP/psm").toInt();
              l2cap_address.l2_psm = htobs(g_pConfig->value("L2CAP/psm").toInt());
              /* Copy MAC address */
              str2ba( host, &l2cap_address.l2_bdaddr);
              local_address = *((struct sockaddr *) &l2cap_address);
              break;
          }
          case RfComm:
          {
              struct sockaddr_rc rfcomm_adress = {0};
              qDebug() << "Using RfComm protocol";
              btproto = BTPROTO_RFCOMM;
              qDebug() << "Rfcomm sock_type: " << g_pConfig->value("RFCOMM/sock_type").toString();
              sock_type = ConvertSocketyType(g_pConfig->value("RFCOMM/sock_type").toString());
              rfcomm_adress.rc_family = AF_BLUETOOTH;
              qDebug() << "Rfcomm channel: " <<(uchar) g_pConfig->value("RFCOMM/rc_channel").toInt();
              rfcomm_adress.rc_channel = (uchar) g_pConfig->value("RFCOMM/rc_channel").toInt();
              /* Copy MAC address */
              str2ba( host, &rfcomm_adress.rc_bdaddr);
              local_address = *((struct sockaddr *) &rfcomm_adress);
              break;
          }
          default:
          {
              emit transferError("Invalid protocol " + m_SocketType);
              qCritical() << "Invalid protocol " << m_SocketType;
              return -1;
          }
      }

      //check if m_iSocket is in use.
      if(m_iSocket > 0)
          Close();

      // Allocate socket for bluetooth with specified protocol
      m_iSocket = ::socket(AF_BLUETOOTH, sock_type, btproto);
      qDebug() << "socket returned " << m_iSocket;

      // Connect to MAC address
      ret = ::connect(m_iSocket, &local_address, sizeof(local_address));
      qDebug() << "connect returned " << ret;


      // Check connection failed
      if(ret < 0)
      {
          this->Close();
          emit transferError("Connection failed, Error: "+ QString(strerror(errno)));
          qCritical() << "Connection failed, Error: " << strerror(errno);
          errno = 0;
          return -1;
      }

      MWTS_LEAVE;
      return m_iSocket;
}


/**
 * Create Bluetooth server socket for given type
 * @param type  Socket type
 * @return      Socket descriptor, or -1 if error
 */
int BluetoothSocket::CreateServer(TransferType type)
{
    MWTS_ENTER;

    m_enumRole = Server;

    int serverSocket = 0;
    struct sockaddr local_address = {0}, remote_address = {0};
    struct sockaddr_l2 l2cap_address = {0};
    struct sockaddr_rc rfcomm_address = {0};
    int ret = 0, btproto = 0, sock_type = 0;
    char buf[64] = { 0 };

    // Set protocol settings
    if(type == L2CAP)
    {
        qDebug() << "Using L2CAP protocol";

        btproto = BTPROTO_L2CAP;
        qDebug() << "config L2CAP/sock_type" << g_pConfig->value("L2CAP/sock_type").toString();
        sock_type = ConvertSocketyType(g_pConfig->value("L2CAP/sock_type").toString());
        l2cap_address.l2_family = AF_BLUETOOTH;
        qDebug() << "config L2CAP/psm" << g_pConfig->value("L2CAP/psm").toInt();
        l2cap_address.l2_psm = htobs(g_pConfig->value("L2CAP/psm").toInt());
        l2cap_address.l2_bdaddr = *BDADDR_ANY;
        local_address = *((struct sockaddr *) &l2cap_address);
    }
    else if(type == RfComm)
    {
        qDebug() << "Using RfComm protocol";
        btproto = BTPROTO_RFCOMM;
        qDebug() << "Rfcomm sock_type: " << g_pConfig->value("RFCOMM/sock_type").toString();
        sock_type = ConvertSocketyType(g_pConfig->value("RFCOMM/sock_type").toString());
        rfcomm_address.rc_family = AF_BLUETOOTH;
        qDebug() << "Rfcomm channel: " <<(uchar) g_pConfig->value("RFCOMM/rc_channel").toInt();
        rfcomm_address.rc_channel = (uchar)g_pConfig->value("RFCOMM/rc_channel").toInt();
        rfcomm_address.rc_bdaddr = *BDADDR_ANY;
        local_address = *((struct sockaddr *) &rfcomm_address);
    }
    else
    {
        emit transferError("Invalid protocol " + type);
        qCritical() << "Invalid protocol " << type;
        return -1;
    }
    socklen_t opt = sizeof(remote_address);

    //check if m_iSocket is in use.
    if(m_iSocket > 0)
        Close();


    // Allocate socket for bluetooth with specified protocol
    serverSocket = ::socket(AF_BLUETOOTH, sock_type, btproto);
    qDebug() << "socket returned " << serverSocket;

    // Bind server socket
    ret = ::bind(serverSocket, &local_address, sizeof(local_address));
    qDebug() << "bind returned " << ret;

    if(ret < 0)
    {
        this->Close();
        emit transferError(QString("bind command failed. Error: ") + QString(strerror(errno)));
        qCritical() << "bind command failed. Error: " << strerror(errno);
        errno = 0;
        return -1;
    }

    // Change socket into listening mode
    ret = ::listen(serverSocket, 1);
    qDebug() << "listen returned " << ret;

    if(ret < 0)
    {
        this->Close();
        emit transferError(QString("listen command failed. Error: ") + QString(strerror(errno)));
        qCritical() << "listen command failed Error: " << strerror(errno);
        errno = 0;
        return -1;
    }

    // Init timeout
    qDebug() << "Test timeout: "<<TEST_TIMEOUT;
    m_pTimer->start(TEST_TIMEOUT);

    // Accept one connection
    qDebug() << "do accept";
    m_iSocket = accept(serverSocket, &remote_address, &opt);
    qDebug() << "accept returned " << m_iSocket;

    // Stop timeout
    m_pTimer->stop();

    // Close server socket
    ::close(serverSocket);

    if(m_iSocket < 0)
    {
        this->Close();
        emit transferError(QString("accept command failed. Error: ") + QString(strerror(errno)));
        qCritical() << "accept command failed. Error: " << strerror(errno);
        errno = 0;
        return -1;
    }

    // Get remote address
    if(type == L2CAP)
    {
        struct sockaddr_l2 remote_l2cap;
        remote_l2cap = *((struct sockaddr_l2 *) &remote_address);
        ba2str(&remote_l2cap.l2_bdaddr, buf);
    }

    QString log("accepted connection from ");
    log.append(buf);

    qDebug() << log;

    g_pResult->Write(log);

    MWTS_LEAVE;
    return m_iSocket;
}


int BluetoothSocket::Transfer(ServiceRole role,
                              TransferType type/* = L2CAP*/,
                              char* host /*= NULL*/,
                              long int bytes /*=-1*/,
                              int buffsize /*= -1*/,
                              int time /* = -1*/ )
{
    MWTS_ENTER;
    int sent = 1;

    qDebug() << "BluetoothSocket::Transfer";

    /* Init role */
    m_enumRole = role;
    /* Init type */
    m_SocketType = type;

    /* delete old host */
    delete m_phost;
    m_phost = NULL;

    /* Create the host/mac adress */
    if (host == NULL)
    {
        qDebug() << "host_mac" << g_pConfig->value("host_mac").toString();
        m_phost = new QString(g_pConfig->value("host_mac").toString());
    }
    else
    {
        /* check if equals to any key available in .conf file */
        /* load mac addresses from conf file */
        qDebug() << "Load mac addresses";
        g_pConfig->beginGroup(MAC_KEYWORD);
        QStringList keys = g_pConfig->childKeys();
        qDebug() << "Found"<<keys.length()<<"keys";
        qDebug() << "Find compatible partner for key:"<<host;
        foreach(QString str, keys)
        {
            qDebug() << "Found key:"<<str;
            if(str.compare(QString(host))==0)
            {
                QString value = g_pConfig->value(str).toString();
                qDebug() << "Using key:"<<str<<"with value"<<value;
                m_phost = new QString(value);
            }
        }
        g_pConfig->endGroup();
        if(m_phost->isEmpty())
            m_phost = new QString(host);
        qDebug() << "m_phost" << *m_phost;
    }

    /* Init the bytes to be sent */
    if (bytes <  0L)
    {
        qDebug() << "config FILE/file_size" << g_pConfig->value("FILE/file_size").toLongLong();
        m_bytes = g_pConfig->value("FILE/file_size").toLongLong();
    }
    else
    {
        m_bytes = bytes ;
        qDebug() << "m_bytes" << m_bytes;
    }

    /* Init the buffsize */
    if (buffsize <  0L)
    {
        qDebug() << "config FILE/mtu_size" << g_pConfig->value("FILE/mtu_size").toInt();
        m_buffsize = g_pConfig->value("FILE/mtu_size").toInt();
    }
    else
    {
        m_buffsize= buffsize;
        qDebug() << "m_buffsize" << m_buffsize;
    }

    /* init LOLA send time */
    qDebug() << "m_sendtime" << time;
    m_sendtime = time;

    /* init transfer variables */
    m_cBytes_left = bytes;
    m_cBytes_transfered = 0;

    qDebug() << "call BluetoothSocket::start()";
    start();

    return sent;
}

/**
 * Close sockets
 */
void BluetoothSocket::Close()
{
    MWTS_ENTER;

    // Close connection
    if(m_iSocket > 0)
    {
        qDebug() << "Closing socket:"<<m_iSocket;
        int ret = close(m_iSocket);
        qDebug() << "Close returned:"<<ret;
        if(ret >= 0)
            m_iSocket = 0;
    }

    MWTS_LEAVE;
}

/**
 * Creates a test file to send to server.
 *
 */
void BluetoothSocket::CreateBuffer(int iSize)
{
    MWTS_ENTER;
    int i = 0;
    char c;

    if(m_cBuffer)
    {
        delete (m_cBuffer);
        m_cBuffer = NULL;
    }

    m_cBuffer = new char[iSize];

    while (i < iSize )
    {
        c = (48+rand()%42);
        m_cBuffer[i] = c;
        i++;
    }

    MWTS_LEAVE;
}

/**
 * Converts QString socket type to BlueZ socket type
 * SOCK_SEQPACKET, SOCK_STREAM, SOCK_RDM, SOCK_DCCP or SOCK_DGRAM
 * @param type	Type in string
 * @return		Type in int, or -1 for error
 */

int BluetoothSocket::ConvertSocketyType(QString type)
{
    MWTS_ENTER;
    int intType = -1;

    if (type == "SOCK_SEQPACKET")
    {
        intType = SOCK_SEQPACKET;
    }
    else if (type == "SOCK_STREAM")
    {
        intType = SOCK_STREAM;
    }
    else if (type == "SOCK_RDM")
    {
        intType = SOCK_RDM;
    }
    else if (type == "SOCK_DGRAM")
    {
        intType = SOCK_DGRAM;
    }
    else
    {
        emit transferError("Invalid socket type");
        qCritical() << "Invalid socket type";
    }
    MWTS_LEAVE;
    return intType;
}

/**
 * write socket untill get amount of bytes is sent
 */
bool BluetoothSocket::runTransfer()
{
    MWTS_ENTER;
    bool ret = true;
    qDebug() << "Start transfering data";

    /* init periodicThroughput variables */
    double previous_time = m_time.elapsed();
    long int previous_bytes_transferred = 0L;

    /* init buffer variables */
    m_cBytes_left = m_bytes;
    m_cBytes_transfered = 0L;

    /* init timeout */
    qDebug() << "Transfer timeout:"<<TEST_TIMEOUT;
    m_pTimer->start(TEST_TIMEOUT);

    qDebug() << "Transfer ongoing";

    /* Run loop for file transfer */
    while (m_cBytes_left > 0L)
    {
        /* Call the transfer functions */
        int ret = 0;
        if(m_enumRole == Client)
        {
            ret = Write();
        }
        else
        {
            ret = Read();
        }
//        qDebug() << m_cBytes_transfered << " of " << m_bytes << " transferred.";

        /* check for error */
        if (ret < 0)
        {
            emit transferWarning("Error while transfering using socket " +  QString(strerror(errno)) );
            qWarning() <<  "Error while transfering using socket: " <<  strerror(errno) << errno;
            return false;
        }
        /* update transfer values */
        else
        {
            m_cBytes_transfered += ret;
            m_cBytes_left -= ret;
            //update checksum
            m_checksum->addData(m_cBuffer,ret);
        }

        /* check for periodic sample */
        if((m_time.elapsed() - previous_time) > PERIODIC_SAMPLE_TIME)
        {
            /* emit periodicThroughput*/
//            emit periodicThroughput(m_time.elapsed()/1000.0,
//                                    ((double) m_cBytes_transfered - (double) previous_bytes_transferred)
//                                    /((m_time.elapsed() - previous_time) / 1000.0));
//        	qDebug() << "PERIODIC THROUGHPUT!";

            /* check that something has been transferred */
            if(m_cBytes_transfered <= previous_bytes_transferred)
            {
                emit transferWarning("Error, no bytes transferred.");
                qWarning() <<  "Error, no bytes transferred.";
                return false;
            }

            /* restart timeout */
            m_pTimer->start(TEST_TIMEOUT);

            /* update the periodic variables */
            previous_bytes_transferred = m_cBytes_transfered;
            previous_time = m_time.elapsed();
        }
    }

    /* stop timeout */
    qDebug() << "Stop timeout";
    m_pTimer->stop();

    /* check that bytes were transferred */
    if(m_cBytes_transfered < m_bytes)
    {
        emit transferWarning("Not all data were transferred.");
        qWarning() <<  "Not all data were transferred. " << m_cBytes_transfered << " of " << m_bytes;
        return false;
    }

    MWTS_LEAVE;
    return ret;
}

/**
* Read socket
*/
int BluetoothSocket::Read()
{
    int ret = 0;
    /* read bytes from socket */
    ret = ::read(m_iSocket, m_cBuffer , m_buffsize);
//    qDebug() << ret << "bytes read";
    return ret;
}

/**
* Write socket
*/
int BluetoothSocket::Write()
{
    int ret = 0;
//    qDebug() << "Write";
    /* write bytes into socket */
    ret = ::write(m_iSocket, m_cBuffer, m_buffsize);
//    qDebug() << ret << "bytes written";
    return ret;
}

/**
 * Run as thread
 */
void BluetoothSocket::run()
{
    MWTS_ENTER;
    bool bret = true;
    double bytesPerSecond = 0;
    int bytesTransferedTotal = 0;

    /* Different function calls for different roles */
    if (m_enumRole == Client)
    {
        /* Create client */
        qDebug() << "BluetoothSocket::run do CreateClient()";
        if(!(CreateClient() > 0))
        {
            emit transferError("Cannot create client");
            qCritical() << "Cannot create client";
            bret = false;
        }
    }
    else
    {
        qDebug() << "BluetoothSocket:: run do CreateServer";
        if(CreateServer(m_SocketType) <0)
        {
            emit transferError("Cannot create server");
            qCritical() << "Cannot create server";
            bret = false;
        }
    }

    /* start time */
    m_time.start();
    /* reset checksum */
    m_checksum->reset();

    /* do until time is reached (for LOLA) */
    do
    {
        /* Create Buffer */
        if(bret) CreateBuffer(m_buffsize);
        /* run transfer */
        qDebug() << "BluetoothSocket::run do runTransfer";
        if(bret) bret = runTransfer();
        bytesTransferedTotal += m_cBytes_transfered;
    }
    while((m_time.elapsed() / 1000.0) < m_sendtime && bret);

    /* calculate transfer speed */
    /* bytes transferred divided by time gives the total transfer rate. */
    bytesPerSecond = (double) bytesTransferedTotal/ ((double) m_time.elapsed() / 1000.0 );

    /* compare checksums */
    if(bret) bret = CompareChecksum();

    /* close sockets */
    Close();

    /* finish transfer */
    qDebug() << "BluetoothSocket::run do emit transferFinished";
    emit transferFinished(bytesPerSecond,bret);

    /* signal that transfer stopped */
    qDebug() << "BluetoothSocket::run emit bluetooth test Stopped()";
    emit transferStopped();

    MWTS_LEAVE;
}

/**
 * Compares the checksums
 * @return bool res, returns true if compare was succesfull
 */
bool BluetoothSocket::CompareChecksum()
{
    MWTS_ENTER;

    //swap roles
    (m_enumRole == Client)?(m_enumRole = Server):(m_enumRole = Client);
    qDebug() << "Role swapped to "<< ((m_enumRole == Client)?("Client"):("Server"));

    m_bytes = m_checksum->result().length()+1;
    qDebug() << "Checksum length == " << m_bytes;
    m_buffsize = m_bytes;
    QByteArray temp = m_checksum->result();
    qDebug() << "Checksum: " << temp;

    //set buffer
    delete (m_cBuffer);
    m_cBuffer = NULL;
    m_cBuffer = new char[m_bytes];
    memset(m_cBuffer,0,m_bytes);
    if(m_enumRole == Client)
        memcpy(m_cBuffer, m_checksum->result().data(),m_bytes);

    runTransfer();

    //compare checksum
    QByteArray tmp;
    if(strcmp(m_cBuffer,temp.data()) == 0)
    {
        tmp.append("OK");
    }
    else
        tmp.append("NO");

    qDebug() << "Checksum == " << tmp;

    //swap roles
    (m_enumRole == Client)?(m_enumRole = Server):(m_enumRole = Client);
    qDebug() << "Role swapped to "<< ((m_enumRole == Client)?("Client"):("Server"));

    //set buffer
    m_bytes = QByteArray("MM").length()+1;
    m_buffsize = m_bytes;
    delete m_cBuffer;
    m_cBuffer = NULL;
    m_cBuffer = new char[m_bytes];
    memset(m_cBuffer,0,m_bytes);
    if(m_enumRole == Client)
        memcpy(m_cBuffer,tmp.data(),m_bytes);
    qDebug() << "Transfer message";

    runTransfer();

    //compare checksum messages
    if(strcmp(m_cBuffer,"OK") != 0)
    {
        emit transferWarning("Checksums were different");
        qWarning() << "Checksums were different";
        return false;
    }
    else
        qDebug() << "Checksum message == " << m_cBuffer;

    MWTS_LEAVE;
    return true;
}

/**
 * Handle timeout signal
 */
void BluetoothSocket::Timeout()
{
    MWTS_ENTER;

    this->Close();

    emit transferError("Got timeout signal, closing sockets");
    qCritical() << "Got timeout signal, closing sockets";

    //wait for main thread to complete
    wait();

    MWTS_LEAVE;
}
