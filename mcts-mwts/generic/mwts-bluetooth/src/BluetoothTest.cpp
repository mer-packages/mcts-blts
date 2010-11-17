/* BluetoothTest.cpp -- Source code for Bluetooth test class
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

#include "stable.h"
#include "BluetoothTest.h"
#include "bluetoothdbus.h"
#include "bluetoothhci.h"

/**
 * Constructor for Bluetooth test class
 */
BluetoothTest::BluetoothTest(QObject* parent)
    : MwtsTest(parent), m_pSocket(0),m_pBtApi(0), m_bIsDbus(true), m_pObex(0)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

/**
 * Destructor for Bluetooth test class
 */
BluetoothTest::~BluetoothTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void BluetoothTest::OnInitialize()
{
    MWTS_ENTER;

    // Enable debug
    g_pLog->EnableDebug(true);

    // Init Sockets
    m_pSocket = new BluetoothSocket(this);
    m_pObex = new BluetoothObex(this);

    /* connect slots and signals */
    connect(m_pSocket,
            SIGNAL(periodicThroughput(double, double)),
            this,
            SLOT(periodicThroughput(double, double)));
    connect(m_pSocket,
            SIGNAL(transferFinished(double,bool)),
            this,
            SLOT(transferFinished(double,bool)));
    connect(m_pSocket,
            SIGNAL(transferStopped()),
            this,
            SLOT(transferStopped()));
    connect(m_pSocket,
            SIGNAL(transferError(QString)),
            this,
            SLOT(transferError(QString)));
    connect(m_pSocket,
            SIGNAL(transferWarning(QString)),
            this,
            SLOT(transferWarning(QString)));
    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void BluetoothTest::OnUninitialize()
{
    MWTS_ENTER;

    delete m_pBtApi;
    m_pBtApi = 0;

    if(m_pObex)
    {
    	delete m_pObex;
    }

    if(m_pSocket)
    {
        if(m_pSocket->isRunning())
            qCritical() << "The socket thread were left running";
        delete m_pSocket;
        m_pSocket = NULL;
    }
    MWTS_LEAVE;
}

/**
*   Sets which API is in use.
*   Init all Api specific things.
*   @param  int api, which api to use.
*   @return int result, whether init were succesfull.
*/
int BluetoothTest::SetApi(int api)
{
    MWTS_ENTER;
    bool bret = true;

    m_bIsDbus = api;    // int-to-bool conversion

    if(m_bIsDbus)   //DBUS
    {
        m_pBtApi = new BluetoothDbus(this);
        if (bret) bret = m_pBtApi->Init();
        if(!bret)
            qDebug() << "DBus is not availlable";
    }
    else //C api
    {
        m_pBtApi = new BluetoothHci(this);
        if (bret) bret = m_pBtApi->Init();
        if(!bret)
            qDebug() << "HCI API is not availlable";
    }

    MWTS_LEAVE;
    if(bret)
        return 0;
    return -1;
}

/**
 * Set BT device's scan mode
 * @param mode  Scan mode
 * @return      true if succeeded, otherwise false
 */
bool BluetoothTest::SetScanMode(BluetoothApiInterface::ScanMode mode)
{
    MWTS_ENTER;

    bool bret = true;

    if(!m_pBtApi)
    {
        qCritical() << "No Api specified";
        return false;
    }

    bret = m_pBtApi->SetScanMode(mode);

    g_pResult->StepPassed("ScanMode", bret);

    MWTS_LEAVE;
    return bret;
}

/**
 * Set BT device's power mode
 * @param bool  Power mode, on = true, off = false
 * @return      true if succeeded, otherwise false
 */
bool BluetoothTest::SetPowerMode(bool isOn)
{
    MWTS_ENTER;

    bool bret = true;

    if(!m_pBtApi)
    {
        qCritical() << "No Api specified";
        return false;
    }
    bret = m_pBtApi->SetPowerMode(isOn);

    g_pResult->StepPassed("PowerMode", bret);

    MWTS_LEAVE;
    return bret;
}

/**
 * Make BT device scan and resolve names
 * @return  true if succeeded, otherwise false
 */
bool BluetoothTest::Scan()
{
    MWTS_ENTER;
    bool bret = true;

    if(!m_pBtApi)
    {
        qCritical() << "No Api specified";
        return false;
    }
    int endtime,starttime;

    // Get start time
    starttime = g_pTime->elapsed();

    bret = m_pBtApi->Scan();

    endtime = g_pTime->elapsed() - starttime;

    g_pResult->StepPassed("Scan", bret);
    g_pResult->AddMeasure("Scan", endtime/1000.0f, "seconds");

    MWTS_LEAVE;
    return true;
}

/**
 * Transfer for given amount bytes
 * @param role  Role
 * @param type  Socket type
 * @param host  MAC address of the host
 * @param bytes amount of bytes to send
 * @param buffize buffer size used
 * @param time  time to send in seconds
 * @return      amount transferred or -1 if error
 */
int BluetoothTest::Transfer(ServiceRole role,
                            TransferType type/* = L2CAP*/,
                            char* host /*= NULL*/,
                            long int bytes /*=-1*/,
                            int buffsize /*= -1*/,
                            int time /*= -1*/  )
{
    MWTS_ENTER;
    qDebug() <<  "BluetoothTest::Transfer" << "SocketRole" <<  role << "SocketType" <<  type <<  "host"  <<  host << "bytes" << bytes << "buffsize" << buffsize << "time" << time;

    qDebug("BluetoothTest::Transfer, all initialised to get signals and listerners started");

    qDebug() << "call Transfer";
	int transfered = m_pSocket->Transfer(role,type, host, bytes, buffsize, time);
    Start();
    qDebug() << "transfer returned " << transfered;

    MWTS_LEAVE;

    if (transfered > 0)
        return 1;

    return -1;
}

/**
* Functionalities for BT device management.
* @param command    functinality. eg. connect, disconnect, remove
* @param host       host adress
* @param pin        pin of pairing
* @param isClient   pairing role, client/server
* @param isHeadset  is the device a headset
* @return           returns success of the functionality. 0 == success, -1 == failure.
*/
int BluetoothTest::Device(int command, char* host /*= 0*/, char* pin /*= 0*/, bool isClient/*=true*/, bool isHeadset/*=false*/)
{
    MWTS_ENTER;
    bool bret = true;    //return value
    QString phost;
    QString ppin;

    /* Create the host/mac adress */
    if (host == NULL)
    {
        qDebug() << "host_mac" << g_pConfig->value("host_mac").toString();
        phost = g_pConfig->value("host_mac").toString();
    }
    else
    {
        /* check if equals to any key available in .conf file */
        /* load mac addresses from conf file */
        qDebug() << "Load mac addresses";
        g_pConfig->beginGroup("MAC");
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
                phost = value;
            }
        }
        g_pConfig->endGroup();
        if(phost.isEmpty())
            phost = host;
        qDebug() << "phost" << phost;
    }

    /* Create pin code */    
    if (pin == NULL)
    {
        qDebug() << "pin" << g_pConfig->value("passkey").toString();
        ppin = g_pConfig->value("pin").toString();
    }
    else
    {
        ppin = pin;
        qDebug() << "pin" << ppin;
    }

    /* Set settings */
    m_pBtApi->SetSettings(phost,ppin,isClient,isHeadset);

    switch(command)
    {
    case 0: //connect
        if(bret) bret = m_pBtApi->Connect();
        g_pResult->StepPassed("device connected", bret);
        break;
    case 1: //disconnect
        if(bret) bret = m_pBtApi->Disconnect();
        g_pResult->StepPassed("device disconnected", bret);
        break;//	qDebug() << "Ensure that Obex is initialized";
        //	if(!m_pObex->EnsureObex())
        //	{
        //		qCritical() << "Obex initialization failed";
        //		return -1;
        //	}
    case 2: //remove
        if(bret) bret = m_pBtApi->Remove();
        g_pResult->StepPassed("device removed", bret);
        break;
    default:
        bret = false;
        break;
    }

    MWTS_LEAVE;
    if(bret)
        return 0;
    else
        return -1;
}

/**
 * @brief Transfers a file using obex protocol
 * @param ServiceRole role, role of obex protocol,default client
 * @param char* host, host mac address
 * @param char* filename, name of file to transfer
 * @return 0 on success
 */
int BluetoothTest::FileTransfer(char* filename, char* host, ServiceRole role)
{
	MWTS_ENTER;
	int bret = true;

	if(!m_pObex)
	{
		qCritical() << "No obex instance";
		return -1;
	}

	/* parse host */
	QString phost;
	if(host == 0)
        phost = QString(g_pConfig->value("host_mac").toString());
	else
		phost = host;
	qDebug() << "host_mac:"<< phost;
	/* create path */
	qDebug() << "Append path and filename";
	QString path = g_pConfig->value("Obex/path").toString();
	path.append(filename);
	qDebug() << "file:" << path;

	/* get proto path */
	QString proto = g_pConfig->value("Obex/proto").toString();

	/* transfer */
	qDebug() << "Call obex transfer";
	bret = m_pObex->Transfer(role, phost,path,proto);

	/* mark result */
    g_pResult->StepPassed(this->CaseName(), bret);

	MWTS_LEAVE;
	if(!bret)
		return -1;
	return 0;
}

/**
* @brief gets the mac-adress of this device.
* @return mac of this device.
*/
const QString& BluetoothTest::GetAddress() const
{
    if(!m_pBtApi)
    {
        qCritical() << "No Api specified";
        return QString();
    }
    return m_pBtApi->GetAddress();
}

/**
* @brief tells main test class that file transfer has stopped.
*/
void BluetoothTest::transferStopped()
{
    MWTS_ENTER;
    MWTS_DEBUG("transfer stopped");
    if(m_pSocket)
        m_pSocket->terminate();
    Stop();
    MWTS_LEAVE;
}

/**
* @brief writes periodic throughput (Mbit/s) into result file
*/
void BluetoothTest::periodicThroughput(double sampleTime, double bytesPerSecond)
{
    MWTS_ENTER;
    MWTS_DEBUG("Periodic Throughput");
    qDebug() << "Periodic Throughput" << sampleTime << bytesPerSecond;
    g_pResult->AddMeasure( "Periodic Sample Time", sampleTime, "s" );
    g_pResult->AddMeasure( "Periodic Throughput", (bytesPerSecond/1024.0)/1024.0*8, "Mbit/s" );
    MWTS_LEAVE;
}

/**
* @brief tells the main test class that transfer finished succesfully
*/
void BluetoothTest::transferFinished(double bytesPerSecond,bool isSuccessfull)
{
    MWTS_ENTER;
    qDebug() << "Transfer finished: "<<isSuccessfull;
    g_pResult->StepPassed(this->CaseName(), isSuccessfull);
    g_pResult->AddMeasure( "Transfer Throughput", (bytesPerSecond/1024.0)/1024.0*8, "Mbit/s" );
    MWTS_LEAVE;
}

/**
* @brief signals that an error happened in the transfer, and test has to be terminated.
*/
void BluetoothTest::transferError(QString error)
{
    MWTS_ENTER;
    MWTS_DEBUG("transfer error");
    g_pResult->StepPassed(this->CaseName(), false);
    g_pResult->Write(error);
    qCritical() << "transfer error" << error;
    if(m_pSocket)
        m_pSocket->terminate();
    Stop();
    MWTS_LEAVE;
}

/**
* @brief signals that small error happened in transfer. Logs but doesn't terminate.
*/
void BluetoothTest::transferWarning(QString warning)
{
    MWTS_ENTER;
    MWTS_DEBUG("transfer warning");
    g_pResult->Write(warning);
    qWarning() << "transfer error" << warning;
    MWTS_LEAVE;
}
