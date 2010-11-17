/* ObexServerAgent.cpp
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
#include "ObexServerAgent.h"
#include "dbus-headers/ObexServerAgentRelay.h"
#include "BluetoothCommonTypes.h"


ObexServerAgent::ObexServerAgent(const QString& file, QObject* parent):
	QObject(parent)
{
	MWTS_ENTER;
    m_pRelay = new ObexServerAgentRelay(this);

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

ObexServerAgent::~ObexServerAgent()
{
	MWTS_ENTER;
	delete m_pRelay;
	m_pRelay = 0;
	MWTS_LEAVE;
}

/**
 * 	This method gets called when the service daemon
 *	needs to accept/reject a Bluetooth object push request.
 *	Returns the full path (including the filename) where
 *	the object shall be stored.
 *	@param	transfer, the path to the transfer session
 *	@param	bt_address, address of sending bluetooth device
 *	@param	name, file name
 *	@param	type,
 *	@param	length, length in bytes
 *	@param	time,
 *	@return QString path, full path where to transfer the file.
 * */
QString ObexServerAgent::Authorize(const QDBusObjectPath &transfer,
										const QString& bt_address,
										const QString& name,
										const QString& type,
										int length,
										int time)
{
	MWTS_ENTER;

	/* print */
	qDebug() << "Transfer path:" << transfer.path();
	qDebug() << "BT address:" << bt_address;
	qDebug() << "name:" << name;
	qDebug() << "type:" << type;
	qDebug() << "length:" << length;
	qDebug() << "time:" << time;

	/* send signal */
	emit authorize(transfer, bt_address, name, type, length, time);

	MWTS_LEAVE;
	return m_sFile;			//return path where to put the file
}

/**
 *	This method gets called to indicate that the agent
 *	request failed before a reply was returned. It cancels
 *	the previous request.
 * */
void ObexServerAgent::Cancel()
{
	MWTS_ENTER;
	/* send signal */
	emit cancelled();
	MWTS_LEAVE;
}

/**
 * Returns the dbus path of the agent
 * @return dbus path
 */
const QString& ObexServerAgent::Path() const
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return m_sPath;
}
