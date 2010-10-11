/* mwtsthroughput.h --
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



#ifndef _INCLUDED_MWTS_THROUGHPUT_H
#define _INCLUDED_MWTS_THROUGHPUT_H

#include <MwtsCommon>
#include <QObject>
#include <QProcess>

/** Base class for creating different kinds of load */
class MWTSCOMMON_EXPORT MwtsThroughput: public QObject
{
	Q_OBJECT

public:
	MwtsThroughput();
	virtual ~MwtsThroughput();

	bool Start();
	void Stop();

	/** Pure virtual external application execute functions
	This is implemented in subclasses to do actual execution*/
	virtual void SetClientMeasurement(char* a_ServerIP, int transtime, char* direction) = 0;
	virtual void SetServerMeasurement()= 0;

protected:
	/** Pure virtual external application execute functions
	This is implemented in subclasses to do actual execution*/
	virtual bool Exec() = 0;

	QProcess*	m_pProcess;
	QStringList	m_params;

};

/** Class for measuring throughput via iPerf */
class MWTSCOMMON_EXPORT MwtsIPerfThroughput : public MwtsThroughput
{
	Q_OBJECT

public:
	void SetClientMeasurement(char* a_ServerIP, int transtime, char* direction);
	void SetServerMeasurement();

protected:
	bool Exec();
	void ParseOutput();

	enum IPERF_ROLE
	{
		IPERF_CLIENT = 0,
		IPERF_BICLIENT = 1,
		IPERF_SERVER = 2
	};
	IPERF_ROLE	m_pIperfRole;

	bool		m_pReturnValue;

private Q_SLOTS:
	void onError( QProcess::ProcessError error );
	void onFinished( int exitCode, QProcess::ExitStatus exitStatus );
	void onStarted();
	void onStateChanged( QProcess::ProcessState newState );
};

// /** Template Class header for measuring throughput via some onother technology */
// class MWTSCOMMON_EXPORT MwtsTenplateThroughput: public MwtsThroughput
// {
// protected:
// 	bool Exec();
// };



#endif // #ifndef _INCLUDED_MWTS_THROUGHPUT_H

