/* functions.cpp -- Test assets MIN functions
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
#include "interface.h"
#include "SystemInfoTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
SystemInfoTest Test;


/**
* @fn void BatteryTest()
* @brief CGet status of the main battery and power state
* Implemenation QT dbus classes
* @param item	MIN scripter parameters
* @return		ENOERR
*/

LOCAL int BatteryTest (MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestBattery();
    return ENOERR;
}

/**
 *  @fn int ChargingTypeWallTestFunc()
 *  @brief Put charging callbacks on.
 *         If callback for charging is got with Wall charging type, then
 *         the callback for not charging, the test is written succeeded
 *         to the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int ChargingTypeWallTestFunc (MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestWallPower();
    return ENOERR;
}

/**
 *  @fn int ChargingTypeUSB500mATestFunc()
 *  @brief Put charging callbacks on.
 *         If callback for charging is got with USB with 500mA charging type,
 *         then the callback for not charging, the test is written succeeded
 *         to the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int ChargingTypeUSB500mATestFunc (MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestBatteryPower();
    return ENOERR;
}

/**
 *  @fn int ChargingTypeUSB100mATestFunc()
 *  @brief Put charging callbacks on.
 *         If callback for charging is got with USB with 100mA charging type,
 *         then the callback for not charging, the test is written succeeded
 *         to the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int ChargingTypeUSB100mATestFunc (MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestChargingTypeUSB100mA();
    return ENOERR;
}


int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);
    ENTRYTC (*list, "BatteryTest", BatteryTest);
    ENTRYTC (*list, "ChargingTypeWallTestFunc", ChargingTypeWallTestFunc);
    ENTRYTC (*list, "ChargingTypeUSB500mATestFunc", ChargingTypeUSB500mATestFunc);
    ENTRYTC (*list, "ChargingTypeUSB100mATestFunc", ChargingTypeUSB100mATestFunc);
    return ENOERR;
}

