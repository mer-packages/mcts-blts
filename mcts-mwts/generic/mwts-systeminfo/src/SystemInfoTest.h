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

//#include <QProcess>
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

    /**
     *  @fn void TestChargingTypeUSB100mA()
     *  @brief Plug-in USB charger with 100mA, wait awhile, then unplug
     *         the charger. If wall charger with expected charger type is
     *         plugged and unplugged, test is written succeeded to the
     *         result file.
     *         If timeout or unexpedted charger type is plugged-in, test is
     *         written failed to the result file.
     */
    void TestChargingTypeUSB100mA();

    private:
   	    void SetCharging();


    private Q_SLOTS:
    	    /* Charging callbacks */
    	    void ChargingStateChanged(/*const Maemo::QmBattery::ChargingState state*/);

            /* This signal is emitted when battery level has changed. level is the new level.*/
            void batteryLevelChanged ( int level );

            /* This signal is emitted when battery status has changed. status is the new status. */
            void batteryStatusChanged ( QSystemDeviceInfo::BatteryStatus status );

            /* This signal is emitted when the power state has changed, such as when a phone gets plugged in to the wall. state is the new power state. */
            void powerStateChanged ( QSystemDeviceInfo::PowerState state );



    private:
        bool m_pReturnValue;
        bool m_QmExpectedWallPower;
        bool m_QmChargerPlugged;


};

#endif //#ifndef _INCLUDED_SYSTEMINFO_TEST_H

