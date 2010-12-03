/* functions.cpp
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
#include "SensorsTest.h"

#include <MwtsCommon>

const char *module_date = __DATE__;
const char *module_time = __TIME__;

SensorsTest Test;



/**
 *  @fn int AccelerometerTestFunc()
 *  @brief Put accelerometer callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int AccelerometerTestFunc (MinItemParser * item)
{
	MWTS_ENTER;
	
	int ret = 1;
	ret = Test.TestAccelerometer();
	return ret;
}

/**
 *  @fn int AmbientLightSensorTestFunc()
 *  @brief Put AmbientLightSensor callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int AmbientLightSensorTestFunc (MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestAmbientLightSensor();
	return ret;
}

/**
 *  @fn int CompassTestFunc()
 *  @brief Put Compass callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CompassTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestCompass();
	return ret;
}



/**
 *  @fn int MagnetometerTestFunc()
 *  @brief Put Magnetometer callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int MagnetometerTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestMagnetometer();
	return ret;
}



/**
 *  @fn int OrientationSensorTestFunc()
 *  @brief Put OrientationSensor callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int OrientationSensorTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestOrientationSensor();
	return ret;
}



/**
 *  @fn int ProximitySensorTestFunc()
 *  @brief Put Compass callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int ProximitySensorTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestProximitySensor();
	return ret;
}



/**
 *  @fn int RotationSensorTestFunc()
 *  @brief Put Compass callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int RotationSensorTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestRotationSensor();
	return ret;
}



/**
 *  @fn int TapSensorTestFunc()
 *  @brief Put Compass callbacks on. Put timeout on.
 *         If callback is got first, test is written succeeded to the result file.
 *         If timeout is got first, test is written failedto the result file.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int TapSensorTestFunc(MinItemParser * item)
{
	MWTS_ENTER;

	int ret = 1;
	ret = Test.TestTapSensor();
	return ret;
}



/**
 * @}
 */
int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

    ENTRYTC (*list, "AccelerometerTestFunc", AccelerometerTestFunc);
    ENTRYTC (*list, "AmbientLightSensorTestFunc", AmbientLightSensorTestFunc);
    ENTRYTC (*list, "CompassTestFunc", CompassTestFunc);
    ENTRYTC (*list, "MagnetometerTestFunc", MagnetometerTestFunc);
    ENTRYTC (*list, "OrientationSensorTestFunc", OrientationSensorTestFunc);
    ENTRYTC (*list, "ProximitySensorTestFunc", ProximitySensorTestFunc);
    ENTRYTC (*list, "RotationSensorTestFunc", RotationSensorTestFunc);
    ENTRYTC (*list, "TapSensorTestFunc", TapSensorTestFunc);

	return ENOERR;
}
