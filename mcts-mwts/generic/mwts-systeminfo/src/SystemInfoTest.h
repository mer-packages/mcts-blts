/* SystemInfoTest.cpp
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


#ifndef _INCLUDED_SYSTEMINFO_TEST_H
#define _INCLUDED_SYSTEMINFO_TEST_H

#include <QProcess>
#include <MwtsCommon>


/**
 * SystemInfo test class
 * Inherited from MwtsTest class
 */
/**
 *  @class SystemInfoTest
 *  @brief Implements main functionality of the test asset
 *
 *  This class is used as the main test asset code. Implements
 *  tests for dsme using dbus calls. Device temperature tesr
 *  are only tests that are implemented now, but also battery
 *  status can be found from dsme
 *
 * When this was written, dsme seems to be start of implementing
 * because implemented client side dbus calls were only temperature surface,
 * even if battery/charging status and omap temparatures should also be handled.
 *
 *  Look also QT mobility API
 *  for similar functionality.
 *
 */

#define SERVICE_NAME            "com.nokia.thermalmanager"
#define SERVICE_PATH            "/com/nokia/thermalmanager"


class SystemInfoTest : public MwtsTest
{
	Q_OBJECT
public:

    /**
     * @fn SystemInfoTest()
     * @brief Constructo for SystemInfoTest class
     */
    /**
	 * Constructor for template test class
	 */
    SystemInfoTest();

    /**
     * @fn ~SystemInfoTest()
     * @brief Destructor for SystemInfoTest class
     */
    /**
	 * Destructor for template test class
	 */
    virtual ~SystemInfoTest();

	/**
	 * Overridden functions for MwtsTest class
	 * OnInitialize is called before test execution
	 */
	void OnInitialize();

	/**
     * @fn void OnUninitialize()
     * @brief Overridden functions for MwtsTest class
	 * OnUninitialize is called after test execution
     */
    void OnUninitialize();

	/**
	 * Test function
	 */

    /**
     *  @fn void TestBattery()
     *  @brief Get status of the main battery and power state
      *  @param remote, true if call remote device by ssh
     */
    void TestBattery();


    private Q_SLOTS:
            void onError( QProcess::ProcessError error );
            void onFinished( int exitCode, QProcess::ExitStatus exitStatus );
            void onStarted();
            void onStateChanged( QProcess::ProcessState newState );

private:
        QProcess*	m_pProcess;
        QStringList	m_params;
        bool m_pReturnValue;


};

#endif //#ifndef _INCLUDED_SYSTEMINFO_TEST_H

