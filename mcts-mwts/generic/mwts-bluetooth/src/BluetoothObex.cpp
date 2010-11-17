/* BluetoothObex.cpp
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
 */
#include "BluetoothObex.h"
#include <stable.h>
#include <QFile>

#define DBUS_OBEX_NAME "org.openobex"
#define DBUS_OBEX_CLIENT_NAME "org.openobex.client"
#define DBUS_OBEX_PATH "/"
#define DBUS_OBEX_INTERFACE_AGENT "org.openobex.Agent"
#define DBUS_OBEX_INTERFACE_MANAGER "org.openobex.Manager"
#define DBUS_OBEX_INTERFACE_CLIENT "org.openobex.Client"
#define DBUS_OBEX_INTERFACE_TRANSFER "org.openobex.Transfer"
#define DBUS_OBEX_INTERFACE_SESSION "org.openobex.Session"
#define DBUS_OBEX_INTERFACE_FTP "org.openobex.FileTransfer"

/**
 * Obex class Constructor
 */
BluetoothObex::BluetoothObex(QObject* parent)
	:QObject(parent),
	 m_bIsReady(false),
	 m_eRole(UndefinedRole),
	 m_sHost(""),
	 m_sFilePath(""),
	 m_pClient(0),
	 m_pManager(0),
	 m_iSize(0),
	 m_iStartTime(0),
	 m_pServerAgent(0),
	 m_pClientAgent(0)
{
	MWTS_ENTER;
	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
	MWTS_LEAVE;
}

/**
 * 	Obex class detructor
 * */
BluetoothObex::~BluetoothObex()
{
	MWTS_ENTER;

	/* if transfers then delete them */
	if(!m_Transfers.isEmpty())
	{
		foreach(QDBusInterface* face, m_Transfers)
		{
			delete face;
			face = 0;
		}
		m_Transfers.clear();
	}

    /* unregister agent from dbus */
    QDBusConnection::sessionBus().unregisterObject(DBUS_AGENT_PATH);
    QObject* obj = QDBusConnection::sessionBus().objectRegisteredAt(DBUS_AGENT_PATH);
	bool bret = ((obj)?(true):(false));
	qDebug() << "Agent object found === "<< bret;

	qDebug() << "delete server agent";
	delete m_pServerAgent;
	m_pServerAgent = 0;
	qDebug() << "delete client agent";
	delete m_pClientAgent;
	m_pClientAgent = 0;
	qDebug() << "delete manager";
	delete m_pManager;
	m_pManager = 0;
	qDebug() << "delete client";
	delete m_pClient;
	m_pClient = 0;
	MWTS_LEAVE;
}

/**
 *	@brief Initiates a file transfer using obex protocol.
 *	@param role, the servicerole of the transfer instance.
 *	@param host, host mac address, to which the transfer take place with
 *	@param filename, the filename to be sent
 *	@param protofilename, the proto filename of which we will take a copy of.
 *	@return	success, return the success of transfer.
 * */
bool BluetoothObex::Transfer(ServiceRole role, QString host, QString filename, QString protofilename)
{
	MWTS_ENTER;
	bool bret = true;

	/* init variables */
	m_eRole = role;
	m_sHost = host;
	m_sFilePath = filename;
	m_sProtoPath = protofilename;

	/* Ensure that we have dbus connection */
	bret = QDBusConnection::sessionBus().isConnected();
	if(!bret)
		qCritical() << "No dbus connection available";

	qDebug() << "Role:" << ((role==Server)?("Server"):("Client"));

	/* Ensure that we don't have the file */
	bret = QFile::exists(m_sFilePath);
	qDebug() << "File exists =="<< bret;
	if(bret)
	{
		bret = QFile::remove(m_sFilePath);
		qDebug() << "File removed:" << bret;
	}
	else
		bret = true;

	/* start timeout timer */
	qDebug() << "Start timeout";
	m_Timer.start(TEST_TIMEOUT);

	/* start throughput time */
	qDebug() << "Start throughput timer";
	m_Time.start();

	/* Setup role specific transfer */
	if(bret)
	{
		if(role == Server)
			bret = WaitForIncomingTransfer();
		else
			bret = SendFile();

		/* end throughput time */
		int endtime = m_Time.elapsed();
		qDebug() << "Total bytes transferred:" << m_iSize;
		int ms = (endtime-m_iStartTime);
		qDebug() << "Time" << ms << "ms";
		double seconds = ms / 1000.0;
		qDebug() << "Time" << seconds << "s";
		double bytesPerSecond = ((double)m_iSize) / seconds;
		qDebug() << "BytesPerSecond" << bytesPerSecond << "b/s";
		qDebug() << "Throughput:"<< ((bytesPerSecond*8.0)/1024.0)/1024.0 << "MBit/s";
		g_pResult->AddMeasure("Transfer throughput",((bytesPerSecond*8.0)/1024.0)/1024.0,"MBit/s");
	}

	MWTS_LEAVE;
	return bret;
}

/**
 * 	@brief	The file sending specific stuf..
 *  @return success, returns success if file were sent succesfully.
 * */
bool BluetoothObex::SendFile()
{
	MWTS_ENTER;
	bool bret = true;

	/* recreate the file from proto */
	bret = QFile::copy(m_sProtoPath, m_sFilePath);
	/* give permissions */
	bret = QFile::setPermissions(m_sFilePath,QFlag(0x7777));

	/* create client interface */
	m_pClient = CreateInterface(DBUS_OBEX_CLIENT_NAME, DBUS_OBEX_INTERFACE_CLIENT, DBUS_OBEX_PATH);

	/* create agent */
	m_pClientAgent = new ObexClientAgent(m_sFilePath,this);
	/* connect agent signals */
	connect(m_pClientAgent, SIGNAL(request(const QDBusObjectPath& )),
			this, SLOT(OnRequest(const QDBusObjectPath& )));
	connect(m_pClientAgent, SIGNAL(progress(const QDBusObjectPath& , qulonglong )),
			this, SLOT(OnProgress(const QDBusObjectPath& , qulonglong )));
	connect(m_pClientAgent, SIGNAL(complete(const QDBusObjectPath&,bool )),
			this, SLOT(OnComplete(const QDBusObjectPath&,bool )));
	connect(m_pClientAgent, SIGNAL(release()),
			this, SLOT(OnRelease()));
	connect(m_pClientAgent, SIGNAL(error(const QString&)),
			this, SLOT(OnError(const QString&)));

	/* call send */
	qDebug() << "create arguments";
	qDebug() << "file:" << m_sFilePath;
	QVariantMap target;
	target.insert("Destination",m_sHost);
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(target)
				 << qVariantFromValue(QStringList(m_sFilePath))
				 << qVariantFromValue(QDBusObjectPath(m_pClientAgent->Path()));
	qDebug() << "Send files";
	m_pClient->callWithArgumentList(QDBus::BlockWithGui, QLatin1String("SendFiles"),argumentList);
	qDebug() << "Client:" << m_pClient->lastError().message();

	/* wait for transfer to stop */
	qDebug() << "Waiting for transfer to finish...";
	int ret = m_Eventloop.exec();
	if(ret < 0)
	{
		qCritical() << "Eventloop exited with error code.";
		qDebug() << "Client:" << m_pClient->lastError().message();
		bret = false;
	}

	MWTS_LEAVE;
	return bret;
}

/**
 * 	@brief	This waits for incoming transfer, and receives the file given to it.
 *  @return success, true if received succesfully
 * */
bool BluetoothObex::WaitForIncomingTransfer()
{
	MWTS_ENTER;
	bool bret = true;

	/* create manager */
	m_pManager = CreateInterface(DBUS_OBEX_NAME,DBUS_OBEX_INTERFACE_MANAGER,DBUS_OBEX_PATH);
	/* connect manager signals */
	connect(m_pManager, SIGNAL(TransferStarted(QDBusObjectPath)),
			this, SLOT(OnTransferStarted(QDBusObjectPath)));
	connect(m_pManager, SIGNAL(TransferCompleted(QDBusObjectPath, bool)),
			this, SLOT(OnComplete(QDBusObjectPath,bool)));
//	connect(m_pManager, SIGNAL(SessionCreated(QDBusObjectPath)),
//			this, SLOT(OnTransferStarted(QDBusObjectPath)));
//	connect(m_pManager, SIGNAL(SessionRemoved(QDBusObjectPath)),
//			this, SLOT(OnComplete(QDBusObjectPath)));
	qDebug() << m_pManager->lastError().message();

	/* create agent */
	m_pServerAgent = new ObexServerAgent(m_sFilePath, this);
	qDebug() << "Agent created";
	/* connect agent signals */
	connect(m_pServerAgent, SIGNAL(authorize(	const QDBusObjectPath&,
												const QString&,
												const QString&,
												const QString&,
												int,
												int)),
			this, SLOT(OnAuthorize(				const QDBusObjectPath&,
												const QString&,
												const QString&,
												const QString&,
												int,
												int)));
	connect(m_pServerAgent, SIGNAL(cancelled()),
			this, SLOT(OnCancelled()));
	qDebug() << "Signals connected";
	/* register agent */
	m_pManager->call(QLatin1String("RegisterAgent"),qVariantFromValue(QDBusObjectPath(m_pServerAgent->Path())));
	qDebug() << "agent registered";
	qDebug() << m_pManager->lastError().message();

	/* start running */
	qDebug() << "Waiting for incoming transfer...";
	int ret = m_Eventloop.exec();
	if(ret < 0)
	{
		qCritical() << "Eventloop exited with error code";
		qDebug() << m_pManager->lastError().message();
		bret = false;
	}


	MWTS_LEAVE;
	return bret;
}

/**
*   @brief      Creates a DBus Interface and returns instance of it.
*   @param		QString			service, dbus service
*   @param      QString         type of the interface.
*   @param      QString         path of the object.
*   @return     DBusInterface   dbus interface instance.
*/
QDBusInterface* BluetoothObex::CreateInterface(const QString& service, const QString& type, const QString& path)
{
    MWTS_ENTER;
    QDBusInterface* pret = 0;

    qDebug() << "Create dbus interface: " << type ;
    qDebug() << "for service:"<< service;
    qDebug() << "with path:"<< path;
    pret = new QDBusInterface(service,path,type,QDBusConnection::sessionBus());
    qDebug() << "Has a dbus Interface == " << pret->isValid() << " of type " << type;
    if(!pret->isValid()) qCritical() << "No dbus Interface "<< type << " availlable";

    MWTS_LEAVE;
    return pret;
}

void BluetoothObex::OnTimeout()
{
	MWTS_ENTER;
	qCritical() << "Timeout occurred";

	/* stop eventloop if running */
	if(m_Eventloop.isRunning())
	{
		qDebug() << "Stopped Eventloop";
		m_Eventloop.exit(-1);					//exit eventloop with an error code.
	}

	MWTS_LEAVE;
}

/**
 *	@brief Is called when sending side requests a transfer (client).
 *	@param transfer, the transfer requested
 * */
void BluetoothObex::OnRequest(const QDBusObjectPath& transfer)
{
	MWTS_ENTER;
	qDebug() << "transfer has been requested";

	/* stop timeout */
	m_Timer.stop();

	/* add transfer to list */
	QDBusInterface* transferi = CreateInterface(DBUS_OBEX_NAME,DBUS_OBEX_INTERFACE_TRANSFER,transfer.path());
	m_Transfers.insert(transfer.path(),transferi);
	qDebug() << "Transfer interface added to the list";

	/* take start time */
	m_iStartTime = m_Time.elapsed();

	MWTS_LEAVE;
}

/**
 * 	@brief Is called periodically when sending (client) side is transfering.
 * 	@param	transfer, transfer in use.
 * 	@param	transferred, amount of bytes transferred
 * */
void BluetoothObex::OnProgress(const QDBusObjectPath& transfer, qulonglong transferred)
{
	MWTS_ENTER;
	qDebug() << "Bytes transferred:" << transferred;
	m_iSize = (int) transferred;
	MWTS_LEAVE;
}

/**
 *	@brief	Called when transfer is complete (client&server)
 *	@param	transfer, the transfer finished
 *	@param	success, if completed succesfully
 * */
void BluetoothObex::OnComplete(const QDBusObjectPath& transfer, bool success)
{
	MWTS_ENTER;
	qDebug() << "Transfer has completed";
	int ret = 0;

	/* check for success */
	if(!success)
		return;		// not necessarily failed. Maybe not just yet begun.

	/* remove transfer from list */
	QDBusInterface* face = m_Transfers.value(transfer.path());
	delete face;
	face = 0;
	qDebug() << m_Transfers.remove(transfer.path()) << "transfers removed from the list";

	/* check if can exit */
	if(m_Transfers.isEmpty())
	{
		/* stop eventloop */
		if(m_Eventloop.isRunning())
		{
			qDebug() << "Stopped Eventloop";
			m_Eventloop.exit(ret);					//exit eventloop with an success code.
		}
	}
	MWTS_LEAVE;
}

/**
 * 	@brief called when agent is released (client).
 * */
void BluetoothObex::OnRelease()
{
	MWTS_ENTER;
	/* check if eventloop is running */
	if(m_Eventloop.isRunning())
	{
		/* eventloop were running and agent released. Thus the transfer were not complete. */
		m_Eventloop.exit(-1);	//exit with error code
		qCritical() << "Transfer failure. Agent released, without transfer to complete.";
	}
	MWTS_LEAVE;
}

/**
 * 	@brief	called when error happens (client)
 * 	@param	error, the error message
 * */
void BluetoothObex::OnError(const QString& error)
{
	MWTS_ENTER;
	/* check if eventloop is running */
	if(m_Eventloop.isRunning())
	{
		m_Eventloop.exit(-1);	//exit with error code
		qCritical() << "Transfer error:" << error;
	}
	MWTS_LEAVE;
}

/**
 *	@brief	called when a transfer is authorized (server).
 *	@param	transfer, the transfer to be authorized
 *	@param	bt_address, address of which the transfer is coming from
 *	@param	name, name of the transferred file
 *	@param	type,
 *	@param	lenght, number of bytes in the file
 *	@param	time,
 * */
void BluetoothObex::OnAuthorize(	const QDBusObjectPath& transfer,
									const QString& bt_address,
									const QString& name,
									const QString& type,
									int length,
									int time)
{
	MWTS_ENTER;
	qDebug() << "Transfer request has been authorized";
	MWTS_LEAVE;
}

/**
 * 	@brief called if transfer is cancelled (server).
 * */
void BluetoothObex::OnCancelled()
{
	MWTS_ENTER;
	qCritical() << "Transfer has been canceled from client side";
	/* stop eventloop if running */
	if(m_Eventloop.isRunning())
	{
		qDebug() << "Stopped Eventloop";
		m_Eventloop.exit(-1);					//exit eventloop with an error code.
	}
	MWTS_LEAVE;
}

/**
 * 	@brief	Called when transfer started (server).
 * 	@param	transfer, the transfer to start
 * */
void BluetoothObex::OnTransferStarted(const QDBusObjectPath& transfer)
{
	MWTS_ENTER;
	qDebug() << "Transfer started";

	/* stop timeout */
	m_Timer.stop();

	/* add transfer to list */
	QDBusInterface* transferi = CreateInterface(DBUS_OBEX_NAME,DBUS_OBEX_INTERFACE_TRANSFER,transfer.path());
	m_Transfers.insert(transfer.path(),transferi);
	connect(transferi, SIGNAL(Progress(int,int)),this, SLOT(OnServerSideProgress(int,int)));
	qDebug() << "Transfer interface added to the list";

	/* take start time */
	m_iStartTime = m_Time.elapsed();

	MWTS_LEAVE;
}

/**
 * 	@brief called periodically when transfer in progress (server).
 * 	@param	total, total bytes to transfer
 * 	@param	transferred, bytes transferred
 * */
void BluetoothObex::OnServerSideProgress(int total, int transferred)
{
	MWTS_ENTER;
	qDebug() << transferred << "of" << total << "transferred";
	m_iSize = transferred;
	MWTS_LEAVE;
}
