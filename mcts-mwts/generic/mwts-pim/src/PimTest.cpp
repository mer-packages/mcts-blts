/* PimTest.cpp -- source code for PimTest class
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
#include "PimTest.h"

/**
 * Constructor for Pim test class
 */
PimTest::PimTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Destructor for Pim test class
 */
PimTest::~PimTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void PimTest::OnInitialize()
{
	MWTS_ENTER;

        // Create organizer test implementation
        m_organizerTest = new OrganizerTest(*g_pConfig,*g_pResult);

        // Create contacts test implementation
        m_contactsTest = new ContactsTest(*g_pConfig,*g_pResult);

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void PimTest::OnUninitialize()
{
	MWTS_ENTER;

        delete m_organizerTest;
        delete m_contactsTest;

        MWTS_LEAVE;
}


/**
 * Returns instance of OrganizerTest
 */
OrganizerTest& PimTest::OrganizerTestImpl()
{
    return *m_organizerTest;
}

/**
 * Returns instance of ContactsTest
 */
ContactsTest& PimTest::ContactsTestImpl()
{
    return *m_contactsTest;
}

// eof
