/* SensorsTest.h
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

#ifndef _SENSORS_TEST_H
#define _SENSORS_TEST_H

#include <QSensor>
//#include <QTapSensor>
//#include <QApplication>

#include <MwtsCommon>

QTM_USE_NAMESPACE;

/**
 *  @class SensorsTest
 *  @brief Implements main functionality of the test asset
 *
 *  This class is used as the main test asset code. Implements
 *  tests for sensors.
 */


class SensorsTest : public MwtsTest
{
	Q_OBJECT
public:

	/**
	 *	Indicates what type of sensor is used at the moment.
	 */
	enum SensorType
	{
		AccelerometerType,
		AmbientLightSensorType,
		CompassType,
		GyroscopeType,
		LightSensorType,
		MagnetometerType,
		OrientationSensorType,
		ProximitySensorType,
		RotationSensorType,
		TapSensorType
	};

	/**
	 *  @fn SensorsTest( )
	 *  @brief Constructor for SensorsTest.
	 */
	SensorsTest();

	/**
	 *  @fn virtual ~SensorsTest( )
	 *  @brief Virtual destructor for SensorsTest.
	 */
	virtual ~SensorsTest();

	/**
	 *  @fn virtual void OnInitialize()
	 *  @brief Virtual initialization function. Derived from MwtsTest.
	 */
	void OnInitialize();

	/**
	 *  @fn virtual void OnUninitialize()
	 *  @brief Virtual uninitialization function. Derived from MwtsTest.
	 */
	void OnUninitialize();

	void InitSensor(SensorsTest::SensorType type);

	void StartSensor();

	void OnFailTimeout();

	void OnIdle();

private:

	void checkBackendAvailability();


private slots:

/* For Sensor callbacks */

	void onActiveChanged();
	void onAvailableSensorsChanged();
	void onBusyChanged();
	void onReadingChanged();
	void onSensorError(int error);

	void debugMessage();

 /**
  *  @fn void sensorError()
  *  @brief This slot is called when an error code is set on the sensor.
  */
//	void sensorError( int error );

    /**
      *  @fn void readingChanged()
      *  @brief This slot is emitted when the reading has changed.
      */
//	void readingChanged();


private:
	QSensor* m_Sensor;
	//QTapSensor* m_Sensor;
	SensorType currentSensorType;
	bool m_is_readingChanged;
	//QSettings* defaultSensors;
};

#endif //#ifndef _SENSORS_TEST_H


