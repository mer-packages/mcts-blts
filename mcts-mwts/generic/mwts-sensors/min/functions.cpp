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
	Initializes sensor of type given in parameter
 */
LOCAL int InitSensor(MinItemParser * item)
{
	MWTS_ENTER;

	char *type = NULL;

	if (mip_get_next_string(item, &type))
	{
		qCritical() << "The type must contain chars.";
		return 1;
	}
	else
	{
		QString s(type);
		if (s == "accelerometer")
		{
			Test.InitSensor(SensorsTest::AccelerometerType);
		}
		else if (s == "ambient_light_sensor")
		{
			Test.InitSensor(SensorsTest::AmbientLightSensorType);
		}
		else if (s == "compass")
		{
			Test.InitSensor(SensorsTest::CompassType);
		}
		else if (s == "gyroscope")
		{
			Test.InitSensor(SensorsTest::GyroscopeType);
		}
		else if (s == "light_sensor")
		{
			Test.InitSensor(SensorsTest::LightSensorType);
		}
		else if (s == "magnetometer")
		{
			Test.InitSensor(SensorsTest::MagnetometerType);
		}
		else if (s == "orientation_sensor")
		{
			Test.InitSensor(SensorsTest::OrientationSensorType);
		}
		else if (s == "proximity_sensor")
		{
			Test.InitSensor(SensorsTest::ProximitySensorType);
		}
		else if (s == "rotation_sensor")
		{
			Test.InitSensor(SensorsTest::RotationSensorType);
		}
		else if (s == "tap_sensor")
		{
			Test.InitSensor(SensorsTest::TapSensorType);
		}
		else
		{
		   qCritical() << "No such type";
		   return 1;
		}
	}
	free(type);

	return ENOERR;
}

/**
  Starts test and activate sensor
 */
LOCAL int StartTest(__attribute__((unused)) MinItemParser * item)
{
	 MWTS_ENTER;
	 Test.StartSensor();
	 return ENOERR;
}



/**
 * @}
 */
int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

	ENTRYTC (*list, "InitSensor", InitSensor);
	ENTRYTC (*list, "StartTest", StartTest);

	return ENOERR;
}

