/* 
 #
 # Copyright (C) 2010 Intel Corporation.
 # This program is free software; you can redistribute it and/or modify it
 # under the terms and conditions of the GNU General Public License,
 # version 2, as published by the Free Software Foundation.
 #
 # This program is distributed in the hope it will be useful, but WITHOUT
 # ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 # FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 # for more details.
 #
 # You should have received a copy of the GNU General Public License along with
 # this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 # Place - Suite 330, Boston, MA 02111-1307 USA.

 # Authors: Fu, Yi  <yi.fu@intel.com>
 */

#ifndef _SENSORS_TEST_H
#define _SENSORS_TEST_H

#include <QSensor>
#include <QApplication>
#include <QTimer>

QTM_USE_NAMESPACE


/**
 *  @class SensorsTest
 *  @brief Implements main functionality of the test asset
 *
 *  This class is used as the main test asset code. Implements
 *  tests for sensors.
 */
class SensorsTest : public QObject
{
	Q_OBJECT
public:
	/**
	 *  @fn SensorsTest( )
	 *  @brief Constructor for SensorsTest.
	 */
	SensorsTest();

	/**
	 *  @fn virtual ~SensorsTest( )
	 *  @brief Virtual destructor for SensorsTest.
	 */
	virtual ~SensorsTest()
  {
	}


	/**
	 *  @fn void TestSensor()
	 *  @brief Generic sensor testing
	 *  Put accelerometer callbacks on. Put timeout on.
	 *  If callback is got first, test is written succeeded to the result file.
   *  If timeout is got first, test is written failed to the result file.
	 *  @return 0, if successfull. Error otherwise.
	 */
	int TestSensor();

/* For Sensor callbacks */
public slots:
 	/**
 	*  @fn void sensorError()
  *  @brief This slot is called when an error code is set on the sensor.
  */
  void sensorError( int error );

  /**
  *  @fn void readingChanged()
  *  @brief This slot is emitted when the reading has changed.
  */
  void readingChanged();
  virtual void OnTestTimeout();
  virtual void OnFailTimeout();
  void Stop();

public:
	QtMobility::QSensor* m_Sensor;
	QCoreApplication * m_app;

private:
	bool m_is_readingChanged;	
  QTimer * m_pFailTimeoutTimer;
  QTimer * m_pTestTimeoutTimer;
	int m_nFailTimeout;
	int m_nTestTimeout;
};

#endif //#ifndef _SENSORS_TEST_H


