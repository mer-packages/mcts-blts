/* ftpwindow.cpp -- ftpwindow class functionality
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
#include "ftpwindow.h"

/*
 * Constructor
 */
FtpWindow::FtpWindow(MwtsTest *testAsset)
    : m_pTestAsset(testAsset),
      m_pFile(NULL),
      m_iSizeOfFile(0),
      m_fTransferSpeed(0),
      m_bResult(false)
{
    MWTS_ENTER;

    m_pFtp = new QFtp(this);

    connect( m_pFtp, SIGNAL(commandStarted(int)),
             this, SLOT(ftp_commandStarted(int)) );
    connect( m_pFtp, SIGNAL(commandFinished(int,bool)),
             this, SLOT(ftp_commandFinished(int,bool)) );
    connect( m_pFtp, SIGNAL(stateChanged(int)),
             this, SLOT(ftp_stateChanged(int)) );
    connect( m_pFtp, SIGNAL(done(bool)),
             this, SLOT(ftp_done(bool)) );
    connect( m_pFtp, SIGNAL(listInfo(const QUrlInfo &)),
             this, SLOT(ftp_listInfo(const QUrlInfo &)) );
    connect( m_pFtp, SIGNAL(dataTransferProgress(qint64, qint64)),
             this, SLOT(updateDataTransferProgress(qint64, qint64)) );

    m_sDownloadPath = g_pConfig->value("FTP/download_path").toString();
    m_sUploadPath   = g_pConfig->value("FTP/upload_path").toString();
    m_sLocalPath    = g_pConfig->value("FTP/local_path").toString();

    MWTS_LEAVE;
}

/*
 * Destructor
 */
FtpWindow::~FtpWindow()
{
    if (m_pFtp)
    {
        // make sure the connection is shut
        this->destroy();
        delete m_pFtp;
    }

    if (m_pFile)
    {
        m_pFile->close();
        delete m_pFile;
    }
}

bool FtpWindow::connectToHost(const QString ip)
{
    // close the existing connection
    if ( m_pFtp->state() != QFtp::Unconnected )
    {
        qDebug() << "Closing an existing connection";
        m_pFtp->close();
        m_pTestAsset->Start();
    }

    qDebug() << "Connecting to host " << ip;
    m_pFtp->connectToHost(ip);
    m_pTestAsset->Start();

    if (m_pFtp->state() == QFtp::Connected)
    {
        return true;
    }
    return false;
}

bool FtpWindow::login(const QString username, const QString password)
{
    qDebug() << "Logging in with username:" << username << "and password:" << password;
    m_pFtp->login(username, password);
    m_pTestAsset->Start();

    if (m_pFtp->state() == QFtp::LoggedIn)
    {
        return true;
    }
    return false;
}

bool FtpWindow::downloadFile(QString strFilename)
{
    m_pFile = new QFile(strFilename);
    if (!m_pFile->open(QIODevice::WriteOnly))
    {
        qCritical() << "Error in file creation";
        delete m_pFile;
        m_pFile = NULL;
        return false;
    }

    m_fTransferSpeed = 0;

    if (!m_sDownloadPath.isNull())
    {
        qDebug() << "Changing ftp path to:" << m_sDownloadPath;
        m_pFtp->cd(m_sDownloadPath);
    }

    qDebug() << "Starting to download file:" << strFilename;

    g_pTime->restart();
    m_pFtp->get(strFilename, m_pFile);

    // start the event loop
    m_pTestAsset->Start();

    // elapsed time is in milliseconds
    double transTime = g_pTime->elapsed();
    double sizeInMB = (double)m_iSizeOfFile / (double)(1024*1024);
    double timeInSeconds = (double)transTime / (double)1000;
    m_fTransferSpeed = sizeInMB / timeInSeconds;

    return m_bResult;
}

bool FtpWindow::uploadFile(QString strFilename)
{
    if (strFilename.isNull())
    {
        return false;
    }

    QString fullPath = "";

    m_fTransferSpeed = 0;

    if (!m_sLocalPath.isNull())
    {
        fullPath = m_sLocalPath + "/" + strFilename;
    }

    qDebug() << "Reading file from:" << fullPath;
    m_pFile = new QFile(fullPath);
    if (!m_pFile->open(QIODevice::ReadOnly))
    {
        qCritical() << "Error in file reading";
        delete m_pFile;
        m_pFile = NULL;
        return false;
    }

    if (!m_sUploadPath.isNull())
    {
        qDebug() << "Changing ftp path to:" << m_sUploadPath;
        m_pFtp->cd(m_sUploadPath);
    }

    qDebug() << "Starting to upload file:" << strFilename;

    g_pTime->restart();
    m_pFtp->put(m_pFile, strFilename);

    // start the event loop
    m_pTestAsset->Start();

    // elapsed time is in milliseconds
    double transTime = g_pTime->elapsed();
    double sizeInMB = (double)m_iSizeOfFile / (double)(1024*1024);
    double timeInSeconds = (double)transTime / (double)1000;
    m_fTransferSpeed = sizeInMB / timeInSeconds;

    return m_bResult;
}

void FtpWindow::updateDataTransferProgress(qint64 readBytes, qint64 totalBytes)
{
    m_iSizeOfFile = totalBytes;
    qDebug() << "Transfered:" << readBytes << "/" << totalBytes << " bytes";
}

void FtpWindow::ftp_commandStarted(int /*id*/)
{
    qDebug() << "Started FTP Command:";

    switch( m_pFtp->currentCommand() )
    {
    case QFtp::ConnectToHost:
        qDebug() << "    ConnectToHost";
        break;
    case QFtp::Login:
        qDebug() << "    Login";
        break;
    case QFtp::Close:
        qDebug() << "    Close";
        break;
    case QFtp::List:
        qDebug() << "    List";
        break;
    case QFtp::Put:
        qDebug() << "    Put";
        break;
    case QFtp::Get:
        qDebug() << "    Get";
        break;
    default:
        qDebug() << "Unknown command:" << m_pFtp->currentCommand();
    }
}

void FtpWindow::ftp_commandFinished(int /*id*/, bool error)
{
    if( error )
    {
        switch( m_pFtp->currentCommand() )
        {
        case QFtp::ConnectToHost:
            qDebug() << "Connect to host finished with an error!";
            logError();
            m_pTestAsset->Stop();
            break;
        case QFtp::Login:
            qDebug() << "Login finished with an error!";
            logError();
            m_pTestAsset->Stop();
            break;
        case QFtp::Close:
            qDebug() << "Close finished with an error!";
            logError();
            m_pTestAsset->Stop();
            break;
        case QFtp::List:
            qDebug() << "List finished with an error!";
            logError();
            break;
        case QFtp::Put:
            qDebug() << "Put command finished with an error!";
            logError();
            m_bResult = false;
            m_pTestAsset->Stop();
            break;
        case QFtp::Get:
            qDebug() << "Get command finished with an error!";
            logError();
            m_bResult = false;
            m_pTestAsset->Stop();
            break;
        default:
            qDebug() << "There was an error with unrecognized command:" << m_pFtp->currentCommand();
        }
    }
    else
    {
        switch( m_pFtp->currentCommand() )
        {
        case QFtp::ConnectToHost:
            qDebug() << "Successfully connected to host";
            m_pTestAsset->Stop();
            break;
        case QFtp::Login:
            qDebug() << "Login succeeded";
            m_pTestAsset->Stop();
            break;
        case QFtp::Close:
            qDebug() << "Close succeeded";
            m_pTestAsset->Stop();
            break;
        case QFtp::List:
            qDebug() << "List succeeded";
            break;
        case QFtp::Put:
            qDebug() << "Put command succeeded";
            m_bResult = true;
            m_pTestAsset->Stop();
            break;
        case QFtp::Get:
            qDebug() << "Get command succeeded";
            m_bResult = true;
            m_pTestAsset->Stop();
            break;
        default:
            qDebug() << "Unrecognized command finished:" << m_pFtp->currentCommand();
        }
    }
}

void FtpWindow::ftp_stateChanged(int state)
{
    switch ( (QFtp::State)state )
    {
    case QFtp::Unconnected:
        qDebug() << "State is: Unconnected";
        m_pTestAsset->Stop();
        break;
    case QFtp::HostLookup:
        qDebug() << "State is: Looking Host";
        break;
    case QFtp::Connecting:
        qDebug() << "State is: Connecting";
        break;
    case QFtp::Connected:
        qDebug() << "State is: Connected";
        break;
    case QFtp::LoggedIn:
        qDebug() << "State is: Logged in";
        break;
    case QFtp::Closing:
        qDebug() << "State is: Closing";
        break;
    default:
        qDebug() << "Unrecognized state:" << state;
        break;
    }
}

void FtpWindow::ftp_done(bool error)
{
    if (error)
    {
        qDebug() << "All pending FTP commands have finished but there was an error!";
        logError();
        m_pTestAsset->Stop();
    }
    else
    {
        qDebug() << "All pending FTP commands have finished successfully!";
    }
}

void FtpWindow::ftp_listInfo(const QUrlInfo &i)
{
    // stop the event loop
    m_pTestAsset->Start();

    qDebug() << "///////";
    qDebug() << i.name();
    qDebug() << i.size();
    qDebug() << i.owner();
    qDebug() << i.group();
    qDebug() << i.lastModified().toString("MMM dd yyyy");
}


void FtpWindow::destroy()
{
    qDebug() << "Closing FTP connection....";

    if ( m_pFtp->state() != QFtp::Unconnected )
    {
        m_pFtp->close();
        m_pTestAsset->Start();
    }
}

void FtpWindow::logError()
{
    switch(m_pFtp->error())
    {
    case QFtp::HostNotFound:
        qDebug() << "Error: host not found";
        m_bResult = false;
        break;
    case QFtp::ConnectionRefused:
        qDebug() << "Error: connection refused";
        m_bResult = false;
        break;
    case QFtp::NotConnected:
        qDebug() << "Error: not connected";
        break;
    case QFtp::UnknownError:
        qDebug() << "Error: unknown error";
        m_bResult = false;
        break;
    default:
        qDebug() << "Unrecognized error:" << m_pFtp->error();
        m_bResult = false;
        break;
    }

    qDebug() << "Error string:" << m_pFtp->errorString();
}
