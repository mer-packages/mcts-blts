/* ObexClientAgent.cpp
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
#include <QtDBus>
#include <stable.h>
#include "ObexClientAgent.h"
#include "dbus-headers/ObexClientAgentRelay.h"
#include "BluetoothCommonTypes.h"

ObexClientAgent::ObexClientAgent(const QString& file, QObject* parent):
	QObject(parent)
{
	MWTS_ENTER;
    m_pRelay = new ObexClientAgentRelay(this);

    m_sPath = DBUS_AGENT_PATH;

    if(!file.isEmpty())
    	m_sFile = file;

    /* register agent to dbus */
    bool bret = QDBusConnection::sessionBus().registerObject(DBUS_AGENT_PATH, this);
    qDebug() << "Agent object registered == " << bret;

    /* Double check agents existence */
    qDebug() << "Connection connected == "<< QDBusConnection::sessionBus().isConnected();
    QObject* obj = QDBusConnection::sessionBus().objectRegisteredAt(Path());
    bret = ((obj)?(true):(false));
    qDebug() << "Agent object found === "<< bret;

	MWTS_LEAVE;
}

ObexClientAgent::~ObexClientAgent()
{
	MWTS_ENTER;
	delete m_pRelay;
	m_pRelay = 0;
	MWTS_LEAVE;
}

/**
 *	This method gets called when the service daemon
 *	unregisters the agent. An agent can use it to do
 *	cleanup tasks. There is no need to unregister the
 *	agent, because when this method gets called it has
 *	already been unregistered.
 */
void ObexClientAgent::Release()
{
	MWTS_ENTER;
	/* clean up stuff */
	emit release();
	MWTS_LEAVE;
}

/**
 * 	Accept or reject a new transfer (client and server)
 *	and provide the filename for it.
 *
 *	In case of incoming transfers it is the filename
 *	where to store the file and for outgoing transfers
 *	it is the filename to show the remote device. If left
 *	empty it will be calculated automatically.
 * 	@param transfer, a transfer instance to be requested
 * 	@return path, filename of the transferred file
 */
QString ObexClientAgent::Request(const QDBusObjectPath &transfer)
{
	MWTS_ENTER;
//	qDebug() << "Set file to:" << m_sFile;
	/* emit signal */
	emit request(transfer);
	MWTS_LEAVE;
//	return m_sFile;
	return "";
}

/**
 *	Progress within the transfer has been made. The
 *	number of transferred bytes is given as second
 *	argument for convenience.
 * 	@param transfer, transfer instance
 * 	@param transferred, number of transferred bytes
 */
void ObexClientAgent::Progress(const QDBusObjectPath &transfer,qulonglong transferred)
{
	MWTS_ENTER;
	/* emit signal*/
	emit progress(transfer, transferred);

	MWTS_LEAVE;
}

/**
 *	This funcion is called when the transfer is complete.
 *	@parem transfer, transfer instance
 * */
void ObexClientAgent::Complete(const QDBusObjectPath &transfer)
{
	MWTS_ENTER;
	/* emit signal*/
	emit complete(transfer,true);	// transfer ended succesfully
	MWTS_LEAVE;
}

/**
 *	Informs that the transfer has been terminated because
 *	of some error.
 * 	@param transfer	transfer object
 * 	@param message	error message
 */
void ObexClientAgent::Error(const QDBusObjectPath& transfer, const QString& message)
{
	MWTS_ENTER;
	qDebug() << message;
	emit error(message);
	MWTS_LEAVE;
}

/**
 * 	Returns the dbus path of the agent
 * 	@return dbus path
 */
const QString& ObexClientAgent::Path() const
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return m_sPath;
}

