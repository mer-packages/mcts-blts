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
 *  @fn int WallPowerTestFunc()
 *  @brief Put charging callbacks on.
 *         If callback for charging is got with Wall charging type, then
 *         the callback for not charging, the test is written succeeded
 *         to the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int WallPowerTestFunc(MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestWallPower();
    return ENOERR;
}

/**
 *  @fn int BatteryPowerTestFunc()
 *  @brief Put charging callbacks on.
 *         If callback for charging is got with USB with 500mA charging type,
 *         then the callback for not charging, the test is written succeeded
 *         to the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int BatteryPowerTestFunc (MinItemParser * item)
{
    MWTS_ENTER;
    Test.TestBatteryPower();
    return ENOERR;
}



int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);
    ENTRYTC (*list, "WallPowerTest", WallPowerTestFunc);
    ENTRYTC (*list, "BatteryPowerTest", BatteryPowerTestFunc);
    return ENOERR;
}

