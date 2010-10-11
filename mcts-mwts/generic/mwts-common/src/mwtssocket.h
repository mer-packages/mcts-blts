/* mwtssocket.h --
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

#ifndef _INCLUDED_MWTS_SOCKET_H
#define _INCLUDED_MWTS_SOCKET_H

#include <MwtsCommon>
#include <QObject>

// Forward classes
class QAbstractSocket;


/** Base class for Socket functionality */
class MWTSCOMMON_EXPORT MwtsSocket: public QObject
{
	Q_OBJECT

public:
	MwtsSocket();
	virtual ~MwtsSocket();

/**
 * Create QT socket from base C, int socket. C-socket must be open and connected, either client or server socket
 * @param a_iSocket	C-socket
 * @return Qt-socket, type is QAbstractSocket, NULL if error
 */
    QAbstractSocket* CreateSocket(int a_iSocket);
	
	/**
	 * Send for given amount bytes
	 * @param bytes	amount of bytes to send
	 * @return 		amount transferred or -1 if error
	 */
	int Send( int bytes );

   
protected:
protected slots:
  void readyRead();

private:
	QAbstractSocket* m_pQSocket;


};


#endif // #ifndef _INCLUDED_MWTS_SOCKET_H

