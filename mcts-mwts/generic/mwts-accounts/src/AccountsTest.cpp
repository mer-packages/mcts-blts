/* AccountsTest.cpp -- source code for AccountsTest class
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
#include "AccountsTest.h"

/**
 * Constructor for template test class
 */
AccountsTest::AccountsTest() 
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
AccountsTest::~AccountsTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void AccountsTest::OnInitialize()
{
	MWTS_ENTER;

	// create objects for the test
	// connect wanted signals
	// do any kind of initialization here
	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void AccountsTest::OnUninitialize()
{
	MWTS_ENTER;
	// disconnect signals
	// clean objects etc
	MWTS_LEAVE;
}

/**
 * Test function
 */
void AccountsTest::TestSomething()
{
	MWTS_ENTER;

	qDebug("qDebug works also");

	// reading configuration is simple.
	QString sExample=g_pConfig->value("example/value1").toString();

	// as an example a timer is created. You should create something else.
	QTimer* pTimer=new QTimer(this);
	pTimer->start(2000);
	connect(pTimer, SIGNAL(timeout()), this, SLOT(Stop()));
	
	// now start the test.
	MWTS_DEBUG("Starting the main loop");
	Start();

	// when timer emits signal 'timeout' the Stop is called
	// then we continue here.
	MWTS_DEBUG("Main loop stopped");

	// write results. the first one is just informal if you want to.
	g_pResult->Write("Succesfully waited for time to elapse!");
	g_pResult->StepPassed("Waiting", true);
	g_pResult->AddMeasure("Waiting", 1.0, "Seconds per second");
}

