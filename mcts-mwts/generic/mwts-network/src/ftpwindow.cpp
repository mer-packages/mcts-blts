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
{
	MWTS_ENTER;

	this->testAsset = testAsset;

	ftp = new QFtp( this );

	connect( ftp, SIGNAL(commandStarted(int)),
			SLOT(ftp_commandStarted()) );
	connect( ftp, SIGNAL(commandFinished(int,bool)),
			SLOT(ftp_commandFinished()) );
	connect( ftp, SIGNAL(stateChanged(int)),
			SLOT(ftp_stateChanged(int)) );
	connect( ftp, SIGNAL(done(bool)),
			SLOT(ftp_done(bool)) );
	connect( ftp, SIGNAL(listInfo(const QUrlInfo &)),
			SLOT(ftp_listInfo(const QUrlInfo &)) );
	connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
			this, SLOT(updateDataTransferProgress(qint64, qint64)));


	this->downloadPath = g_pConfig->value("FTP/download_path").toString();
	this->uploadPath = g_pConfig->value("FTP/upload_path").toString();
	this->localPath = g_pConfig->value("FTP/local_path").toString();

	MWTS_LEAVE;
}

/*
 * Destructor
 */
FtpWindow::~FtpWindow()
{
	if(ftp)
	{
		// make sure the connection is shut
		this->destroy();
		delete ftp;
		ftp = NULL;
	}

	if(file)
	{
		delete file;
		file = NULL;
	}
}

void FtpWindow::ftp_listInfo( const QUrlInfo &i )
{
	// stop the event loop
	testAsset->Start();

	qDebug() << "///////";
	qDebug() << i.name();
	qDebug() << i.size();
	qDebug() << i.owner();
	qDebug() << i.group();
	qDebug() << i.lastModified().toString("MMM dd yyyy");
}

bool FtpWindow::downloadFile(QString strFilename)
{
	file = new QFile(strFilename);
	if (!file->open(QIODevice::WriteOnly)) {
		qCritical() << "Error in file creation";
		delete file;
		return false;
	}

	bResult = true;
	fTransferSpeed = 0;

	if(!this->downloadPath.isNull())
	{
		qDebug() << "changing ftp path to " << this->downloadPath;
		ftp->cd(this->downloadPath);
	}

	qDebug() << "Starting to download file" << strFilename;

	g_pTime->restart();

	ftp->get(strFilename, file);
	// stop the event loop
	testAsset->Start();

	// elapsed time is in milliseconds
	double transTime = g_pTime->elapsed();

	fTransferSpeed = (sizeOfFile / 1024) / 1024 / (transTime/1000);

	return bResult;
}

bool FtpWindow::uploadFile(QString strFilename)
{
	if ( strFilename.isNull() )
	{
		return false;
	}

	QString localPath = strFilename;
	bResult = true;
	fTransferSpeed = 0;


	if(!this->localPath.isNull())
	{
		localPath = this->localPath + "/" + strFilename;
	}


	qDebug() << "Reading file " << localPath;
	file = new QFile(localPath);
	if (!file->open(QIODevice::ReadOnly)) {
		qCritical() << "Error in file reading";
		delete file;
		return false;
	}

	testAsset->SetFailTimeout( 120000 );

	if(!this->uploadPath.isNull())
	{
		qDebug() << "changing ftp path to " << this->uploadPath;
		ftp->cd(this->uploadPath);
	}

	qDebug() << "Starting to upload file " << strFilename;


	g_pTime->restart();
	ftp->put( file, strFilename );

	// start the event loop
	testAsset->Start();

	// elapsed time is in milliseconds
	double transTime = g_pTime->elapsed();

	fTransferSpeed = (sizeOfFile / 1024) / 1024 / (transTime/1000);

	return bResult;
}

void FtpWindow::updateDataTransferProgress(qint64 readBytes,
											qint64 totalBytes)
{
	sizeOfFile = totalBytes;
	qDebug() << "Transfered:" << readBytes << "/" << totalBytes << " bytes";
}

void FtpWindow::ftp_commandStarted()
{
	qDebug() << "FTP Command Started";

	switch(ftp->currentCommand())
	{
		case QFtp::ConnectToHost:
			qDebug() << "Connect to host command";
		break;
		case QFtp::Login:
			qDebug() << "Login command!";
		break;
		case QFtp::Close:
			qDebug() << "Close command!";
		break;
		case QFtp::List:
			qDebug() << "List command!";
		break;
		case QFtp::Put:
			qDebug() << "Put command!";
		break;
		case QFtp::Get:
			qDebug() << "Get command!";
			break;
		default:
			qDebug() << "Command started " << ftp->currentCommand();
	}
}

void FtpWindow::ftp_commandFinished()
{
	qDebug() << "FTP Command finished";

	switch(ftp->currentCommand())
	{
		case QFtp::ConnectToHost:
			qDebug() << "Connect to host command finished";
		break;
		case QFtp::Login:
			qDebug() << "Login succeeded";
			// stop the event loop
			testAsset->Stop();
		break;
		case QFtp::Close:
			qDebug() << "Close command finished";
		break;
		case QFtp::List:
			qDebug() << "List command finished";
		break;
		case QFtp::Put:
			qDebug() << "Put command finished";
			// stop the event loop
			testAsset->Stop();
		break;
		case QFtp::Get:
			qDebug() << "Get command finished";
			// stop the event loop
			testAsset->Stop();
			break;
		default:
			qDebug() << "Command finished " << ftp->currentCommand();
	}
}

void FtpWindow::ftp_stateChanged( int state )
{
	switch ( (QFtp::State)state ) {
		case QFtp::Unconnected:
			qDebug() << "State is : Unconnected";
			testAsset->Stop();
			break;
		case QFtp::HostLookup:
			qDebug() << "State is : Looking Host";
			break;
		case QFtp::Connecting:
			qDebug() << "State is : Connecting";
			break;
		case QFtp::Connected:
			qDebug() << "State is : Connected";
			break;
		case QFtp::LoggedIn:
			qDebug() << "State is : Logged in";
			break;
		case QFtp::Closing:
			qDebug() << "State is: Closing";
			break;
	}

}

void FtpWindow::ftp_done( bool error )
{
	if ( error )
	{

		switch(error) {
			case QFtp::HostNotFound:
				qDebug() << "Error: host not found";
			break;
			case QFtp::ConnectionRefused:
				qDebug() << "Error: connection refused";
			break;
			case QFtp::NotConnected:
				qDebug() << "Error: not connected";
			break;
			case QFtp::UnknownError:
				qDebug() << "Error: unknown";
			break;
		}
		bResult = false;
		// If we are connected, but not logged in, it is not meaningful to stay connected
		if ( ftp->state() == QFtp::Connected )
		{
			ftp->close();
		}
		testAsset->Stop();
	}
}

bool FtpWindow::connectToHost(const QString ip, const QString username,const QString password)
{
	qDebug() << "FtpWindow::connectToHost";

	// close the existing connection
	if ( ftp->state() != QFtp::Unconnected )
		ftp->close();

	qDebug() << "Connecting to host " << ip;
	ftp->connectToHost( ip );
	ftp->login( username, password );

	testAsset->Start();

	if(ftp->state() == QFtp::LoggedIn)
	{
		return true;
	}

	return false;
}

void FtpWindow::destroy()
{
	qDebug() << "Closing FTP connection....";

	if ( ftp->state() != QFtp::Unconnected )
	{
		ftp->close();
		testAsset->Start();
	}
}



