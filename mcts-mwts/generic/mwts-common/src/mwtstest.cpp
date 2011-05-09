/* mwtstest.cpp --
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
#include <MwtsCommon>
#include "mwtsload.h"
#include "mwtsthroughput.h"
#include "test_module_api.h"

#include <stdlib.h>
#include <QDir>

/** Constructs the test object and all needed dependencies*/
MwtsTest::MwtsTest(QObject* pParent)
: QObject(pParent)
{
//	MWTS_ENTER;  do not use MWTS_ENTER. logging not enabled at this point!!
	m_sCaseName = "";
	g_pTest = this;
	g_pLog = new MwtsLog();
	g_pResult = MwtsResult::instance();
	g_pCpuLoad = new MwtsCpuLoad();
	g_pMemLoad = new MwtsMemLoad();
	g_pIPerfThroughput = new MwtsIPerfThroughput();
	g_pTime = new QTime();
	m_pFailTimeoutTimer = NULL;
	m_pTestTimeoutTimer = NULL;

	g_pConfig=NULL;
	g_pApp=NULL;

	m_nTestTimeout=1000*3600*24;
	m_nFailTimeout=1000*3600*48;


	m_pIdleTimer=new QTimer(this);
	m_pIdleTimer->setSingleShot(false);
}

/** Destructor. Can detect if the test case is crashed before uninitialization*/
MwtsTest::~MwtsTest()
{
//	MWTS_ENTER;

	if(g_pApp) // this should have been deleted in Uninitialize
	{
		// case must have crashed - write to result
		g_pResult->Write("***CRASHED***");
		delete g_pApp;
	}
	if(g_pCpuLoad)
		delete g_pCpuLoad;
	if(g_pMemLoad)
		delete g_pMemLoad;
	if(g_pIPerfThroughput)
	{
		delete g_pIPerfThroughput;
		g_pIPerfThroughput = NULL;
	}
	if(g_pConfig)
		delete g_pConfig;
	if(g_pResult)
		delete g_pResult;
	if(g_pLog)
		delete g_pLog;
	g_pLog=NULL;

}

/**
  Returns name of the test module
*/
QString MwtsTest::Name()
{
	return this->metaObject()->className();
}

/**
  Returns name of currently running test case
  This works only with min 2009w40 and later
*/
QString MwtsTest::CaseName()
{
	// fetch name from MIN
	QString sName = QString(tm_get_caller_name());
	if(sName!="")
	{
		return sName;
	}
	if(m_sCaseName=="") // if no name is set
	{
		return "noname";
	}
	return m_sCaseName;
}

void MwtsTest::SetCaseName(QString sName)
{
	m_sCaseName=sName;
}


/**
  Initializes the test module, opens logs etc
  Calls OnInitialize which should be implemented in derived class
*/
void MwtsTest::Initialize()
{
	MWTS_ENTER;

	MwtsMonitor::instance()->Start();
	QString testpath="/var/log/tests/";
	QDir testdir(testpath);
	if(!testdir.exists())
	{
		if(!testdir.mkpath(testpath))
		{
			qCritical() << "Unable to create test log dir: " << testpath;
		}
	}

	qDebug() << "im here *" << testpath;
	
	QString logpath=testpath+CaseName()+".log";

	qDebug() << "im here **" << logpath;
	//QString logpath = "/var/log/tests/dupa";
	g_pLog->Open(logpath);

	qDebug() << "im here ***";
	g_pResult->Open(CaseName());

	qDebug() << "im here ***";

	qDebug()<<"Creating config";
	//QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/lib/tests/");
	g_pConfig=new MwtsConfig();

	qDebug()<<"Creating application";
	setenv("DISPLAY", ":0.0", 1);
	g_pApp = MwtsApp::instance();
	this->OnInitialize();

	connect(m_pIdleTimer, SIGNAL(timeout()), this, SLOT(OnIdle()));
	m_pIdleTimer->start(1000);

}


/**
  Uninitializes the test module, closes logs and results, writes report.
  Calls OnUninitialize which should be implemented in derived class
*/
void MwtsTest::Uninitialize()
{
	MWTS_ENTER;
	this->OnUninitialize();

	MwtsMonitor::instance()->Stop();

	g_pResult->WriteReport();

	// empty line in the end of result file helps catting all results
	g_pResult->Write("");

	qDebug() << "g_pResult: " << g_pResult;
	qDebug() << "g_pLog: " << g_pLog;

	g_pResult->Close();

	if(g_pApp)
	{
		delete g_pApp;
		g_pApp=NULL;
	}

}


bool MwtsTest::IsPassed()
{
	if(!g_pResult->IsPassed())
		return false;
	return true;
}

/**
  Default implementation of initialization that does nothing.
  This should be reimplemented in derived class if any kind of module initialization is wanted
*/

void MwtsTest::OnInitialize()
{
	// do nothing by default
}

/**
  Default implementation of uninitialization that does nothing.
  This should be reimplemented in derived class if any kind of module uninitialization is wanted
*/

void MwtsTest::OnUninitialize()
{
	// do nothing by default
}


/**
  Sets test timeout
  @param: timeout milliseconds
*/
void MwtsTest::SetTestTimeout(int milliseconds)
{
	MWTS_ENTER;
	qDebug() << "Setting test timeout to " << milliseconds;
	m_nTestTimeout=milliseconds;
}

/**
  Sets timeout after which test is marked timeouted
  @param: timeout milliseconds
*/
void MwtsTest::SetFailTimeout(int milliseconds)
{
	MWTS_ENTER;
	qDebug() << "Setting fail timeout to " << milliseconds;
	m_nFailTimeout=milliseconds;
}

/**
  Starts the test or test step. Sets timeout timers
  to stop the test.
  Executes QApplication. This function returns when Stop is called in slots or fail/test timeout occurs
*/
void MwtsTest::Start()
{
	MWTS_ENTER;

	qDebug()<<"Starting test";
	m_bFailTimeout=FALSE;

	Q_ASSERT_X(!m_pFailTimeoutTimer, "Start", "Stop() not called before calling Start() again");

	m_pFailTimeoutTimer=new QTimer;
	m_pFailTimeoutTimer->setSingleShot(true);
	connect(m_pFailTimeoutTimer, SIGNAL(timeout()), this, SLOT(OnFailTimeout()));

	m_pTestTimeoutTimer=new QTimer;
	m_pTestTimeoutTimer->setSingleShot(true);
	connect(m_pTestTimeoutTimer, SIGNAL(timeout()), this, SLOT(OnTestTimeout()));

	m_pFailTimeoutTimer->start(m_nFailTimeout);
	m_pTestTimeoutTimer->start(m_nTestTimeout);

	g_pApp->exec();
	qDebug()<<"Test executed";
}

/**
  Stops the execution of test. This is called automatically when timeout occurs.
  This should be called in slot-methods where you decide that testing should stop before timeout.
*/
void MwtsTest::Stop()
{
	MWTS_ENTER;
	qDebug()<<"Stopping test";

	if(m_pFailTimeoutTimer)
	{
		delete m_pFailTimeoutTimer;
		m_pFailTimeoutTimer = NULL;
	}


	if(m_pTestTimeoutTimer)
	{
		delete m_pTestTimeoutTimer;
		m_pTestTimeoutTimer = NULL;
		g_pApp->exit();
	}
	MWTS_LEAVE;

}

/**
  This function is called every now and then. You can override this method in your
  subclass to do some small tasks like updating status etc. Default timeout is once in a second.
*/
void MwtsTest::OnIdle()
{
	MWTS_ENTER;
	// do nothing by default
	// should we have some sort of "test ongoing" indicator?
}

/**
  This function is called when normal test timeout occurs.
  You can override this method in your subclass if you want to handle timeout some specific way
*/

void MwtsTest::OnTestTimeout()
{
	MWTS_ENTER;
	qDebug()<<"Test timeout";
	Stop();
}

/**
  This function is called when test fail timeout occurs.
  You can override this method in your subclass if you want to handle fail timeout some specific way
*/

void MwtsTest::OnFailTimeout()
{
	MWTS_ENTER;
	qCritical() << "Test fail timeout";

	m_bFailTimeout = TRUE;
	Stop();
}



