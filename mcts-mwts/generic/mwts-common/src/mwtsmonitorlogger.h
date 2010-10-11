/* mwtsmonitorlogger.h --
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

#ifndef MWTSMONITORLOGGER_H
#define MWTSMONITORLOGGER_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QTime>


class MwtsMonitorLogger : public QObject
{
	Q_OBJECT
public:
	MwtsMonitorLogger();
	void Write(QString tag, double value);
	void Write(QString text);

protected slots:
	void OnConnected();
	void OnDisconnected();
	void OnSocketData();
	void OnNetworkError(QAbstractSocket::SocketError socketError);

protected:
	QFile m_OutputFile;
	QTcpSocket* m_pSocket;
	QTime time;
};


#endif // MWTSMONITORLOGGER_H
