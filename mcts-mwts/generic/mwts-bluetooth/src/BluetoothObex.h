/* BluetoothObex.h
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

#ifndef BLUETOOTHOBEX_H_
#define BLUETOOTHOBEX_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QEventLoop>
#include <QMap>
#include "BluetoothCommonTypes.h"
#include "dbus-headers/ObexClientAgent.h"
#include "dbus-headers/ObexServerAgent.h"

class BluetoothObex: public QObject
{
	Q_OBJECT
public:
	BluetoothObex(QObject* parent = 0);
	~BluetoothObex();

	bool Transfer(ServiceRole role, QString host, QString filename, QString protofilename);

	bool SendFile();
	bool WaitForIncomingTransfer();

public slots:
	void OnTimeout();
	/* slots for client agent */
	void OnRequest(const QDBusObjectPath& transfer);
	void OnProgress(const QDBusObjectPath& transfer, qulonglong transferred);
	void OnComplete(const QDBusObjectPath& transfer, bool success);
	void OnRelease();
	void OnError(const QString& error);
	/* slots for server agent */
	void OnAuthorize(	const QDBusObjectPath& transfer,
						const QString& bt_address,
						const QString& name,
						const QString& type,
						int length,
						int time);
	void OnCancelled();
	/* slots for manager interface */
	void OnTransferStarted(const QDBusObjectPath& transfer);
	/* slots for transfer interface */
	void OnServerSideProgress(int total, int transferred);

protected:
	QDBusInterface* CreateInterface(const QString& service,const QString& type, const QString& path);

private:
	ServiceRole	m_eRole;			///< role, eg. client/server
	QString		m_sHost;			///< host mac
	QString		m_sFilePath;		///< file path
	QString		m_sProtoPath;		///< path of the prototype file
	QTimer		m_Timer;			///< timeout timer
	QEventLoop	m_Eventloop;		///< eventloop

	QDBusInterface* m_pManager;		///< manager instance
	QDBusInterface*	m_pClient;		///< client instance

	QMap<QString,QDBusInterface*> m_Transfers;	///< list of ongoing transfers

	/* throughput */
	QTime		m_Time;				///< time for throughput time measurement
	int			m_iStartTime;		///< measure starting time.
	int			m_iSize;			///< size of transfer

	/* agents */
	ObexClientAgent*	m_pClientAgent;		///< client agent instance
	ObexServerAgent*	m_pServerAgent;		///< Server agent instance

	bool		m_bIsReady;			///< flag telling whether the asset is ready or not.
};

#endif /* BLUETOOTHOBEX_H_ */
