/* mwtstest.h --
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



#ifndef _INCLUDED_MWTS_TEST_H
#define _INCLUDED_MWTS_TEST_H

#include "mwtsglobal.h"
#include "mwtsapp.h"
#include "mwtsresult.h"
#include "mwtslog.h"

#include <QObject>
#include <QTimer>
#include <QString>

/** Base class for all test asset classes. Derive this class to create your own test class*/
class MWTSCOMMON_EXPORT MwtsTest : public QObject
{
	Q_OBJECT

public:
	MwtsTest(QObject* pParent=0, QString platform="");
	virtual ~MwtsTest();

	/** Returns name of the test class*/
	QString Name();
	/** Returns name of currently running test case.
	This can be overridden in your test class if you need to specify it yourself,
	for example in cases that you don't use MIN test framework */
	virtual QString CaseName();
	void SetCaseName(QString sName);

	/** Returns name of currently running platform.
        */
	virtual QString Platform();
	void SetPlatform(QString sName);

	void SetFailTimeout(int milliseconds);
	void SetTestTimeout(int milliseconds);

	void Initialize();
	void Uninitialize();
	bool IsPassed();

	// Pure virtuals to be implemented by derived asset
	virtual void OnInitialize();
	virtual void OnUninitialize();


public slots:
	void Start();
	void Stop();

protected slots:

	virtual void OnTestTimeout();
	virtual void OnFailTimeout();
	virtual void OnIdle();

protected:
	int m_nTestTimeout;
	int m_nFailTimeout;
	bool m_bFailTimeout;
	QTimer* m_pIdleTimer;
	QTimer* m_pFailTimeoutTimer;
	QTimer* m_pTestTimeoutTimer;
	QString m_sCaseName;
	QString m_sPlatform;


};


#endif // #ifndef _INCLUDED_MWTS_TEST_H

