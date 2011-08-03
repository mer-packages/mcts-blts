/* mwtsmonitorlogger.cpp --
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
#include "mwtsmonitorlogger.h"
#include <MwtsCommon>

/** Initializes monitor logger. Starts connecting to remote host,
  opens a monitor output file */
MwtsMonitorLogger::MwtsMonitorLogger()
{
	m_pSocket=new QTcpSocket;

	connect(m_pSocket, SIGNAL(connected()), this, SLOT(OnConnected()));
	connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(OnNetworkError(QAbstractSocket::SocketError)));
	connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(OnSocketData()));

	qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError" );

	qDebug() << "Connecting to host 192.168.2.14 port 12345";
	m_pSocket->connectToHost("192.168.2.14", 12345);

	m_OutputFile.setFileName("/var/log/tests/"+g_pTest->CaseName()+".monitor");
    qDebug() << "Opening monitor output file" << m_OutputFile.fileName();
    if(!m_OutputFile.open(QIODevice::WriteOnly))
	{
		qCritical()<<"Unable to open monitor outputfile " +
				m_OutputFile.fileName();
		return;
	}
	time.start();

}
/** OnConnected gets called when connection to remote host is done succesfully*/
void MwtsMonitorLogger::OnConnected()
{
	MWTS_ENTER;
	qDebug() << "MwtsMonitor connected!";
	QString str = "TestModule:" + g_pTest->Name();
	Write(str);
	str = "TestCase:" + g_pTest->CaseName();
	Write(str);

}
/** OnDisconnected gets called when disconnected from remote host */
void MwtsMonitorLogger::OnDisconnected()
{
	MWTS_ENTER;
	qCritical() << "MwtsMonitor disconnected!";

}

/** OnSocketData gets called when we receive data from remote host*/
void MwtsMonitorLogger::OnSocketData()
{
	MWTS_ENTER;
	QByteArray data=m_pSocket->readAll();
	m_pSocket->write("Data received!");
	m_pSocket->flush();
	qCritical() << "MwtsMonitor new socket data!";
}

/** OnNetworkError gets called if a network error occurs*/
void MwtsMonitorLogger::OnNetworkError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qDebug("MwtsMonitor: Remote host closed connection");
		break;

	case QAbstractSocket::HostNotFoundError:
        qDebug("MwtsMonitor: Could not connect to host");
		break;

	case QAbstractSocket::ConnectionRefusedError:
        qDebug("MwtsMonitor: The connection was refused by the peer");
		break;

	default:
		qDebug() << "MwtsMonitor: Following socket error occurred: " << m_pSocket->errorString();
		break;
	}

}

/** Writes data to monitor file and remote host*/
void MwtsMonitorLogger::Write(QString string)
{
	QString sTime;
	sTime.sprintf("%.3lf", time.elapsed()/1000.0);
	QString str= sTime+":"+string+"\n";

    if(m_pSocket->isValid())
    {
        m_pSocket->write(str.toAscii().constData());
        m_pSocket->flush();
    }

	m_OutputFile.write(str.toAscii().constData());
	m_OutputFile.flush();
}

/** Writes a double value to monitor file and remote host*/
void MwtsMonitorLogger::Write(QString tag, double value)
{
	QString str = tag + ":" + QString::number(value);
	Write(str);
}
