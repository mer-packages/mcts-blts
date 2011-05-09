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
#include <QtDBus>
#include <QDBusInterface>

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

    /**
	*  Closes all active wlan sessions
	*/ 
	void CloseActiveSessions();	

    /**
	* Turns wlan chip on/off
	*/ 
	bool SwitchWlan(const QString state);

    /**
	 * Create network session from target config
	 */
	bool ConnectToConfig(const QNetworkConfiguration& config);

    /**
     * helper method to get target service dbus object path from connman manager
     * @param ap_name
     */
    QString GetServicePath(const QString ap_name);

    /**
     * Creates connection to wlan/psd access point. Uses connman
     * @param ap_name target access point name
     */
    bool ConnmanConnection(const QString ap_name);

    /**
     * Removes available service configuration
     * @param ap_name target access/service point name
     */
    bool RemoveService(const QString ap_name);

    /**
     * Sets tethering on/off
     * @param mode on or off
     */
    bool SetTethering(const QString mode);

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

    /**
	 * Idle loop
	 */
	void RunIdle();

    /**
	 * Stops running networksession
	 */
    bool StopSession(const QString ap_name);

    /**
	 * Returns the status network connection
	 */
	bool IsOnline();

    /**
	 * Finds the ip of given interface
	 */
	bool FindIp(const QString strInterface);

    /**
	 * Roaming setter
	 */
	inline void setRoaming() { bCanRoam = true; }


protected slots:
	void configurationsUpdateCompleted();
	void configurationAdded(const QNetworkConfiguration& config);
	void networkStateChanged(QNetworkSession::State state);
	void networkError(QNetworkSession::SessionError error);
	void configurationsAdded(const QNetworkConfiguration& config);
	void configurationsRemoved(const QNetworkConfiguration& config);
	void configurationsChanged(const QNetworkConfiguration& config);

    /*
     * Connman manager slot
     */
    void slotConnmanStateChanged();

	/*
	 * Slots for roaming support.
	 */
	void newConfigurationActivatedHandler ();
	void preferredConfigurationChangedHandler ( const QNetworkConfiguration & config, bool isSeamless );

	/*
	* Slots for http download functions
	*/
	void downloadFinished(QNetworkReply *reply);
    void slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void slotError(QNetworkReply::NetworkError error);

    /*
	 * Slots for QProcess signals
	 */
	void slotError(QProcess::ProcessError error);
	void slotFinished( int exitCode, QProcess::ExitStatus exitStatus );
	void slotStateChanged  ( QProcess::ProcessState newState );
	void slotReadStandardOutput();


private:
    void debugPrintConfiguration(QNetworkConfiguration config);
    QNetworkConfiguration GetConfigurationByName(QString ap_name);
    QString saveFileName(const QUrl &url);


private:
    enum SecType { WEP, WPA_PKS, NONE };
    typedef QMap<QString, SecType> StringToEnumMap;

    QDBusInterface *m_ConnmanManager;

    QNetworkConfigurationManager networkManager;
	QNetworkConfiguration networkConfiguration;
	QNetworkSession *networkSession;

	// for http download purposes
    QNetworkAccessManager m_httpManager;
    QNetworkReply *m_pHttpReply;
    QFile *m_pHttpDownloadFile;
    qint64 m_iHttpDownloadCounter;
    bool m_bHttpDownloadSuccess;

	bool bCanRoam;

	QString m_strDeviceIp;

	// Overall result
	bool m_bResult;
	QByteArray m_baStdErr;
	QByteArray m_baStdOut;

	QProcess* m_pProcess;

	bool m_bUpdateCompleted;
};

#endif //#ifndef _INCLUDED_NETWORK_TEST_H

