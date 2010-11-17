/* NetworkTest.h -- header of mwts-network test asset
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

#ifndef _INCLUDED_NETWORK_TEST_H
#define _INCLUDED_NETWORK_TEST_H

#include <QtNetwork>

#include <qftp.h>
#include <qhttp.h>

#include <QMap>
#include <QtCore>
#include <QSignalSpy>
#include <MwtsCommon>


class NetworkTest : public MwtsTest
{
	Q_OBJECT
public:
	NetworkTest();
	virtual ~NetworkTest();

	/**
	 * initialization methods
	 */
	void OnInitialize();
	void OnUninitialize();

	/**
	 * Scans available access points. Must be executed before any session/connection opening
	 */
	void UpdateConfigs();

	/**
	 * Lists available configurations found
	 */
	void ListConfigurations();

	/**
	 * Connects to some discovered configuration
	 */
	bool ConnectToDefault();

	/**
	 * Connects to given (ESSID) access point
	 */
	bool ConnectToName(QString ap_name);

	void CloseActiveSessions();	

	bool AddApPassword(const QString ap_name);
	bool AddApPasswords();

	/*
	 * Create network session from target config
	 */
	bool ConnectToConfig(const QNetworkConfiguration& config);

	/**
	 * Just to check if UpdateConfigurations signal is catched
	 */
	bool IsUpdateComplete();

	/**
	 * Returns if given ap is foudn
	 */
	bool IsAccessPointFound(QString ap_name);

	/**
	 * Downloads file from ftp server
	 */
	bool Downloadfile(const QString strFilename);

	/**
	 * Downloads file from http server
	 */
	bool DownloadfileHttp(const QString strFilename);


	QString ConfigurationByName(const QString name);

	/**
	 * Uploads file to ftp server
	 */
	bool UploadFile(const QString strFilename);

	/**
	 * Starts iperf in server via ssh
	 */
	bool ServerLaunchIperf(const char* time);

	/*
	 * Idle loop
	 */
	void RunIdle();

	/*
	 * Stops running networksession
	 */
	void StopSession();

	/*
	 * Returns the status network connection
	 */
	bool IsOnline();

	/*
	 * Finds the ip of given interface
	 */
	bool FindIp(const QString strInterface);

	/*
	 * Roaming setter
	 */
	inline void setRoaming() { bCanRoam = true; }

	/*
	 * Min access function to start http file download
	 */
	bool downloadFileHttp(const QString strUrl);

	/*
	 * Function to save http download to file
	 * Used with http-download
	 */
	bool saveToDisk(const QString &filename, QIODevice *data);

	/*
	 * Helper function to get file name from url
	 */
	QString saveFileName(const QUrl &url);

private:
	/*
	 * Helper function to get file name from url
	 */
	void debugPrintConfiguration(QNetworkConfiguration config);

	QNetworkConfiguration GetConfigurationByName(QString ap_name);

	enum SecType { WEP, WPA_PKS, NONE };

	typedef QMap<QString, SecType> StringToEnumMap;

protected slots:
	void configurationsUpdateCompleted();
	void configurationAdded(const QNetworkConfiguration& config);
	void networkStateChanged(QNetworkSession::State state);
	void networkError(QNetworkSession::SessionError error);
	void configurationsAdded(const QNetworkConfiguration& config);
	void configurationsRemoved(const QNetworkConfiguration& config);
	void configurationsChanged(const QNetworkConfiguration& config);

	/*
	 * Slots for roaming support.
	 */
	void newConfigurationActivatedHandler ();
	void preferredConfigurationChangedHandler ( const QNetworkConfiguration & config, bool isSeamless );


	/*
	* Slots for http download functions
	*/
	void downloadFinished(QNetworkReply *reply);
	void slotError(QNetworkReply::NetworkError error);

	/**
	 * Slots for QProcess signals
	 */
	void slotError(QProcess::ProcessError error);
	void slotFinished( int exitCode, QProcess::ExitStatus exitStatus );
	void slotStateChanged  ( QProcess::ProcessState newState );
	void slotReadStandardOutput();

private:
	QNetworkConfigurationManager networkManager;
	QNetworkConfiguration networkConfiguration;
	QNetworkSession *networkSession;

	// for http download purposes
	QNetworkAccessManager httpmanager;

	QString m_strHttpFileName;

	bool bCanRoam;

	bool m_bHttpDownloadSuccess;

	QString m_strDeviceIp;

	// Overall result
	bool m_bResult;
	QByteArray m_baStdErr;
	QByteArray m_baStdOut;

	QProcess* m_pProcess;

	bool m_bUpdateCompleted;
};

#endif //#ifndef _INCLUDED_NETWORK_TEST_H

