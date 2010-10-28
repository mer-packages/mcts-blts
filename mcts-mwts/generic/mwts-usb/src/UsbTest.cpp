/* UsbTest.cpp -- Contains usb test asset functionality
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
#include "UsbTest.h"

UsbTest::UsbTest() 
{

	MWTS_ENTER;
	MWTS_LEAVE;
}

UsbTest::~UsbTest()
{
}


void UsbTest::OnInitialize()
{
	MWTS_ENTER;

	m_pProcess=new QProcess();

	connect(m_pProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotError(QProcess::ProcessError)));
	connect(m_pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
	connect(m_pProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slotStateChanged(QProcess::ProcessState)));
	connect(m_pProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotReadStandardOutput()));
	connect(m_pProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadStandardOutput()));

	MWTS_LEAVE;

}

void UsbTest::OnUninitialize()
{
	MWTS_ENTER;
	/*
	if(m_pProcess)
	{
		m_pProcess->disconnect();
		delete m_pProcess;
		m_pProcess = NULL;
	}
	*/
	MWTS_LEAVE;
}

void UsbTest::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
	MWTS_ENTER;

	qDebug() << "Process finished exitCode: " << exitCode << " status: " << exitStatus;

	if(exitStatus == QProcess::CrashExit || exitCode != 0)
	{
		qCritical() << "Process crashed or invalid return code";
		m_bResult = false;
	}
	this->Stop();
	MWTS_LEAVE;
}

void UsbTest::slotError(QProcess::ProcessError error)
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
	this->Stop();
	MWTS_LEAVE;
}

void UsbTest::slotReadStandardOutput()
{
	MWTS_ENTER;
	QByteArray temp = m_pProcess->readAllStandardOutput();
	temp.append(m_pProcess->readAllStandardError());
	qDebug() << temp;
	m_baStdErr.append(temp);
	MWTS_LEAVE;
}

void UsbTest::slotStateChanged  ( QProcess::ProcessState newState )
{
	MWTS_ENTER;
	qDebug() << "Process state changed to " << newState;
	MWTS_LEAVE;
}


void UsbTest::Transfer(const char* a_destination,
	                   const char* a_source)
{
	MWTS_ENTER;
	
	m_bResult = true;
	m_baStdErr.clear();
	m_fReceiveSpeed = 0.0f;
	m_fSendSpeed = 0.0f;
	
	QStringList params;

	params << "-qBv" << a_source <<  a_destination;

	qDebug() << "Starting scp with params" << params;
	m_pProcess->start("/usr/bin/scp", params);
	qDebug() << "Started process with pid" << m_pProcess->pid();

	// Start waiting
	this->Start();

	ParseOutput();
	MWTS_LEAVE;
}

void UsbTest::ParseOutput()
{

	MWTS_ENTER;
	int i = 0, pos = 0;
	bool invalid = true;
	QString str = m_baStdErr;

	// Split lines
	QStringList lines = str.split("\n", QString::SkipEmptyParts);

	if (lines.length() >= 1)
	{
		for(i = 0; i <= lines.length();i++)
		{
			QString line = lines[i];

			// Find correct line
			pos = line.indexOf("Bytes per second", 0, Qt::CaseInsensitive);

			if(pos >= 0)
			{
				// Parse sent and received speeds
				line = line.right(line.size() - 23 - pos);

				lines = line.split(",");

				QString strSent = lines[0];
				QString strReceived = lines[1];
				strReceived = strReceived.right(strReceived.size() - 9);

				qDebug() << strSent;
				qDebug() << strReceived;

				// Scp logs bytes per seconds, change to MegaBytes per sec
				m_fSendSpeed = strSent.toFloat() / 1024 / 1024;
				m_fReceiveSpeed = strReceived.toFloat() / 1024 /1024;

				invalid = false;
				break;
			}
		}
	}

	if(invalid)
	{
		m_bResult = false;
		qCritical() << "Invalid output";
	}

	MWTS_LEAVE;
}



