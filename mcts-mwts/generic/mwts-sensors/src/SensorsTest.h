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
//#include <QApplication>

#include <MwtsCommon>


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

	///////////////////////////////////////////////////////////////////////////////////

	/**
	 *  @fn void TestAccelerometer()
	 *  @brief Put accelerometer callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestAccelerometer();

	/**
	 *  @fn void TestAmbientLightSensor()
	 *  @brief Put AmbientLightSensor callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestAmbientLightSensor();

	/**
	 *  @fn void TestCompass()
	 *  @brief Put Compass callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestCompass();

	/**
	 *  @fn void TestMagnetometer()
	 *  @brief Put Magnetometer callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestMagnetometer();

	/**
	 *  @fn void TestOrientationSensor()
	 *  @brief Put OrientationSensor callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestOrientationSensor();

	/**
	 *  @fn void TestProximitySensor()
	 *  @brief Put ProximitySensor callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestProximitySensor();

	/**
	 *  @fn void RotationSensor()
	 *  @brief Put RotationSensor callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestRotationSensor();

	/**
	 *  @fn void TapSensor()
	 *  @brief Put TapSensor callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestTapSensor();

	///////////////////////////////////////////////////////////////////////////////////

	void printSensorsTypes();



private:
	/**
	 *  @fn void TestSensor()
	 *  @brief Generic sensor testing
	 *         Put accelerometer callbacks on. Put timeout on.
	 *         If callback is got first, test is written succeeded to the result file.
     *         If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	//int TestSensor();


private slots:

/* For Sensor callbacks */

	void onActiveChanged();
	void onAvailableSensorsChanged();
	void onBusyChanged();
	void onReadingChanged();
	void onSensorError(int error);

	void debug1();

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
	QtMobility::QSensor* m_Sensor;
	SensorType currentSensorType;
	bool m_is_readingChanged;
};





#endif //#ifndef _SENSORS_TEST_H


