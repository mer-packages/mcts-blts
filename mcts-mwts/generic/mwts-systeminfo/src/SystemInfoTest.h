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

#include <QSystemDeviceInfo>

#include <MwtsCommon>

QTM_USE_NAMESPACE



/**
 * SystemInfo test class
 * Inherited from MwtsTest class
 */
/**
 *  @class SystemInfoTest
 *  @brief Implements main functionality of the test asset
 *
 *  This class is used as the main test asset code. Implements
 *  tests for dsme using QSystemDeviceInfo calls.
 *
 */


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
     * Test functions
	 */


    /**
     *  @fn void TestWallPower()
     *  @brief Plug-in wall charger, wait awhile, then unplug the charger.
     *         If wall charger with expected charger type is plugged and
     *         unplugged, test is written succeeded to the result file.
     *         If timeout or unexpedted charger type is plugged-in, test is
     *         written failed to the result file.
     */
    void TestWallPower();

    /**
     *  @fn void TestBatteryPower()
     *  @brief Plug-in USB charger with 500mA, wait awhile, then unplug
     *         the charger. If wall charger with expected charger type is
     *         plugged and unplugged, test is written succeeded to the
     *         result file.
     *         If timeout or unexpedted charger type is plugged-in, test is
     *         written failed to the result file.
     */
    void TestBatteryPower();

    private:
   	    void SetCharging();


    private Q_SLOTS:

            /* This signal is emitted when battery level has changed. level is the new level.*/
            void batteryLevelChanged ( int level );

            /* This signal is emitted when battery status has changed. status is the new status. */
            void batteryStatusChanged ( QSystemDeviceInfo::BatteryStatus status );

            /* This signal is emitted when the power state has changed, such as when a phone gets plugged in to the wall. state is the new power state. */
            void powerStateChanged ( QSystemDeviceInfo::PowerState state );



    private:
        QSystemDeviceInfo* m_pSysteminfo;

        bool m_pReturnValue;
        bool m_QmExpectedWallPower;
        bool m_QmExpectedBatteryPower;
        bool m_QmChargerPlugged;
        bool m_FirstStep;
 

};

#endif //#ifndef _INCLUDED_SYSTEMINFO_TEST_H

