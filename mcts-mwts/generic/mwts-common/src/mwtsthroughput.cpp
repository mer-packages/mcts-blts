/* mwtsthroughput.cpp --
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
#include "mwtsthroughput.h"

#define TEST_TIME	"180"
#define TEST_TIME_INT	180
/**
  Constructor of MwtsThroughput base class
*/

MwtsThroughput::MwtsThroughput()
{
	MWTS_ENTER;
	m_pProcess = new QProcess();
	MWTS_LEAVE;
}

MwtsThroughput::~MwtsThroughput()
{

	if ( m_pProcess )
	{
		m_pProcess->kill();
		m_pProcess->close();
		m_pProcess->disconnect();
		//delete m_pProcess;
		m_pProcess = NULL;
	}

}


/**
  Starts the iperf process. Runs Exec() which is implemented in sub classes.
*/
bool MwtsThroughput::Start()
{
	MWTS_ENTER;
	bool ret = Exec(); // implemented in subclasses
	MWTS_LEAVE;
	return ret;
}
/**
  Stops iperf. Kills the process running iperf. This is automatically called by the destructor.
*/
void MwtsThroughput::Stop()
{
	MWTS_ENTER;
	if ( m_pProcess->pid() != 0 )
	{
		qDebug() << "Killing the process...";
		m_pProcess->kill();
		m_pProcess->waitForFinished();
	}
	else
	{
		// This might happen, if the process exits cleanly.
		qDebug() << "PID == 0, can not kill!";
	}
	MWTS_LEAVE;
}



/* execution implementations */

/**
  Load execution implementation of throughput class
*/
bool MwtsIPerfThroughput::Exec()
{
	MWTS_ENTER;

	m_pReturnValue = false;

	// Connect to important signals
	connect( m_pProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)) );
	connect( m_pProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onFinished(int,QProcess::ExitStatus)) );
	connect( m_pProcess, SIGNAL(started()), SLOT(onStarted()) );
	connect( m_pProcess, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)) );


	qDebug() << "Starting iPerf with params" << m_params;
	if ( m_pIperfRole == IPERF_CLIENT )
	{
		qDebug() << "Starting iperf in client role.";
		m_pProcess->start("/usr/bin/iperf", m_params);
		qDebug() << "Wating for iperf to finish...";
		// We might use waitForFinished, but then we'd loose the ability to
		// use fail timeout
		//m_pProcess->waitForFinished(TEST_TIME_INT*1100);
		// Wait until iperf client has finished.
		g_pTest->Start();
		m_pProcess->disconnect();
	}
	else
	{
		qDebug() << "Starting iperf in server role.";
		m_pProcess->start("/usr/bin/iperf", m_params);
		qDebug() << "Wating for iperf to start...";
		// We might use waitForStarted, but then we'd loose the ability to
		// use fail timeout
		//m_pProcess->waitForStarted(TEST_TIME_INT*1100);
		// Wait until iperf server has started.
		g_pTest->Start();
		// Mark the return vaue to true, for successfull start
		m_pReturnValue = true;
	}

	MWTS_LEAVE;
	return m_pReturnValue;
}


/**
  Sets measurement to client mode
  @param a_ServerIP IP to connect
*/
void MwtsIPerfThroughput::SetClientMeasurement(char* a_ServerIP, int transtime, char* direction)
{
	MWTS_ENTER;

	// Make sure that we have valid time, event if the default is given
	int testtime = transtime;

	if (transtime <= 0 )
	{
		testtime = TEST_TIME_INT;
	}

	qDebug() << "Using time " << testtime;
	qDebug() << "Using direction: " << direction;

	// We are running client
	m_pIperfRole = IPERF_CLIENT;

	if(strncmp (direction,"bidirectional", 2) == 0)
	{
		// We are running client
		m_pIperfRole = IPERF_BICLIENT;
		// Clear any existing params before appending new
		m_params.clear();
		// starting with mode '-r' to enable bidirectional measuring
		m_params << "-t" << QString::number( testtime ) <<  "-f" << "m" << "-c" << a_ServerIP << "-y" << "C" << "-r";
	}
	else
	{
		// Clear any existing params before appending new
		m_params.clear();
		m_params << "-t" << QString::number( testtime ) <<  "-f" << "m" << "-c" << a_ServerIP << "-y" << "C";
	}


	MWTS_LEAVE;
}

/**
  Sets measurement to server mode
*/

void MwtsIPerfThroughput::SetServerMeasurement()
{
	MWTS_ENTER;

	// We are running a server
	m_pIperfRole = IPERF_SERVER;

	// Clear any existing params before appending new
	m_params.clear();
	m_params << "-s" <<  "-f" << "m" << "-y" << "C";

	MWTS_LEAVE;
}

/** Parses iperf output */
void MwtsIPerfThroughput::ParseOutput()
{
	MWTS_ENTER;

	// Grab stdout and stderr
	QByteArray stdOutBytes = m_pProcess->readAllStandardOutput();
	QByteArray stdErrBytes = m_pProcess->readAllStandardError();

	qDebug() << "stdout: " << stdOutBytes;
	qDebug() << "stderr: " << stdErrBytes;

	double tempSpeed = 0;
	double downSpeed = 0;

	int iBidirectionalStep = 1;
	QString stdOut( stdOutBytes );
	if ( stdOut.length() > 0 )
	{
		// If we were running an iterative cases, measurements for iterations are on their own lines
		int i = 0;
		QStringList lines = stdOut.split("\n");
		for (i = 0; i < lines.length(); i++ )
		{
			// bidirectional troughputs have two results, upload and download
			if(m_pIperfRole == IPERF_BICLIENT)
			{
				qDebug() << "Measuring bidirectional troughput....";
				// Output format will be one line of comma-separated values, in the order as follows
				// (Date and time),(Local IP),(Local port),(Remote IP),(Remote port),(ID),(Start time-End time),(Bytes transferred),(Speed, format depends on options)
				QStringList parts = lines[i].split( QChar( ',' ) );
				if ( parts.size() < 9 )
				{
					qDebug() << "Not enough data was outputted.";
					break;
				}
				QDateTime dateTime = QDateTime::fromString( parts.at( 0 ).trimmed(), QString( "yyyyMMddHHmmss" ) );
				qDebug() << "Date & time: " << dateTime.toString( Qt::TextDate );
				qDebug() << "Local IP & port: " << parts.at( 1 ).trimmed() << ":" << parts.at( 2 ).trimmed();
				qDebug() << "Remote IP & port: " << parts.at( 3 ).trimmed() << ":" << parts.at( 4 ).trimmed();
				qDebug() << "ID: " << parts.at( 5 ).trimmed();
				qDebug() << "Time slice: " << parts.at( 6 ).trimmed();
				qDebug() << "Bytes transferred: " << parts.at( 7 ).trimmed();
				qDebug() << "Tranfer speed (bits/second): " << parts.at( 8 ).trimmed();

				// Calculate the speed, (From bits/second to Mbits/second)
				bool ok = false;
				double speed = parts.at( 8 ).trimmed().toULong( &ok );

				if ( ok == true )
				{
					// step two means we have both upload and download troughputs
					if(iBidirectionalStep == 2)
					{
						iBidirectionalStep = 1;

						qDebug() << "speed up " << tempSpeed;
						// save this value for the step two
						downSpeed = speed / 1000.0 / 1000.0;

						qDebug() << "speed down " << downSpeed;

						g_pResult->AddMeasure( "Throughput down:", downSpeed, "Mbits per second" );
						//g_pResult->AddMeasure( "Throughput up:", downSpeed, "Mbits per second" );
						g_pResult->StepPassed( "Throughput", true );

						m_pReturnValue = true;

						break;
					}

					// save this value for the step two
				    tempSpeed = speed / 1000.0 / 1000.0;

					iBidirectionalStep++;
				}
				else
				{
					g_pResult->Write( "Failed to convert speed from string to double!" );
					g_pResult->StepPassed( "Throughput", false );
					m_pReturnValue = false;
				}
			}
			// normal case, not bidirectional
			else
			{
				// Output format will be one line of comma-separated values, in the order as follows
				// (Date and time),(Local IP),(Local port),(Remote IP),(Remote port),(ID),(Start time-End time),(Bytes transferred),(Speed, format depends on options)
				QStringList parts = lines[i].split( QChar( ',' ) );
				if ( parts.size() < 9 )
				{
					qDebug() << "Not enough data was outputted.";
					break;
				}
				QDateTime dateTime = QDateTime::fromString( parts.at( 0 ).trimmed(), QString( "yyyyMMddHHmmss" ) );
				qDebug() << "Date & time: " << dateTime.toString( Qt::TextDate );
				qDebug() << "Local IP & port: " << parts.at( 1 ).trimmed() << ":" << parts.at( 2 ).trimmed();
				qDebug() << "Remote IP & port: " << parts.at( 3 ).trimmed() << ":" << parts.at( 4 ).trimmed();
				qDebug() << "ID: " << parts.at( 5 ).trimmed();
				qDebug() << "Time slice: " << parts.at( 6 ).trimmed();
				qDebug() << "Bytes transferred: " << parts.at( 7 ).trimmed();
				qDebug() << "Tranfer speed (bits/second): " << parts.at( 8 ).trimmed();

				// Calculate the speed, (From bits/second to Mbits/second)
				bool ok = false;
				double speed = parts.at( 8 ).trimmed().toULong( &ok );
				if ( ok == true )
				{

					double speedMbits = speed / 1000.0 / 1000.0;
					g_pResult->AddMeasure( "Throughput", speedMbits, "Mbits per second" );
					g_pResult->StepPassed( "Throughput", true );
					m_pReturnValue = true;
				}
				else
				{
					g_pResult->Write( "Failed to convert speed from string to double!" );
					g_pResult->StepPassed( "Throughput", false );
					m_pReturnValue = false;
				}
			}

		}
	}
	else
	{
		g_pResult->Write( "Failed to measure throughput!" );
		g_pResult->StepPassed( "Throughput", false );
		m_pReturnValue = false;
	}

	MWTS_LEAVE;
}


/** Gets called if there is error in process*/
void MwtsIPerfThroughput::onError( QProcess::ProcessError error )
{
	MWTS_ENTER;
	m_pReturnValue = false;
	switch ( error )
	{
		case QProcess::FailedToStart:
			qDebug() << "The process failed to start.";
			break;
		case QProcess::Crashed:
			qDebug() << "The process crashed.";
			break;
		case QProcess::Timedout:
			qDebug() << "The last waitFor...() function timed out.";
			break;
		case QProcess::WriteError:
			qDebug() << "An error occurred when attempting to write to the process.";
			break;
		case QProcess::ReadError:
			qDebug() << "An error occurred when attempting to read from the process.";
			break;
		case QProcess::UnknownError:
			qDebug() << "An unknown error occurred.";
			break;
	}

	MWTS_LEAVE;
}

/** Called when the process finishes. Logs whether successfull or not*/
void MwtsIPerfThroughput::onFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
	MWTS_ENTER;
	qDebug() << "Exit code: " << exitCode;
	switch ( exitStatus )
	{
		case QProcess::NormalExit:
			qDebug() << "The process exited normally.";
			m_pReturnValue = true;
			break;
		case QProcess::CrashExit:
			qDebug() << "The process crashed.";
			m_pReturnValue = false;
			break;
	}
	g_pTest->Stop();
	ParseOutput();
	MWTS_LEAVE;
}

/** Gets called when iperf process started*/
void MwtsIPerfThroughput::onStarted()
{
	MWTS_ENTER;
	qDebug() << "The process has started.";
	if (m_pIperfRole == IPERF_SERVER)
	{
		g_pTest->Stop();
	}
	MWTS_LEAVE;
}

/** Gets called when process state changes*/
void MwtsIPerfThroughput::onStateChanged( QProcess::ProcessState newState )
{
	MWTS_ENTER;
	switch ( newState )
	{
		case QProcess::NotRunning:
			qDebug() << "The process is not running.";
			break;
		case QProcess::Starting:
			qDebug() << "The process is starting, but the program has not yet been invoked.";
			break;
		case QProcess::Running:
			qDebug() << "The process is running and is ready for reading and writing.";
			break;
	}
	MWTS_LEAVE;
}




