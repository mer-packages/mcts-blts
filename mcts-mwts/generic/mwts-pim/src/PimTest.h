/* PimTest.h -- Header for PimTest class
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

#ifndef _INCLUDED_PIM_TEST_H
#define _INCLUDED_PIM_TEST_H

#include <MwtsCommon>
#include "contactstest.h"
#include "organizertest.h"

/**
 * Pim test class
 *
 */
class PimTest : public MwtsTest
{
	Q_OBJECT
public:

        /**
         * Constructor for pim test class
	 */
        PimTest();

        /**
         * Destructor for pim test class
	 */
        virtual ~PimTest();

        /**
	 * Overridden functions for MwtsTest class
	 * OnInitialize is called before test execution
	 */
	void OnInitialize();

        /**
	 * Overridden functions for MwtsTest class
	 * OnUninitialize is called after test execution
	 */
	void OnUninitialize();

public: // new functions

        /**
         * Returns instance of OrganizerTest
         */
        OrganizerTest& OrganizerTestImpl();

        /**
         * Returns instance of ContactsTest
         */
        ContactsTest& ContactsTestImpl();

private: // data

    // test implementations
    OrganizerTest *m_organizerTest;
    ContactsTest *m_contactsTest;
};

#endif //#ifndef _INCLUDED_PIM_TEST_H

