/* mwtslog.h --
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

#ifndef _INCLUDED_MWTS_LOG_H
#define _INCLUDED_MWTS_LOG_H

#include <QObject>
#include <QMutex>
#include <QTime>
#include <MwtsCommon>

class QFile;

/** Logging functionality */
class MWTSCOMMON_EXPORT MwtsLog : public QObject
{
	Q_OBJECT

public:
	MwtsLog();
	virtual ~MwtsLog();

	void Open(QString sFilename);
	void Close();

	void EnableDebug(bool bEnable);
	void EnableTrace(bool bEnable);
	void EnableLogPrint(bool bEnable);

	void Write(QString sText);
	void Debug(QString sText);
	void Warning(QString sText);
	void Error(QString sText);
	void Trace(QString sText);

public slots:
	void SignalLogger();

protected:
	void RedirectStd();

	QFile* m_pLogFile;
	bool m_bDebugEnabled;
	bool m_bTraceEnabled;
	bool m_bLogPrintEnabled;
	QTime time;

    QMutex m_Mutex;
};


#endif // #ifndef _INCLUDED_MWTS_TEST_H

