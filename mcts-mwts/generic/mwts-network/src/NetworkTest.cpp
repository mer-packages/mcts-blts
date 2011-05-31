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
    : m_pConnmanManager(NULL),
    m_pNetworkSession(NULL),
    m_pHttpReply(NULL),
    m_pHttpDownloadFile(NULL),
    m_pProcess(NULL)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

NetworkTest::~NetworkTest()
{
    MWTS_ENTER;

    if(m_pNetworkSession)
    {
        qDebug() << "Closing session";
        m_pNetworkSession->close();
        qDebug() << "Destroying session";
        delete m_pNetworkSession;
        m_pNetworkSession = NULL;
    }

    if(m_pProcess)
    {
        delete m_pProcess;
        m_pProcess = NULL;
    }

    MWTS_LEAVE;
}

void NetworkTest::OnInitialize()
{
    MWTS_ENTER;

    bCanRoam = false;
    m_bResult = false;

    m_pProcess=new QProcess();

    m_bHttpDownloadSuccess = false;


    // connman dbus manager
        m_pConnmanManager = new QDBusInterface("net.connman", "/", "net.connman.Manager", QDBusConnection::systemBus());

    connect(&m_HttpManager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(downloadFinished(QNetworkReply*)));

    connect(&m_NetworkManager, SIGNAL(updateCompleted()),
            this, SLOT(configurationsUpdateCompleted()));
    connect(&m_NetworkManager, SIGNAL(configurationAdded(const QNetworkConfiguration)),
            this, SLOT(configurationAdded(const QNetworkConfiguration)));
    connect(&m_NetworkManager, SIGNAL(configurationChanged(const QNetworkConfiguration&)),
            this, SLOT(configurationsChanged(const QNetworkConfiguration&)));
    connect(&m_NetworkManager, SIGNAL(configurationRemoved(const QNetworkConfiguration&)),
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
    // this->SetTestTimeout(15000);

    m_bUpdateCompleted = false;
}

void NetworkTest::OnUninitialize()
{
    MWTS_ENTER;

    if (m_pNetworkSession) {
        delete m_pNetworkSession;
        m_pNetworkSession = NULL;
    }

    if (m_pProcess)	{
        delete m_pProcess;
        m_pProcess = NULL;
    }

    if (m_pConnmanManager) {
        delete m_pConnmanManager;
        m_pConnmanManager = NULL;
    }
}

/* QProcess slots  */
void NetworkTest::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    MWTS_ENTER;

    qDebug() << "Process finished exitCode: " << exitCode << " status: " << exitStatus;

    switch ( exitStatus ) {
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

    switch(error) {
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
    switch ( newState ) {
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

    if(ip.isNull() || username.isNull()) {
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
        if (configs.at(i).name() == ap) {
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
    if(!IsOnline()) {
            qCritical() << "No connection available, aborting file Download!";
            return false;
    }

    qDebug() << "Nice we have a connection!";

    FtpWindow ftpObject(this);

    QString ip, username, password;

    ip = g_pConfig->value("FTP/ip").toString();
    username = g_pConfig->value("FTP/username").toString();
    password = g_pConfig->value("FTP/password").toString();

    if(ip.isNull() || username.isNull() || password.isNull()) {
        qCritical() << "Missing ftp information(ip, pass or username), please check you NetworkTest.cfg";
        return false;
    }

    qDebug() << "Connecting to ftp host " << ip << " with " << username << "/" << password;
    success = ftpObject.connectToHost(ip, username, password);

    if(!success) {
        qDebug() << "Login failed!";
        return false;
    }

    success = ftpObject.downloadFile(strFilename);

    if(!success) {
        qDebug() << "File download failed!";
        return false;
    }
    else {
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
    if(!IsOnline()) {
        qCritical() << "No connection available, aborting file Upload!";
        return false;
    }

    qDebug() << "Nice we have a connection!";

    FtpWindow ftpObject(this);

    QString ip, username, password;

    ip = g_pConfig->value("FTP/ip").toString();
    username = g_pConfig->value("FTP/username").toString();
    password = g_pConfig->value("FTP/password").toString();

    if(ip.isNull() || username.isNull() || password.isNull()) {
        qCritical() << "Missing ftp information (ip, pass or username), please check you NetworkTest.cfg";
        return false;
    }

    qDebug() << "Connecting to ftp host " << ip << " with username " << username;
    success = ftpObject.connectToHost(ip, username, password);

    if(!success) {
        qDebug() << "Login failed!";
        return false;
    }

    success = ftpObject.uploadFile(strFilename);

    if(!success) {
        qDebug() << "File upload failed!";
        return false;
    }
    else {
        g_pResult->AddMeasure("Upload", ftpObject.transferSpeed(), "MB/s");
    }

    MWTS_LEAVE;
    return success;
}

void NetworkTest::CloseActiveSessions()
{
    qDebug() << "Closing active sessions";

    QList<QNetworkConfiguration> configs = m_NetworkManager.allConfigurations();
    for (int i = 0; i < configs.size(); ++i) {
        if (configs.at(i).state() == QNetworkConfiguration::Active) {
           QNetworkSession session(configs.at(i));
           session.disconnect();
           session.stop();
           session.close();
        }
    }
}

bool NetworkTest::SwitchWlan(const QString state)
{
    MWTS_ENTER;

    qDebug() << "Trying to switch wlan0 device state to: " << state;
    bool bState = false;

    if (state == "on") {
        bState = true;
    } else if (state == "off") {
        bState = false;
    } else 	{
        qDebug() << "state is either on or off. Aborting.";
        return false;
    }

    qDebug() << "Getting manager properties...";
    QDBusReply<QVariantMap> reply = m_pConnmanManager->call("GetProperties");
    if (!reply.isValid())
    {
        QDBusError error = reply.error();
        qDebug() << "Received invalid DBUS reply";
        qDebug() << "Error message:" << error.message();

        qCritical() << "Could not get manager properties, check connman-daemon. Aborting...";
        return false;
    }

    QVariantMap map = reply;
    QVariant technologies;

    if (map.contains("Technologies")) {
        qDebug() << "Reply contains Technologies!";
        technologies = map.value("Technologies");
    }

    QStringList tech_list = qdbus_cast<QStringList>(technologies);

    QDBusReply<QVariantMap> tech_reply;
    QDBusMessage dev_reply;

    qDebug() << "Techs are: " << tech_list;

    QVariant devices;
    for (int i=0;i<tech_list.count();i++) {
        qDebug() << "Opening tech interface";
        QDBusInterface tech_iface("net.connman", tech_list.at(i), "net.connman.Technology", QDBusConnection::systemBus());
        tech_reply = tech_iface.call("GetProperties");

        if (!tech_reply.isValid())
        {
            QDBusError error = tech_reply.error();
            qDebug() << "Received invalid DBUS reply";
            qDebug() << "Error message:" << error.message();

            qDebug() << "Technologies reply invalid, aborting...";
            return false;
        }

        //qDebug() << "Got technology properties, proceeding to open device interface";
        QVariantMap tech_map = tech_reply;

        if (tech_map.contains("Devices"))
        {
            //qDebug() << "Devices found";
            if (tech_map.value("Type") == "wifi") {
                qDebug() << "Proceeding the state change";
                qDebug() << "type is : " << tech_map.value("Type");

                devices = tech_map.value("Devices");
                // cast Devices to a string list
                QStringList wifi_dev = qdbus_cast<QStringList>(devices);

                qDebug() << "Wifi device path is: " << wifi_dev.at(0);
                QDBusInterface dev_iface("net.connman", wifi_dev.at(0),"net.connman.Device", QDBusConnection::systemBus());

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



bool NetworkTest::StopSession(const QString ap_name)
{
    MWTS_ENTER;
    qDebug() << "Disconnection method!";

    QString ap = g_pConfig->value(ap_name + "/name").toString();

    QString service_path = GetServicePath(ap_name);
    if (service_path == "")
    {
        qCritical() << "No service available, can be because of the daemon, or the ap is just not seen currently. aborting";
        return false;
    }
    qDebug() << "Service path is:" << service_path;

    //make sure the signal is connected
    m_pConnmanManager->connection().connect("net.connman", "/", "net.connman.Manager", "StateChanged",
                                          this, SLOT(slotConnmanStateChanged(QString)));

    // set the flag telling if we are connecting or disconnecting
    m_bConnecting = false;

    // open interface to the service
    QDBusInterface iface("net.connman", service_path, "net.connman.Service", QDBusConnection::systemBus());

    qDebug() << "Trying to disconnect...";
    g_pTime->start();
    QDBusMessage disconnect_reply = iface.call("Disconnect");
    if ( disconnect_reply.type() == QDBusMessage::ErrorMessage )
    {
        if (disconnect_reply.errorMessage() == "Not connected")
        {
            qDebug() << "Not connected. No need to disconnect. All good.";
            QString msg = "Access point '" + ap + "' not connected. Nothing to do.";
            g_pResult->Write(msg);
            return true;
        }
        else
        {
            qCritical() << "Disconnecting error:" << disconnect_reply.errorMessage();
            return false;
        }
    }

    // set this to false, we dont know if disconnecting works yet
    m_bResult = false;

    qDebug() << "Waiting for the connection state change....";
    this->Start();

    if(m_pNetworkSession != 0)
    {
            qDebug() << "Session was open, now closing it!";
            qDebug() << "Active time    : " << m_pNetworkSession->activeTime() << " seconds" ;
            qDebug() << "Bytes 	written  : " << m_pNetworkSession->bytesWritten();
            qDebug() << "Bytes received : " << m_pNetworkSession->bytesReceived();

            m_pNetworkSession->disconnect();
            m_pNetworkSession->stop();
            this->Start();
            m_pNetworkSession->close();
    }
    else
    {
            // no session available. Go trough configurations and close all active sessions
            CloseActiveSessions();
    }

    MWTS_LEAVE;
    return m_bResult;
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

    QSignalSpy spy(&m_NetworkManager, SIGNAL(updateCompleted()));
    m_NetworkManager.updateConfigurations();

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

    // reset the byte counter used in logging filter
    m_iHttpDownloadCounter = 0;

    // open the file for downloaded file
    QNetworkRequest request(strUrl);
    QString filename = saveFileName(request.url());
    m_pHttpDownloadFile = new QFile(filename);
    if( !m_pHttpDownloadFile->open(QIODevice::WriteOnly) )
    {
        qDebug() << "Could not open" << filename << "for writing!";
        delete m_pHttpDownloadFile;
        m_pHttpDownloadFile = NULL;
        return false;
    }

    // make the request
    g_pTime->start();
    m_pHttpReply = m_HttpManager.get(request);

    // needed signals for reply
    connect(m_pHttpReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(slotDownloadProgress(qint64, qint64)));
    connect(m_pHttpReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));

    qDebug() << "Waiting for the download to finish...";

    this->Start();

    // close the downloaded file
    m_pHttpDownloadFile->close();
    delete m_pHttpDownloadFile;
    m_pHttpDownloadFile = NULL;

    MWTS_LEAVE;

    // this is set in the downloadFinished-slot
    return m_bHttpDownloadSuccess;
}

QString NetworkTest::saveFileName(const QUrl &url)
{
    QString basepath = g_pConfig->value("HTTP/local_path").toString();
    if(!basepath.endsWith("/"))
    {
        basepath += "/";
    }

    QString filename = QFileInfo(url.path()).fileName();
    if (filename.isEmpty())
    {
        filename = "downloaded";
    }
    QString fullpath = basepath + filename;

    if( QFile::exists(fullpath) )
    {
        // already exists, don't overwrite
        int i = 1;
        fullpath += '.';
        while( QFile::exists(fullpath + QString::number(i)) )
        {
            ++i;
        }

        fullpath += QString::number(i);
    }

    return fullpath;
}

/*
 * HTTP SLOTS
 */
void NetworkTest::downloadFinished(QNetworkReply *reply)
{
    MWTS_ENTER;

    qDebug() << "Download finished from url: " << reply->url();

    double transTime = g_pTime->elapsed();
    double sizeInMB = (double)m_iHttpDownloadFileSize / (double)(1024*1024);
    double timeInSeconds = (double)transTime / (double)1000;
    double transferSpeed = sizeInMB / timeInSeconds;
    g_pResult->AddMeasure("Http download", transferSpeed, "MB/s");

    m_bHttpDownloadSuccess = true;

    // take care of destroying the object
    reply->deleteLater();

    this->Stop();

    MWTS_LEAVE;
}

void NetworkTest::slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_iHttpDownloadFileSize = bytesTotal;

    // write log only after each received megabyte
    if( (bytesReceived - m_iHttpDownloadCounter) > (1024*1024) )
    {
        qDebug() << bytesReceived/(1024*1024) << "MB /" << bytesTotal/(1024*1024) << "MB";
        m_iHttpDownloadCounter = bytesReceived;
    }

    m_pHttpDownloadFile->write(m_pHttpReply->readAll());
}

void NetworkTest::slotError(QNetworkReply::NetworkError error)
{
    MWTS_ENTER;

    switch(error) {
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
    qDebug() << "Validity:  " << m_NetworkManager.configurationFromIdentifier(ap_name).isValid();
    qDebug() << "State:  " << m_NetworkManager.configurationFromIdentifier(ap_name).state();

    if((m_NetworkManager.configurationFromIdentifier(ap_name).state() ==  QNetworkConfiguration::Undefined &&
        m_NetworkManager.configurationFromIdentifier(ap_name).isValid()) ||
        (m_NetworkManager.configurationFromIdentifier(ap_name).state() == QNetworkConfiguration::Discovered &&
        m_NetworkManager.configurationFromIdentifier(ap_name).isValid()))
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

    QList<QNetworkConfiguration> configs = m_NetworkManager.allConfigurations();
    for (int i = 0; i < configs.size(); ++i) {
        debugPrintConfiguration(configs.at(i));
    }
}

bool NetworkTest::ConnectToDefault()
{
    MWTS_ENTER;
    bool retVal = ConnectToConfig(m_NetworkManager.defaultConfiguration());

    MWTS_LEAVE;
    return retVal;
}

QString NetworkTest::ConfigurationByName(const QString name)
{
    MWTS_ENTER;
    QString identifier = "";

    QList<QNetworkConfiguration> configs = m_NetworkManager.allConfigurations();
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

    if(ap.isNull()) {
        qCritical() << "no ap with name " << ap_name << " found from config-file! Check the NetworkTest.conf.";
        return false;
    }

    qDebug() << "AP found, proceeding...";

    UpdateConfigs();

    QString ap_identifier = ConfigurationByName(ap);
    m_NetworkConfiguration = m_NetworkManager.configurationFromIdentifier(ap_identifier);

    qDebug() << "connecting to config named: " << m_NetworkConfiguration.name() << " and identifier: " << m_NetworkConfiguration.identifier();

    bool retval = false;
    retval = ConnectToConfig(m_NetworkConfiguration);

    MWTS_LEAVE;
    return retval;
}
bool NetworkTest::SetTethering(const QString mode)
{
    MWTS_ENTER;

    bool bMode = false;

    if (mode == "on") {
        bMode = true;
    } else if (mode == "off") {
        bMode = false;
    } else {
        qCritical() << "Tethering should be either of or off. Aborting...";
        return false;
    }

    qDebug() << "Opening Connman manager interface";

    //QDBusInterface manager("net.connman", "/", "net.connman.Manager", QDBusConnection::systemBus());

    QString property = "Tethering";
    QList<QVariant> args;

    args << qVariantFromValue(property) << QVariant::fromValue(QDBusVariant(bMode));
    QDBusMessage tethering_reply = m_pConnmanManager->callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);

    qDebug() << "Reply from SetTethering: " << tethering_reply;

    MWTS_LEAVE;
    return true;
}

QString NetworkTest::GetServicePath(const QString ap_name)
{
    QString name = g_pConfig->value(ap_name + "/name").toString();
    qDebug() << "Ap:" << ap_name << " is:" << name;

    qDebug() << "Opening Connman manager interface";
    QDBusInterface maanager("net.connman", "/", "net.connman.Manager", QDBusConnection::systemBus());

    qDebug() << "Getting manager properties...";
    QDBusReply<QVariantMap> reply = m_pConnmanManager->call("GetProperties");
    if (!reply.isValid())
    {
        QDBusError error = reply.error();
        qDebug() << "Received invalid DBUS reply";
        qDebug() << "Error message:" << error.message();

        qCritical() << "Could not get manager properties, check connman-daemon. Aborting...";
        return "";
    }

    QVariantMap map = reply;
    QVariant services;
    if (map.contains("Services"))
    {
        qDebug() << "Manager properties contain Services";
        services = map.value("Services");
    }
    // cast Services to a string list
    QStringList services_list = qdbus_cast<QStringList>(services);

    for (int i=0; i<services_list.count(); i++)
    {
        QDBusInterface service_iface("net.connman", services_list.at(i), "net.connman.Service", QDBusConnection::systemBus());
        reply = service_iface.call("GetProperties");

        if (reply.isValid())
        {
            map = reply;

            QMapIterator<QString, QVariant>iter(map);
            while (iter.hasNext())
            {
                iter.next();
                if (iter.key() == "Name" && iter.value() == name)
                {
                    // service name matches with our access point name
                    qDebug() << "Found service named:" << name;
                    return services_list.at(i);
                }
            }
        }
    }

    return "";
}

bool NetworkTest::ConnmanConnection(const QString ap_name)
{
    MWTS_ENTER;

    QString service_path = GetServicePath(ap_name);
    qDebug() << "Service path is:" << service_path;

    if (service_path == "")
    {
        qCritical() << "No service available, can be because of the daemon, or the ap is just not seen currently. aborting";
        return false;
    }

    // connect the connman state signal to slot
    m_pConnmanManager->connection().connect("net.connman", "/", "net.connman.Manager", "StateChanged",
                                          this, SLOT(slotConnmanStateChanged(QString)));

    // set the flag telling if we are connecting or disconnecting
    m_bConnecting = true;

    // open interface to the service
    QDBusInterface iface("net.connman", service_path, "net.connman.Service", QDBusConnection::systemBus());

    QString propertyName = "";
    QList<QVariant> args;

    if( ap_name == "psd_network")
    {
        qDebug() << "You are connecting to a psd-network. Lets set the apn...";
        QString apnName = g_pConfig->value(ap_name + "/apn").toString();

        if ( apnName == "")
        {
            qCritical() << "Apn returned null from config-file. apn must be set! Aborting...";
            return false;
        }
        qDebug() << "Apn: " << apnName;

        propertyName = "APN";
        args.clear();
        args << QVariant::fromValue(propertyName) << QVariant::fromValue(QDBusVariant(apnName));
        qDebug() << "Setting APN...";
        QDBusMessage apn_reply = iface.callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);
        if( apn_reply.type() == QDBusMessage::ErrorMessage )
        {
            qCritical() << "Apn set error: " << apn_reply.errorMessage();
            return false;
        }

        qDebug() << "Apn set for " << ap_name << " . Proceeding";
    }

    QString name     = g_pConfig->value(ap_name + "/name").toString();
    QString security = g_pConfig->value(ap_name + "/wlan_security").toString();
    QString pass     = "";

    if ( ap_name != "psd_network")
    {
        if (security == "wpa" )
        {
            pass = g_pConfig->value(ap_name + "/EAP_wpa_preshared_passphrase").toString();
            qDebug() << "Encryption type WPA";
            qDebug() << "Passphrase:" << pass;
        }
        else if ( security == "wep")
        {
            pass = g_pConfig->value(ap_name + "/wlan_wepkey1").toString();
            qDebug() << "Encryption type WEP";
            qDebug() << "Passphrase:" << pass;
        }
        else
        {
            qDebug() << "Ap is open, using empty passphrase.";
        }
    }

    propertyName = "Passphrase";
    args.clear();
    args << QVariant::fromValue(propertyName) << QVariant::fromValue(QDBusVariant(pass));
    qDebug() << "Setting passphrase...";
    QDBusMessage  pass_reply = iface.callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);
    if ( pass_reply.type() == QDBusMessage::ErrorMessage )
    {
        qCritical() << "Passphrase set error: " << pass_reply.errorMessage();
        return false;
    }

    g_pTime->start();
    qDebug() << "Trying to connect to service";
    QDBusMessage connect_reply = iface.call("Connect");
    if ( connect_reply.type() == QDBusMessage::ErrorMessage )
    {
        if (connect_reply.errorMessage() == "Already connected")
        {
            qDebug() << "Already connected. No need to connect. All good.";
            QString msg = "Access point '" + name + "' already connected. Nothing to do.";
            g_pResult->Write(msg);
            return true;
        }
        else
        {
            qCritical() << "Connecting error:" << connect_reply.errorMessage();
            return false;
        }
    }

    qDebug() << "Waiting for the connection state change....";
    this->Start();

    // set AutoConnect property to false
    propertyName = "AutoConnect";
    bool autoConnectValue = false;
    args.clear();
    args << QVariant::fromValue(propertyName) << QVariant::fromValue(QDBusVariant(autoConnectValue));
    qDebug() << "Setting AutoConnect property to false...";
    QDBusMessage autoconnect_reply = iface.callWithArgumentList(QDBus::AutoDetect, QLatin1String("SetProperty"), args);
    if ( autoconnect_reply.type() == QDBusMessage::ErrorMessage )
    {
        qCritical() << "AutoConnect set error: " << autoconnect_reply.errorMessage();
        return false;
    }

    MWTS_LEAVE;
    return m_bResult;
}

bool NetworkTest::RemoveService(const QString ap_name)
{
    MWTS_ENTER;

    QString ap;
    ap = g_pConfig->value(ap_name + "/name").toString();

    QString service_path = GetServicePath(ap_name);
    qDebug() << "Service path is: " << service_path;

    if (service_path == "")
    {
        qCritical() << "No path to service. Aborting";
        return false;
    }

    // open interface to the service
    QDBusInterface iface("net.connman", service_path, "net.connman.Service", QDBusConnection::systemBus());

    QString name = g_pConfig->value(ap_name + "/name").toString();

    qDebug() << "Removing service configuration....";
    QDBusMessage remove_reply = iface.call("Remove");
    if ( remove_reply.type() == QDBusMessage::ErrorMessage )
    {
        qCritical() << "Service remove error: " << remove_reply.errorMessage();
        return false;
    }

    MWTS_LEAVE;
    return true;
}

QNetworkConfiguration NetworkTest::GetConfigurationByName(QString ap_name)
{
    MWTS_ENTER;

    // browse trough list of configurations and return one with matching name
    QList<QNetworkConfiguration> configs = m_NetworkManager.allConfigurations();
    for (int i =0; i< configs.size(); ++i) {
        if (ap_name == configs.at(i).name() ) {
            qDebug() << "Found " << ap_name;
            return configs.at(i);
        } else {
            qDebug() << "Name: " << configs.at(i).name() << " - Not matching " << ap_name;
        }
    }
    qDebug() << "Couldn't find " << ap_name << " using default configuration";
    return m_NetworkManager.defaultConfiguration();
}

bool NetworkTest::ConnectToConfig(const QNetworkConfiguration& config)
{
    MWTS_ENTER;

    if (m_pNetworkSession != 0) {
        qDebug("Closing existing session!");
        m_pNetworkSession->stop();
        MWTS_LEAVE;
        return false;
    }

    // debug list available configs
    ListConfigurations();

    // do verify that connection is valid and in discovered or active state
    if(!config.isValid()) {
        qDebug() << "Configuration not valid, so it is not propably seen by scan...";
        qCritical() << "Configuration is not valid, aborting";
        MWTS_LEAVE;
        return false;
    }

    if(config.state() != QNetworkConfiguration::Discovered && config.state() != QNetworkConfiguration::Active) {
        qCritical() << "Configuration is not in Discovered or Active state, aborting";
        MWTS_LEAVE;
        return false;
    }

    qDebug() << "CONFIGURATION VALID CONNECTING TO";
    debugPrintConfiguration(config);


    m_pNetworkSession = new QNetworkSession(config);
    connect(m_pNetworkSession, SIGNAL(stateChanged(QNetworkSession::State)),
            this, SLOT(networkStateChanged(QNetworkSession::State)));
    connect(m_pNetworkSession, SIGNAL(error(QNetworkSession::SessionError)),
            this, SLOT(networkError(QNetworkSession::SessionError)));

    // Setting this property to true before calling open() implies that the connection attempt is made but if no connection can be established,
    // the user is not consulted and asked to select a suitable connection. This property is not set by default and support for it depends on the platform.
    m_pNetworkSession->setSessionProperty ("ConnectInBackground", false);

    if (bCanRoam) {
        qDebug() << "Roaming is set on. Asset will now change to more suitable configuration on the fly...";
        connect(m_pNetworkSession, SIGNAL(newConfigurationActivated()),
                this, SLOT(newConfigurationActivatedHandler()));
        connect(m_pNetworkSession, SIGNAL(preferredConfigurationChanged(const QNetworkConfiguration & config, bool isSeamless)),
                this, SLOT(preferredConfigurationChangedHandler(const QNetworkConfiguration & config, bool isSeamless)));
    }

    g_pTime->start();
    m_pNetworkSession->open();

    if (m_pNetworkSession->waitForOpened(10000)) {
        qDebug("Success, Connection established!");

        double elapsed=g_pTime->elapsed();
        qDebug() << "Connection establishing took: " << elapsed << " ms";

        g_pResult->AddMeasure("Connection latency", elapsed, "ms");

        QString interfaceName = m_pNetworkSession->interface().humanReadableName();
        qDebug() << "Interface: " << interfaceName;

        QString bearer = config.bearerName();
        qDebug() << "Bearer: " << bearer;

        QString name = config.name();
        qDebug() << "Name: " << name;

        QString identifier = config.identifier();
        qDebug() << "Identifier: " << identifier;

        switch(config.purpose()) {
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

        if (childs.size() < 1) {
            qDebug() << "No children, this is not a service network.:";
        }
        else {
            // just print the list of children
            for (int i = 0; i < childs.size(); ++i) {
                qDebug() << childs.at(i).name();
            }
        }

        qDebug() << "Roaming is available: " << config.isRoamingAvailable();

        if(config.type() == 0)
            qDebug() << "Type: Internet access point";

        // finally accept the current iap connection
        m_pNetworkSession->accept();

        if (bCanRoam) {
            qDebug() << "Roaming is set on, checking for more suitable ap..";
            this->Start();
        }

    } // if
    else
    {
        if (m_pNetworkSession) {
            m_pNetworkSession->stop();
            delete m_pNetworkSession;
            m_pNetworkSession = 0;
            qCritical() << "The session was not opened in 10 seconds.";
            return false;
        }
    }

    MWTS_LEAVE;
    return true;
}

void NetworkTest::debugPrintConfiguration(QNetworkConfiguration config)
{
    qDebug() << "///// ACCESS POINT /////";
    qDebug() << "Name:" << config.name();
    qDebug() << "Identifier:" << config.identifier();
    qDebug() << "Valid:" << (config.isValid() ? "TRUE" : "FALSE");

    QList<QNetworkConfiguration> children = config.children();
    qDebug() << "Children:" << children.size();
    if(children.size() < 1)
    {
        qDebug() << "    No children, this is not a service network";
    }
    else
    {
        // just print the list of children
        for (int i = 0; i < children.size(); ++i)
        {
            qDebug() << "    " << children.at(i).name();
        }
    }

    QString type =  "Type: ";
    switch( config.type() )
    {
    case QNetworkConfiguration::InternetAccessPoint:
        type += "Internet Access Point";
        break;
    case QNetworkConfiguration::ServiceNetwork:
        type += "Service Network";
        break;
    case QNetworkConfiguration::UserChoice:
        type += "User choice";
        break;
    case QNetworkConfiguration::Invalid:
        type += "Invalid";
        break;
    default:
        type += "N/A";
        break;
    }
    qDebug() << type;

    debugPrintConfigState(config.state());

    MWTS_LEAVE;
}

void NetworkTest::debugPrintConfigState(QNetworkConfiguration::StateFlags state)
{
    qDebug() << "State:";
    if( state.testFlag(QNetworkConfiguration::Undefined) )
    {
        qDebug() << "    Undefined (Found, but no configuration exists in the device)";
    }
    if( state.testFlag(QNetworkConfiguration::Defined) )
    {
        qDebug() << "    Defined (Configurations is found)";
    }
    if( state.testFlag(QNetworkConfiguration::Discovered) )
    {
        qDebug() << "    Discovered (Connectable)";
    }
    if( state.testFlag(QNetworkConfiguration::Active) )
    {
        qDebug() << "    Active";
    }
}

bool NetworkTest::FindIp(const QString strInterface)
{
    MWTS_ENTER;
    bool success = false;

    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsRunning)) {
            foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
                if ( interface.hardwareAddress() != "00:00:00:00:00:00" &&     entry.ip().toString().contains("."))
                {
                    if(interface.name() == strInterface)
                    qDebug() << "interface " << interface.name() << " found! ip is: " << entry.ip().toString();
                    m_strDeviceIp = entry.ip().toString();

                    success = true;
                }

            }
        }
    }

    MWTS_LEAVE;
    return success;
}

// SLOTS

void NetworkTest::slotConnmanStateChanged(QString state)
{
    MWTS_ENTER;
    qDebug() << "State of Connman Manager changed to:" << state;

    if( m_bConnecting && (state.indexOf("online") >=0) )
    {
        // we were waiting for state "online"

        double elapsed=g_pTime->elapsed();
        qDebug() << "Connection establishing took:" << elapsed << "ms";
        g_pResult->AddMeasure("Connect Latency", elapsed, "ms");

        m_bResult = true;

        // we can disconnect the signal for now
        m_pConnmanManager->connection().disconnect("net.connman", "/", "net.connman.Manager", "StateChanged",
                                              this, SLOT(slotConnmanStateChanged(QString)));

        this->Stop();
    }
    else if( !m_bConnecting && (state.indexOf("offline") >=0) )
    {
        // we were waiting for state "offline"

        double elapsed=g_pTime->elapsed();
        qDebug() << "Disconnecting took:" << elapsed << "ms";
        g_pResult->AddMeasure("Disconnect Latency", elapsed, "ms");

        m_bResult = true;

        // we can disconnect the signal for now
        m_pConnmanManager->connection().disconnect("net.connman", "/", "net.connman.Manager", "StateChanged",
                                              this, SLOT(slotConnmanStateChanged(QString)));

        this->Stop();
    }

    MWTS_LEAVE;
}

void NetworkTest::newConfigurationActivatedHandler()
{
    MWTS_ENTER;
    this->Stop();

    qDebug() << "New configuration activated. Going to accept()";
    m_pNetworkSession->accept();

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
    qDebug() << "ADDED CONFIGURATION:" << config.name();
    debugPrintConfigState(config.state());
    MWTS_LEAVE;
}

void NetworkTest::configurationsChanged(const QNetworkConfiguration& config)
{
    MWTS_ENTER;
    qDebug() << "CHANGED CONFIGURATION:" << config.name();
    debugPrintConfigState(config.state());
    MWTS_LEAVE;
}

void NetworkTest::configurationsRemoved(const QNetworkConfiguration& config)
{
    MWTS_ENTER;
    qDebug()<< "REMOVED CONFIGURATION:" << config.name();
    debugPrintConfigState(config.state());
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
            if (m_pNetworkSession) {
                m_pNetworkSession->deleteLater();
                m_pNetworkSession = 0;
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
    switch(error) {
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

