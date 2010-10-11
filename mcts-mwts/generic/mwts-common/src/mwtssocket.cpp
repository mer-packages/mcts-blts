/* mwtssocket.cpp --
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

#include <QAbstractSocket>

#include "stable.h"
#include "mwtssocket.h"

/**
  Constructor of MwtsSocket base class
*/

MwtsSocket::MwtsSocket()
: m_pQSocket(NULL)
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

MwtsSocket::~MwtsSocket()
{
	MWTS_ENTER;
	if(m_pQSocket)
	{
		delete m_pQSocket;
		m_pQSocket = NULL;
	}

	MWTS_LEAVE;
}

QAbstractSocket* MwtsSocket::CreateSocket(int a_iSocket)
{

	qDebug() << "handle QAbstractSocket";
	// delete possible abstract socket
	if(m_pQSocket)
	{
	    disconnect(m_pQSocket, SIGNAL (readyRead()), this, SLOT(readyRead()));

	    qDebug() << "delete m_pQSocket";
		delete m_pQSocket;
		m_pQSocket = NULL;
	}

    // OK This works, move it to mwts-common
	// create new one
	// delete possible abstract socket
	qDebug() << "new QAbstractSocket";
	m_pQSocket = new QAbstractSocket(QAbstractSocket::UnknownSocketType, this);
	bool success = m_pQSocket->setSocketDescriptor(a_iSocket);
	qDebug() << "m_pQSocket->setSocketDescriptor(a_iSocket) returned " << success;

	// Clean up if not succeeded
	if (! success)
	{
	    qDebug() << "delete m_pQSocket";
		delete m_pQSocket;
		m_pQSocket = NULL;
	}
	
	if (success)
	{
	    connect(m_pQSocket, SIGNAL (readyRead()), this, SLOT(readyRead()));
		
	    char* data = "Hello There";
	    qDebug() << "do write ( Hello There )";
	    qint64  written = m_pQSocket->write ( data );
	    qDebug() << "write ( data, strlen(data) returned" << written;
	}


	MWTS_LEAVE;
	return m_pQSocket;
}

int MwtsSocket::Send( int bytes )
{
	MWTS_ENTER;
	int sent = -1;
	
	qDebug() << "Send";
	
	if (m_pQSocket)
	{
	    char* data = "Send data";
	    qDebug() << "do write ( Send data )";
	    qint64  written = m_pQSocket->write ( data );
	    qDebug() << "done  write ( Send data ) returned" << written;
		sent = written;
	}
	else
	{
		qCritical() << "Socket is not created";
		return -1;
	}

    MWTS_LEAVE;

	return sent;
}


void MwtsSocket::readyRead()
{
	MWTS_ENTER;
	QByteArray buff = m_pQSocket->read (1024);
	qDebug() << "read (1024) returned" << buff;
	MWTS_LEAVE;
}




