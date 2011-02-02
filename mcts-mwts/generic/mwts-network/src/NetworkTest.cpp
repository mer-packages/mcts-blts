/* NetworkTest.cpp -- mwts-network test asset
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
#include "NetworkTest.h"
#include "ftpwindow.h"

#include <QTest>
#include <QtDBus>
#include <QVariantMap>
#include <QVariant>

#define TRY_VERIFY(__expr, __message) \
	do{ \
		const int __step = 50;\
		const int __timeout = 90000; \
		if(!(__expr)) { \
			QTest::qWait(0); \
		} \
		for (int __i= 0; __i < __timeout && !(__expr); __i+=__step) {\
			QTest::qWait(__step); \
		} \
		qDebug() << __message; \
		if (__expr) \
			qDebug("True"); \
		else \
			qDebug("False"); \
	} while(0)


#define NT_VERIFY(__expr,__message) \
	if (!__expr) { \
		qDebug() << __message << "FALSE"; \
		MWTS_LEAVE; \
		return false; \
	} \
	qDebug() << __message << "TRUE";

NetworkTest::NetworkTest() 
{
	MWTS_ENTER;

}

NetworkTest::~NetworkTest()
{
	MWTS_ENTER;
}

void NetworkTest::OnInitialize()
{
	MWTS_ENTER;

	bCanRoam = false;

	m_pProcess=new QProcess();

	m_bHttpDownloadSuccess = false;

	connect(&httpmanager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));

	connect(&networkManager, SIGNAL(updateCompleted()),
		this, SLOT(configurationsUpdateCompleted()));
	connect(&networkManager, SIGNAL(configurationAdded(const QNetworkConfiguration)),
		this, SLOT(configurationAdded(const QNetworkConfiguration)));
	connect(&networkManager, SIGNAL(configurationChanged(const QNetworkConfiguration&)),
		this, SLOT(configurationsChanged(const QNetworkConfiguration&)));
	connect(&networkManager, SIGNAL(configurationRemoved(const QNetworkConfiguration&)),
		this, SLOT(configurationsRemoved(const QNetworkConfiguration&)));

	connect(m_pProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotError(QProcess::ProcessError)));
	connect(m_pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
	connect(m_pProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slotStateChanged(QProcess::ProcessState)));
	connect(m_pProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotReadStandardOutput()));
	connect(m_pProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadStandardOutput()));

	m_strDeviceIp = "";

	g_pLog->EnableDebug(true);
	g_pLog->EnableTrace(true);

	// Increase default timeout to 15sec
	this->SetTestTimeout(15000);

	m_bUpdateCompleted = false;
}

void NetworkTest::OnUninitialize()
{
	MWTS_ENTER;

	if(networkSession)
	{
		delete networkSession;
		networkSession = NULL;
	}

	if(m_pProcess)
	{
		delete m_pProcess;
		m_pProcess = NULL;
	}
}

/* QProcess slots  */
void NetworkTest::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
	MWTS_ENTER;

	qDebug() << "Process finished exitCode: " << exitCode << " status: " << exitStatus;

	switch ( exitStatus )
	{
		case QProcess::NormalExit:
			qDebug() << "The SSH process exited normally.";
			m_bResult = true;
			break;
		case QProcess::CrashExit:
			qDebug() << "The SSH process crashed.";
			m_bResult = false;
			break;
	}
	this->Stop();

	MWTS_LEAVE;
}

void NetworkTest::slotError(QProcess::ProcessError error)
{
	MWTS_ENTER;
	qCritical() << "Error: " << error;
	QString errorStr;
	switch(error)
	{
		case QProcess::FailedToStart:
			errorStr = "Failed to start process";
			break;
		case QProcess::Crashed:
			errorStr = "Crashed";
			break;
		case QProcess::Timedout:
			errorStr = "Timed out";
			m_pProcess->kill();
			break;
		case QProcess::WriteError:
			errorStr = "Failed to write";
			break;
		case QProcess::ReadError:
			errorStr = "Failed to read";
			break;
		case QProcess::UnknownError:
		default:
			errorStr = "Unknown error";
			break;
	}

	m_bResult = false;
	MWTS_LEAVE;
}

void NetworkTest::slotReadStandardOutput()
{
	MWTS_ENTER;
	QByteArray temp = m_pProcess->readAllStandardOutput();
	m_baStdOut = temp;
	temp = m_pProcess->readAllStandardError();
	m_baStdErr.append(temp);

	qDebug() << "out:" << m_baStdOut;
	qDebug() << "err:" << m_baStdErr;
	MWTS_LEAVE;
}

void NetworkTest::slotStateChanged  ( QProcess::ProcessState newState )
{
	MWTS_ENTER;
	switch ( newState )
	{
		case QProcess::NotRunning:
			qDebug() << "The SSH process is not running.";
			break;
		case QProcess::Starting:
			qDebug() << "The SSH process is starting, but the program has not yet been invoked.";
			break;
		case QProcess::Running:
			qDebug() << "The SSH process is running and is ready for reading and writing.";
			break;
	}
	MWTS_LEAVE;
}

/*
 * QProcess slots  */

bool NetworkTest::ServerLaunchIperf(const char* time)
{
	MWTS_ENTER;

	m_bResult = true;
	m_baStdErr.clear();

	QStringList params;

	QString ip, username;

	ip = g_pConfig->value("SERVER/server_ip").toString();
	username = g_pConfig->value("SERVER/server_user").toString();
	//identity = g_pConfig->value("SERVER/identity_file").toString();

	if(ip.isNull() || username.isNull())
	{
		qCritical() << "Missing server information(ip, username), please check your NetworkTest.cfg";
		return false;
	}

	//iperf -c <ip> -t <length in seconds> -f m
	params << "-i" << username + "@" + ip << "/usr/bin/iperf" << "-c" << m_strDeviceIp << "-t" << time << " -f" << "m";

	qDebug() << "Starting SSH with params" << params;
	m_pProcess->start("/usr/bin/ssh", params);
	qDebug() << "Started process with pid" << m_pProcess->pid();

	// Start waiting
	this->Start();

	MWTS_LEAVE;
	return m_bResult;
}

bool NetworkTest::IsOnline()
{
	QNetworkConfigurationManager mgr;
	QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
	if (activeConfigs.count() > 0)
		return true;
	else
		return false;
}

bool NetworkTest::IsAccessPointFound(const QString ap_name)
{
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> configs = mgr.allConfigurations();

    // fetch ap ssid from NetworkTest.conf
    const QString ap = g_pConfig->value(ap_name + "/name").toString();

    qDebug() << "Looking if scan found " << ap;

    const int size = configs.size();
    for (int i = 0; i < size; ++i) {
        if(configs.at(i).name() == ap)
        {
            qDebug() << "Access point with name: " << ap << " was found!";
            return true;
        }

    }
	
    qCritical() << "Access point was not found in scan. Aborting...";
    return false;
}

bool NetworkTest::Downloadfile(const QString strFilename)
{
	MWTS_ENTER;

	bool success = false;

	// if we are not online, then there is no point to go further
	if(!IsOnline())
	{
		qCritical() << "No connection available, aborting file Download!";
		return false;
	}

	qDebug() << "Nice we have a connection!";

	FtpWindow ftpObject(this);

	QString ip, username, password;

	ip = g_pConfig->value("FTP/ip").toString();
	username = g_pConfig->value("FTP/username").toString();
	password = g_pConfig->value("FTP/password").toString();

	if(ip.isNull() || username.isNull() || password.isNull())
	{
		qCritical() << "Missing ftp information(ip, pass or username), please check you NetworkTest.cfg";
		return false;
	}

	qDebug() << "Connecting to ftp host " << ip << " with " << username << "/" << password;
	success = ftpObject.connectToHost(ip, username, password);

	if(!success)
	{
		qDebug() << "Login failed!";
		return false;
	}

	success = ftpObject.downloadFile(strFilename);

	if(!success)
	{
		qDebug() << "File download failed!";
		return false;
	}
	else
	{
		g_pResult->AddMeasure("Download", ftpObject.transferSpeed(), "MB/s");
	}

	MWTS_LEAVE;
	return success;
}

bool NetworkTest::UploadFile(const QString strFilename)
{
	MWTS_ENTER;

	bool success = false;

	// if we are not online, then there is no point to go further
	if(!IsOnline())
	{
		qCritical() << "No connection available, aborting file Upload!";
		return false;
	}

	qDebug() << "Nice we have a connection!";


	FtpWindow ftpObject(this);

	QString ip, username, password;

	ip = g_pConfig->value("FTP/ip").toString();
	username = g_pConfig->value("FTP/username").toString();
	password = g_pConfig->value("FTP/password").toString();

	if(ip.isNull() || username.isNull() || password.isNull())
	{
		qCritical() << "Missing ftp information (ip, pass or username), please check you NetworkTest.cfg";
		return false;
	}

	qDebug() << "Connecting to ftp host " << ip << " with username " << username;
	success = ftpObject.connectToHost(ip, username, password);

	if(!success)
	{
		qDebug() << "Login failed!";
		return false;
	}

	success = ftpObject.uploadFile(strFilename);

	if(!success)
	{
		qDebug() << "File upload failed!";
		return false;
	}
	else
	{
		g_pResult->AddMeasure("Upload", ftpObject.transferSpeed(), "MB/s");
	}

	MWTS_LEAVE;
	return success;
}

void NetworkTest::CloseActiveSessions()
{
	qDebug() << "Closing active sessions";

        QList<QNetworkConfiguration> configs = networkManager.allConfigurations();
        for (int i = 0; i < configs.size(); ++i) {
		if(configs.at(i).state() == QNetworkConfiguration::Active)
		{
		   QNetworkSession session(configs.at(i));
		   session.disconnect();
		   session.stop();
		   session.close();
		}
        }
}

bool NetworkTest::AddApPassword(const QString ap_name)
{

	qDebug() << "Opening Connman service interface";

	QDBusInterface manager("org.moblin.connman", "/", "org.moblin.connman.Manager", QDBusConnection::systemBus());

	QDBusReply<QVariantMap> reply = manager.call("GetProperties");

	if(!reply.isValid())
	{
		qCritical() << "Reply is inValid!";
		return false;
	}

	QVariantMap map = reply;   
	QVariant services;

	// get the Services map value
	if (map.contains("Services"))
	{
		qDebug() << "Reply contains Services!";
		services = map.value("Services");   
	}
    
	// cast Services to a string list 
	QStringList services_list = qdbus_cast<QStringList>(services); 

        QString name = g_pConfig->value(ap_name + "/name").toString();

	qDebug() << "Ap: " << ap_name << " is: " << name;
	
	// variantmap vor service getproperties replies
	map = QVariantMap();

	// preserve the service path for the found service
	//QString service_path = "";
	for(int i=0;i<services_list.count();i++)
	{
		QDBusInterface service_iface("org.moblin.connman", services_list.at(i), "org.moblin.connman.Service", QDBusConnection::systemBus());
		reply = service_iface.call("GetProperties");
       		
		if(reply.isValid())
		{
			map = reply;
	
			QMapIterator<QString, QVariant>iter(map);
			while (iter.hasNext()) {
				iter.next();
				if(iter.key() == "Name" && iter.value() == name) // service name matches with our access point name 
				{
					qDebug() << "/////////";
					qDebug() << "Found service named: " << name << " . Path is : " << services_list.at(i); 
				        
					// open interface to the service 
			                QDBusInterface iface("org.moblin.connman", services_list.at(i), "org.moblin.connman.Service", QDBusConnection::systemBus());

					QString name = g_pConfig->value(ap_name + "/name").toString();
					QString security = g_pConfig->value(ap_name + "/wlan_security").toString();
					QString pass = "";

					// if null, then it is wepkey
					if( security == "wpa" )
					{
						qDebug() << "Encryption type wpa";
						pass = g_pConfig->value(ap_name + "/EAP_wpa_preshared_passphrase").toString();
					}
					else if( security == "wep")
					{
						qDebug() << "Encryption type: wep.";
                                                pass = g_pConfig->value(ap_name + "/wlan_wepkey1").toString();						
					}
					else 
					{
						qDebug() << "Ap is open, no need to add passphrase";
					}

					qDebug() << "Password: " << pass;

					QString property = "Passphrase";
					QList<QVariant> args;

					args << qVariantFromValue(property) << QVariant::fromValue(QDBusVariant(pass));
					QDBusMessage pass_reply = iface.callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);
					
					return true;
				}
			}
		}
	}
   

	return false;
}

bool NetworkTest::AddApPasswords()
{
   QList<QString> ap_list;
   ap_list.append("unsecured_ap");
   ap_list.append("wep_ap");
   ap_list.append("wpa_tkip_ap");

   for(int i=0;i<ap_list.count();i++)
   {
      qDebug() << "List: " << ap_list.at(i);
      AddApPassword(ap_list.at(i));
   }

   return true;
}

bool NetworkTest::SwitchWlan(const QString state)
{
	MWTS_ENTER;

	qDebug() << "Trying to switch wlan0 device state to: " << state;
	bool bState = false;
	if(state == "on")
	{
		bState = true;
	}
	else if(state == "off")
	{	
		bState = false;
	}
	else 
	{
		qDebug() << "state is either on or off. Aborting.";
		return false;
	}

        QDBusInterface manager("org.moblin.connman", "/", "org.moblin.connman.Manager", QDBusConnection::systemBus());

        QDBusReply<QVariantMap> reply = manager.call("GetProperties");

        if(!reply.isValid())
        {
                qCritical() << "Reply is inValid!";
                return false;
        }

        QVariantMap map = reply;
        QVariant technologies;

        // get the Services map value
        if (map.contains("Technologies"))
        {
                qDebug() << "Reply contains Technologies!";
                technologies = map.value("Technologies");
        }

        QStringList tech_list = qdbus_cast<QStringList>(technologies);

        QDBusReply<QVariantMap> tech_reply;
	QDBusMessage dev_reply;

	qDebug() << "Techs are: " << tech_list;	

	QVariant devices;
        for(int i=0;i<tech_list.count();i++)
        {
                qDebug() << "Opening tech interface";
		QDBusInterface tech_iface("org.moblin.connman", tech_list.at(i), "org.moblin.connman.Technology", QDBusConnection::systemBus());
                tech_reply = tech_iface.call("GetProperties");
		
		if(!tech_reply.isValid())
		{
			qDebug() << "Technologies reply invalid, aborting...";
			return false;
		}

		//qDebug() << "Got technology properties, proceeding to open device interface";
		QVariantMap tech_map = tech_reply;

	        if(tech_map.contains("Devices"))
		{
			//qDebug() << "Devices found"; 
			if(tech_map.value("Type") == "wifi")
			{
				qDebug() << "Proceeding the state change";
				qDebug() << "type is : " << tech_map.value("Type");

		                devices = tech_map.value("Devices");  
        			// cast Devices to a string list
			        QStringList wifi_dev = qdbus_cast<QStringList>(devices);

				qDebug() << "Wifi device path is: " << wifi_dev.at(0); 
				QDBusInterface dev_iface("org.moblin.connman", wifi_dev.at(0),"org.moblin.connman.Device", QDBusConnection::systemBus());

   		                dev_reply = dev_iface.call("GetProperties");
				qDebug() << "Got reply for the device: " << dev_reply;

                                QString property = "Powered";
                                QList<QVariant> args;

                                args << qVariantFromValue(property) << QVariant::fromValue(QDBusVariant(bState));
                                QDBusMessage switch_reply = dev_iface.callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);

				qDebug() << "::" << switch_reply;
                                return true;
			}
		}

	}

	MWTS_LEAVE;
        return false;
}



void NetworkTest::StopSession()
{
	MWTS_ENTER;
	qDebug() << "Stopping session.!";
	
        if(networkSession != 0)
	{
		qDebug() << "Session was open, now closing it!";
		qDebug() << "Active time    : " << networkSession->activeTime() << " seconds" ;
		qDebug() << "Bytes 	written  : " << networkSession->bytesWritten();
		qDebug() << "Bytes received : " << networkSession->bytesReceived();
	        
                networkSession->disconnect();
         	networkSession->stop();
		this->Start();
		networkSession->close();
	}
	else
	{
		// no session available. Go trough configurations and close all active sessions
		CloseActiveSessions();
	}

	MWTS_LEAVE;
}



void NetworkTest::RunIdle()
{
	this->Start();
}


void NetworkTest::UpdateConfigs()
{
	MWTS_ENTER;

	qDebug() << "UPDATING CONFIGURATIONS / SCANNING FOR AVAILABLE ACCESS POINTS....";
	m_bUpdateCompleted = false;

	QSignalSpy spy(&networkManager, SIGNAL(updateCompleted()));
	networkManager.updateConfigurations();

	// wait for the updateCompleted signal
	TRY_VERIFY(spy.count() == 1, "Scan: Verify that updateCompleted is emitted once");

	MWTS_LEAVE;
}

void NetworkTest::configurationsUpdateCompleted()
{
	MWTS_ENTER;
	qDebug() << "CONFIGURATIONS UPDATE  / SCAN COMPLETE!";
	m_bUpdateCompleted = true;
	MWTS_LEAVE;
}

bool NetworkTest::IsUpdateComplete()
{
	MWTS_ENTER;
	return m_bUpdateCompleted;
	MWTS_LEAVE;
}

bool NetworkTest::DownloadfileHttp(const QString strUrl)
{
	MWTS_ENTER;

	qDebug() << "Attempting a http file download with url: " << strUrl;

	QNetworkRequest request(strUrl);
	QNetworkReply *reply = httpmanager.get(request);

	// needed signals for reply
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		 this, SLOT(slotError(QNetworkReply::NetworkError)));

	qDebug() << "Waiting for the download to finish...";

	this->Start();

	// this is set in the downloadFinished-slot
	return m_bHttpDownloadSuccess;

	MWTS_LEAVE;
}

bool NetworkTest::saveToDisk(const QString &filename, QIODevice *data)
{
	QFile file(filename);
	QDir::setCurrent("/root");

	if (!file.open(QIODevice::WriteOnly)) {
		qDebug() << "Could not open " << filename << " for writing \n";

		return false;
	}

	file.write(data->readAll());
	file.close();

	return true;
}

QString NetworkTest::saveFileName(const QUrl &url)
{
	QString path = url.path();
	QString basename = QFileInfo(path).fileName();

	if (basename.isEmpty())
		basename = "download";

	if (QFile::exists(basename)) {
		// already exists, don't overwrite
		int i = 0;
		basename += '.';
		while (QFile::exists(basename + QString::number(i)))
			++i;

		basename += QString::number(i);
	}

	return basename;
}

/*
 * HTTP SLOTS
 */
void NetworkTest::downloadFinished(QNetworkReply *reply)
{
	MWTS_ENTER;

	this->Stop();
	qDebug() << "Download finished from url: " << reply->url();

	QUrl url = reply->url();
	if (reply->error()) {
		qDebug() << "Download failed!";
		qDebug() << "Error: " << reply->errorString();
	} else {
		QString filename = saveFileName(url);
		if (saveToDisk(filename, reply))
		{
			qDebug() << "Download success. Saved to file: " << filename;
			m_bHttpDownloadSuccess = true;
		}

	}

	// take care of destroying the object
	reply->deleteLater();

	MWTS_LEAVE;
}

void NetworkTest::slotError(QNetworkReply::NetworkError error)
{
	MWTS_ENTER;

	switch(error)
	{
		case QNetworkReply::NoError:
			qDebug() << "QNetworkReply NoError";
		break;
		case QNetworkReply::ConnectionRefusedError:
			qDebug() << "QNetworkReply ConnectionRefusedError";
		break;
		case QNetworkReply::RemoteHostClosedError:
			qDebug() << "QNetworkReply RemoteHostClosedError";
		break;
		case QNetworkReply::TimeoutError:
			qDebug() << "QNetworkReply TimeoutError";
		break;
		case QNetworkReply::HostNotFoundError:
			qDebug() << "QNetworkReply HostNotFoundError";
		break;
		case QNetworkReply::ProtocolFailure:
			qDebug() << "QNetworkReply ProtocolFailure";
		break;
		default:
			qDebug() << "QNetworkReply Error";
		break;
	}

	MWTS_LEAVE;
}

/*
 * HTTP SLOTS
 * ********************/

/*
bool NetworkTest::IsAccessPointFound(QString ap_name)
{
	MWTS_ENTER;
	qDebug() << "Checking if " << ap_name << " is valid.";
	qDebug() << "Validity:  " << networkManager.configurationFromIdentifier(ap_name).isValid();
	qDebug() << "State:  " << networkManager.configurationFromIdentifier(ap_name).state();

	if((networkManager.configurationFromIdentifier(ap_name).state() ==  QNetworkConfiguration::Undefined &&
		networkManager.configurationFromIdentifier(ap_name).isValid()) ||
		(networkManager.configurationFromIdentifier(ap_name).state() == QNetworkConfiguration::Discovered &&
		networkManager.configurationFromIdentifier(ap_name).isValid()))
	{
		qDebug() << "Given access point is in discovered or undefined state and it is also valid, so proceeding...";
		return true;
	}

	return false;
	MWTS_LEAVE;
} */


void NetworkTest::ListConfigurations()
{
	//MWTS_ENTER;
	qDebug() << "LISTING AVAILABLE ACCESS POINT CONFIGURATIONS:";

	QList<QNetworkConfiguration> configs = networkManager.allConfigurations();
	for (int i = 0; i < configs.size(); ++i) {
		debugPrintConfiguration(configs.at(i));
	}
}

bool NetworkTest::ConnectToDefault()
{
	MWTS_ENTER;
	bool retVal = ConnectToConfig(networkManager.defaultConfiguration());
	MWTS_LEAVE;
	return retVal;
}

QString NetworkTest::ConfigurationByName(const QString name)
{
        MWTS_ENTER;
	QString identifier = "";

	QList<QNetworkConfiguration> configs = networkManager.allConfigurations();
	for (int i=0; i< configs.size(); ++i) {
		if (name == configs.at(i).name() ) {
			qDebug() << "Found " << name;
			return configs.at(i).identifier();
		}
	}

        qDebug() << "No identifier for ap named: " << name << " found.";

        return identifier;
}


bool NetworkTest::ConnectToName(QString ap_name)
{
	MWTS_ENTER;

	QString ap;
	ap = g_pConfig->value(ap_name + "/name").toString();
	
	if(ap.isNull())
	{
		qCritical() << "no ap with name " << ap_name << " found from config-file! Check the NetworkTest.conf.";
		return false;
	}

        qDebug() << "AP found, proceeding...";

	UpdateConfigs();        

	AddApPasswords();

	QString ap_identifier = ConfigurationByName(ap);
	networkConfiguration = networkManager.configurationFromIdentifier(ap_identifier);

	qDebug() << "connecting to config named: " << networkConfiguration.name() << " and identifier: " << networkConfiguration.identifier();	

	bool retval = false;
	retval = ConnectToConfig(networkConfiguration);
	MWTS_LEAVE;
	return retval;
}


QNetworkConfiguration NetworkTest::GetConfigurationByName(QString ap_name)
{
	MWTS_ENTER;

	// browse trough list of configurations and return one with matching name
	QList<QNetworkConfiguration> configs = networkManager.allConfigurations();
	for (int i =0; i< configs.size(); ++i) {
		if (ap_name == configs.at(i).name() ) {
			qDebug() << "Found " << ap_name;
			return configs.at(i);
		} else {
			qDebug() << "Name: " << configs.at(i).name() << " - Not matching " << ap_name;
		}
	}
	qDebug() << "Couldn't find " << ap_name << " using default configuration";
	return networkManager.defaultConfiguration();
}

bool NetworkTest::ConnectToConfig(const QNetworkConfiguration& config)
{
	MWTS_ENTER;

	if (networkSession != 0)
	{
		qDebug("Closing existing session!");
		networkSession->stop();
		MWTS_LEAVE;
		return false;
	}

	// debug list available configs
	ListConfigurations();

	// do verify that connection is valid and in discovered or active state
	if(!config.isValid())
	{
		qDebug() << "Configuration not valid, so it is not propably seen by scan...";
		qCritical() << "Configuration is not valid, aborting";
		MWTS_LEAVE;
		return false;
	}

 	if(config.state() != QNetworkConfiguration::Discovered && config.state() != QNetworkConfiguration::Active)
	{
		qCritical() << "Configuration is not in Discovered or Active state, aborting";
		MWTS_LEAVE;
		return false;
	}

	qDebug() << "CONFIGURATION VALID CONNECTING TO";
	debugPrintConfiguration(config);


	networkSession = new QNetworkSession(config);
	connect(networkSession, SIGNAL(stateChanged(QNetworkSession::State)),
		this, SLOT(networkStateChanged(QNetworkSession::State)));
	connect(networkSession, SIGNAL(error(QNetworkSession::SessionError)),
		this, SLOT(networkError(QNetworkSession::SessionError)));

	// Setting this property to true before calling open() implies that the connection attempt is made but if no connection can be established,
	// the user is not consulted and asked to select a suitable connection. This property is not set by default and support for it depends on the platform.
	networkSession->setSessionProperty ("ConnectInBackground", false);

	if(bCanRoam)
	{
		qDebug() << "Roaming is set on. Asset will now change to more suitable configuration on the fly...";
		connect(networkSession, SIGNAL(newConfigurationActivated()),
			this, SLOT(newConfigurationActivatedHandler()));
		connect(networkSession, SIGNAL(preferredConfigurationChanged(const QNetworkConfiguration & config, bool isSeamless)),
		 	this, SLOT(preferredConfigurationChangedHandler(const QNetworkConfiguration & config, bool isSeamless)));
	}

	g_pTime->start();
	networkSession->open();

	if (networkSession->waitForOpened(10000))
	{
		qDebug("Success, Connection established!");

		double elapsed=g_pTime->elapsed();
		qDebug() << "Connection establishing took: " << elapsed << " ms";

		g_pResult->AddMeasure("Connection latency", elapsed, "ms");

		QString interfaceName = networkSession->interface().humanReadableName();
		qDebug() << "Interface: " << interfaceName;

		QString bearer = config.bearerName();
		qDebug() << "Bearer: " << bearer;

		QString name = config.name();
		qDebug() << "Name: " << name;

		QString identifier = config.identifier();
		qDebug() << "Identifier: " << identifier;

		switch(config.purpose())
		{
			case QNetworkConfiguration::UnknownPurpose:
				qDebug() << "Purpose: Unknown";
				break;
			case QNetworkConfiguration::PublicPurpose:
				qDebug() << "Purpose: Public";
				break;
			case QNetworkConfiguration::PrivatePurpose:
				qDebug() << "Purpose: Private";
				break;
			case QNetworkConfiguration::ServiceSpecificPurpose:
				qDebug() << "Purpose: Service Specific";
				break;
			default:
				qDebug() << "Purpose not availableN/A";
				break;
		}

		QList<QNetworkConfiguration> childs = config.children();

		if(childs.size() < 1)
		{
			qDebug() << "No children, this is not a service network.:";
		}
		else
		{
			// just print the list of children
			for (int i = 0; i < childs.size(); ++i) {
				qDebug() << childs.at(i).name();
			}
		}

		qDebug() << "Roaming is available: " << config.isRoamingAvailable();

		if(config.type() == 0)
			qDebug() << "Type: Internet access point";

		// finally accept the current iap connection
		networkSession->accept();

		if(bCanRoam)
		{
			qDebug() << "Roaming is set on, checking for more suitable ap..";
			this->Start();
		}

	} // if
	else
	{
		if (networkSession)
		{
			networkSession->stop();
			delete networkSession;
			networkSession = 0;
			qCritical() << "The session was not opened in 10 seconds.";
			return false;
		}
	}

	MWTS_LEAVE;
	return true;
}

void NetworkTest::debugPrintConfiguration(QNetworkConfiguration config)
{
		qDebug() <<     "///// ACCESS POINT //////// " <<config.name();
		qDebug() <<     "  Name: " <<config.name();
		qDebug() <<     "  Identifier: " <<config.identifier();
		QString valid = config.isValid()?"TRUE":"FALSE";
		qDebug() <<     "  isValid: " << valid;
		QString type =  "  Type: ";
		QString state = "  State: ";

		QList<QNetworkConfiguration> childs = config.children();


		if(childs.size() < 1)
		{
			qDebug() << "  Children: no children, this is not a service network";
		}
		else
		{
			// just print the list of children
			for (int i = 0; i < childs.size(); ++i) {
				qDebug() << childs.at(i).name();
			}
		}

		switch(config.type()) {
			case QNetworkConfiguration::InternetAccessPoint:
				type += "Internet Access Point"; break;
			case QNetworkConfiguration::ServiceNetwork:
				type += "Service Network";break;
			case QNetworkConfiguration::UserChoice:
				type += "User choice"; break;
			case QNetworkConfiguration::Invalid:
				type += "Invalid";break;
			default:
				type += "N/A"; break;
		}
		switch(config.state())
		{
			case QNetworkConfiguration::Undefined:
				state += "Undefined (Found, but no configuration exists in the device)";break;
			case QNetworkConfiguration::Defined:
				state += "Defined (Configurations is found)";break;
			case QNetworkConfiguration::Discovered:
				state += "Discovered (Connectable)";break;
			case QNetworkConfiguration::Active:
				state += "Active";break;
			default:
				state += "N/A";break;
		}
		qDebug() << "  " << type;
		qDebug() << "  " << state;
		MWTS_LEAVE;
}

bool NetworkTest::FindIp(const QString strInterface)
{
	MWTS_ENTER;

	bool success = false;

	foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	{
		if (interface.flags().testFlag(QNetworkInterface::IsRunning))
		{
			foreach (QNetworkAddressEntry entry, interface.addressEntries())
			{
				if ( interface.hardwareAddress() != "00:00:00:00:00:00" &&     entry.ip().toString().contains("."))
				{
					if(interface.name() == strInterface)
					qDebug() << "interface " << interface.name() << " found! ip is: " << entry.ip().toString();
					m_strDeviceIp = entry.ip().toString();

					success = true;
					//items << interface.name() + entry.ip().toString();
				}

			}
		}
	}

	MWTS_LEAVE;
	return success;
}

// SLOTS

void NetworkTest::newConfigurationActivatedHandler()
{
	MWTS_ENTER;
	this->Stop();

	qDebug() << "New configuration activated. Going to accept()";
	networkSession->accept();

	MWTS_LEAVE;
}

void NetworkTest::preferredConfigurationChangedHandler( const QNetworkConfiguration & config, bool isSeamless )
{
	MWTS_ENTER;
	this->Stop();

	qDebug() << "=============================";
	qDebug() << "Found more preferrable configuration: -> " << config.bearerName();
	qDebug() << "Seamless: " << isSeamless;
	qDebug() << "Migrating with it! ==>";

	// waiting for newConfigurationActivated-signal
	this->Start();

	MWTS_LEAVE;
}

void NetworkTest::configurationAdded(const QNetworkConfiguration& config)
{
	MWTS_ENTER;
	qDebug()<< "ADDED CONFIGURATION:" << config.name() << " State: " << config.state();
	MWTS_LEAVE;
}

void NetworkTest::configurationsChanged(const QNetworkConfiguration& config)
{
	MWTS_ENTER;
	//qDebug()<< "CHANGED CONFIGURATION:" << config.name() << " State: " << config.state();
	//qDebug()<< "Config changed: ";
	//debugPrintConfiguration(config);
	MWTS_LEAVE;
}

void NetworkTest::configurationsRemoved(const QNetworkConfiguration& config)
{
	MWTS_ENTER;
	qDebug()<< "REMOVED CONFIGURATION:" << config.name() << " State: " << config.state();
	MWTS_LEAVE;
}

void NetworkTest::networkStateChanged(QNetworkSession::State state)
{
	MWTS_ENTER;
	QString status = "Network State Changed to: ";
	switch(state) {
		case QNetworkSession::Invalid:
			status += "Invalid"; break;
		case QNetworkSession::NotAvailable:
			status += "Not available"; break;
		case QNetworkSession::Connecting:
			status += "Connecting"; break;
		case QNetworkSession::Connected:
			status += "Connected"; break;
		case QNetworkSession::Closing:
			status += "Closing"; break;
		case QNetworkSession::Disconnected:
			status += "Disconnected";
			if (networkSession)
			{
				networkSession->deleteLater();
				networkSession = 0;
			}
			this->Stop();
			break;
		case QNetworkSession::Roaming:
			status += "Roaming";
			break;
		default:
			status += "Unknown";
			break;
	}
	qDebug() << status;
	MWTS_LEAVE;
}

void NetworkTest::networkError(QNetworkSession::SessionError error)
{
	MWTS_ENTER;
	QString text = "Connection error: ";
	switch(error)
	{
		case QNetworkSession::UnknownSessionError:
			text += "Unknown"; break;
		case QNetworkSession::SessionAbortedError:
			text += "Aborted"; break;
		case QNetworkSession::RoamingError:
			text += "Roaming error"; break;
		case QNetworkSession::OperationNotSupportedError:
			text += "Operation not supported"; break;
		case QNetworkSession::InvalidConfigurationError:
			text += "Invalid configuration"; break;
	}
	qDebug() << text;
	MWTS_LEAVE;
}

