/* mwtslog.cpp --
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
#include "MwtsCommon"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

// filehandles for stdout and stderr
static int oldstdout;
static int oldstderr;
static int outpipe[2]={0,0};
static int errpipe[2]={0,0};

int getOrigOut() { return oldstderr; }
int getOrigErr() { return oldstderr; }
int getOutPipe() { return outpipe[0]; }
int getErrPipe() { return errpipe[0]; }


/**
  Message handler for QT debug, warning and error messages
*/

void MwtsMsgHanler( QtMsgType type, const char *msg )
{
	if(!g_pLog)
		return;

	switch ( type )
	{
	case QtDebugMsg:
		g_pLog->Debug(msg);
		break;
	case QtWarningMsg:
		g_pLog->Warning(msg);
		break;
	case QtCriticalMsg:
		g_pLog->Error(msg);
		break;
	case QtFatalMsg:
		g_pLog->Error(msg);
		break;
	}
}

/** Initializes the logging */
MwtsLog::MwtsLog()
{
	m_bLogPrintEnabled=false;
	m_bDebugEnabled=false;
	m_bTraceEnabled=false;

	QString sLogPrint = getenv("MWTS_LOG_PRINT");
	QString envDebug = getenv("MWTS_DEBUG");
	QString envTrace = getenv("MWTS_TRACE");

	if(sLogPrint=="1")
	{
		m_bLogPrintEnabled=true;
	}

	if(envTrace=="1")
	{
		m_bTraceEnabled=true;
	}

	if(envDebug=="1")
	{
		m_bDebugEnabled=true;
		m_bTraceEnabled=true;
	}

	m_pLogFile=NULL;
	qInstallMsgHandler(MwtsMsgHanler);

	RedirectStd();

}

MwtsLog::~MwtsLog()
{
	Close();
	g_pLog=NULL;
}

/** Redirects stdout and stderr to log files */
void MwtsLog::RedirectStd()
{
	int result, flags;

	// Enable stdout redirection to log only if
	// log print is not enabled. We don't want to redirect
	// logs to stdout and back to log
	if(! m_bLogPrintEnabled)
	{
		int flags = fcntl(1, F_GETFL, 0);
		fcntl(1, F_SETFL, flags | O_NONBLOCK);

		oldstdout = dup(1);
		if(oldstdout==-1)
		{
			qCritical()<<"Could not duplicate stdout";
			return;
		}
		result = pipe(outpipe);
		if(result!=0)
		{
			qCritical()<<"Could not create stdout pipe";
			return;
		}

		result = dup2(outpipe[1], 1);
		if(result==-1)
		{
			qCritical()<<"Could not duplicate stdout pipe";
			return;
		}

		flags = fcntl(getOutPipe(), F_GETFL, 0);
		fcntl(getOutPipe(), F_SETFL, flags | O_NONBLOCK);

	}

	flags = fcntl(2, F_GETFL, 0);
	fcntl(2, F_SETFL, flags | O_NONBLOCK);

	oldstderr = dup(2);
	if(oldstderr==-1)
	{
		qCritical()<<"Could not duplicate stderr";
		return;
	}

	result = pipe(errpipe);
	if(result!=0)
	{
		qCritical()<<"Could not create stderr pipe";
	}

	result = dup2(errpipe[1], 2);
	if(result==-1)
	{
		qCritical()<<"Could not duplicate stderr pipe";
		return;
	}

	flags = fcntl(getErrPipe(), F_GETFL, 0);
	fcntl(getErrPipe(), F_SETFL, flags | O_NONBLOCK);

}

/**
  Opens log file
  @param: sFilename filename of the log
*/
void MwtsLog::Open(QString sFilename)
{
	if(m_pLogFile)
	{
		MWTS_DEBUG("Opening new log: " +sFilename);
		delete m_pLogFile;
	}
	m_pLogFile = new QFile(sFilename);
	m_pLogFile->open(QIODevice::WriteOnly | /* QIODevice::Truncate*/  QIODevice::Append);
	time.start();
}


/**
  Closes the log file
*/
void MwtsLog::Close()
{
	if(m_pLogFile)
	{
		m_pLogFile->close();
		delete m_pLogFile;
		m_pLogFile=NULL;
	}
}

/**
  Enables or disables debugging
  @param: bEnable if true, enables debug. if false, disables debug
*/
void MwtsLog::EnableDebug(bool bEnable)
{

/*	if(!m_bDebugEnabled && bEnable) // if changed to enabled state
	{
		// todo connect signal logger to every signal in application
	}
	if(m_bDebugEnabled && !bEnable) // if changed to disabled state
	{
		// todo disconnect signals
	}
*/
	m_bDebugEnabled=bEnable;
}
/**
  Enables or disables debugging
  @param: bEnable if true, enables debug. if false, disables debug
*/
void MwtsLog::EnableTrace(bool bEnable)
{
	m_bTraceEnabled=bEnable;
}


void MwtsLog::EnableLogPrint(bool bEnable)
{
	m_bLogPrintEnabled=bEnable;
}

/**
  Writes message to log
  @param sText text to be written to log
*/
void MwtsLog::Write(QString sText)
{
	static char buffer[1024];
	static int n;
	if(!m_pLogFile)
		return;


	// first read stdout and stderr pipes if there is stuff
	if(getOutPipe()!=0)
	{
		// read stdout pipe
		n=read(getOutPipe(), buffer, 1023);
		if(n>0)
		{
			buffer[n]=0;
			m_pLogFile->write("stdout : ");
			m_pLogFile->write(buffer);
			m_pLogFile->write("\n");

		}
		// read stderr pipe
		n=read(getErrPipe(), buffer, 1023);
		if(n>0)
		{
			buffer[n]=0;
			g_pResult->AddError(buffer);
			m_pLogFile->write("stderr : ");
			m_pLogFile->write(buffer);
			m_pLogFile->write("\n");

		}

	}

	QString sTime;
	sTime.sprintf("%4.3lf", time.elapsed()/1000.0);
	QString str= sTime+":"+sText+"\n";

	m_pLogFile->write(str.toLatin1());

	m_pLogFile->flush();

	if(m_bLogPrintEnabled)
	{
		fprintf(stdout, str.toAscii().constData());
	}

}

/**
  Writes debug message to log, if debug is enabled
  @param sText debug text to be written to log
*/
void MwtsLog::Debug(QString sText)
{
	if(!m_bDebugEnabled)
		return;
	Write("Debug: "+ sText);
}

/**
  Writes trace message to log, if trace is enabled
  @param sText debug text to be written to log
*/
void MwtsLog::Trace(QString sText)
{
	// if MWTS_ENTER used when log is not yet initialized or is deleted.
	if(this==NULL)
	{
		return;
	}
	if(!m_bTraceEnabled)
		return;

	Write("Trace: "+ sText);

}

/**
  Writes warning message to log
  @param sText warning text to be written to log
*/
void MwtsLog::Warning(QString sText)
{
	Write("Warning: "+ sText);
}

/**
  Writes error message to log
  @param sText error text to be written to log
*/
void MwtsLog::Error(QString sText)
{

	g_pResult->AddError(sText);
	Write("Error: "+ sText);
}


/**
  Signal logger function. This is not used at the moment.
  Idea is to trace all QT signals with this handler,
  but it's still quite unclear how to implement this.
  Also unclear if the feature is wanted/needed.
*/
void MwtsLog::SignalLogger()
{
	// todo: do we need to log all signals emitted and received?
}




