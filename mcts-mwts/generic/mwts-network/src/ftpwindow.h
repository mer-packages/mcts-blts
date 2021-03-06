/* ftpwindow.cpp -- interface of the ftpwindow class
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


#include <QtCore>
#include <qftp.h>
#include "stable.h"
#include <MwtsCommon>
#include "NetworkTest.h"

class QFtp;

class FtpWindow : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    FtpWindow(MwtsTest *testAsset);

    /**
     * Destructor
     */
    ~FtpWindow();

    /**
     * Connect to target ftp host
     * @return true/false if succeeded
     */
    bool connectToHost(const QString ip);

    /**
     * Login to target ftp host
     * @return true/false if succeeded
     */
    bool login(const QString username,const QString password);

    /**
     * Download file from ftp server
     * @param strFilename name of the downloadable file
     * @return true/false if succeeded
     */
    bool downloadFile(QString strFilename);

    /**
     * Upload file to ftp server
     * @param strFilename name of the uploaded file
     * @return true/false if succeeded
     */
    bool uploadFile(QString strFilename);

    /**
     * Returns last transfer speed
     * Speed is in bytes/sec
     * @return bytes/sec
     */
    inline double transferSpeed(){ return m_fTransferSpeed; }

protected slots:

    /**
     * Slots for QFtp signals
     */
    void ftp_commandStarted(int id);
    void ftp_commandFinished(int id, bool error);
    void ftp_stateChanged(int state);
    void ftp_done(bool error);
    void ftp_listInfo(const QUrlInfo &info);
    void updateDataTransferProgress(qint64 readBytes, qint64 totalBytes);

private:

    void destroy();
    void logError();

private:

    MwtsTest	*m_pTestAsset;
    QFile		*m_pFile;
    QFtp        *m_pFtp;


    QString		m_sDownloadPath;
    QString		m_sUploadPath;
    QString		m_sLocalPath;
    qint64 		m_iSizeOfFile;
    double		m_fTransferSpeed;
    bool        m_bResult;
};
