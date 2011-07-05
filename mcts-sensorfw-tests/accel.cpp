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

 # Authors:  Fu, Yi <yi.fu@intel.com>
 */

#include <QAccelerometer>
#include <QAmbientLightSensor>
#include <QCompass>
#include <QMagnetometer>
#include <QOrientationSensor>
#include <QProximitySensor>
#include <QRotationSensor>
#include <QTapSensor>
#include <QSensorReading>
#include <QDebug>
#include "accel.h"

QTM_USE_NAMESPACE

 /**
 **  @fn SensorsTest( )
 *   @brief Constructor for SensorsTest.
 **/
SensorsTest::SensorsTest():m_nFailTimeout(10000),
			m_nTestTimeout(10000),
			m_pFailTimeoutTimer(NULL),
			m_pTestTimeoutTimer(NULL)
{
}
/* TestSensor */
// 
int SensorsTest::TestSensor()
{
	int iRet = 0;
	m_is_readingChanged = false;

	connect(m_Sensor,
			SIGNAL(readingChanged ()),
			this,
			SLOT(readingChanged()));
	connect(m_Sensor,
			SIGNAL(sensorError (int)),
			this,
			SLOT(sensorError (int)));

	m_Sensor->start();
	qDebug("SensorsTest::OnInitialize End, all initialised to get signals and listerners started");
	
  if (!m_Sensor->isActive()) 
  {  
      qWarning("Accelerometer didn't start!");        
  }
	
	m_pFailTimeoutTimer = new QTimer;
  m_pFailTimeoutTimer->setSingleShot(true);
  connect(m_pFailTimeoutTimer,SIGNAL(timeout()),this,SLOT(OnFailTimeout())); 
	m_pTestTimeoutTimer = new QTimer;
  m_pTestTimeoutTimer->setSingleShot(true);
  connect(m_pTestTimeoutTimer,SIGNAL(timeout()),this,SLOT(OnTestTimeout()));

	m_pFailTimeoutTimer->start(m_nFailTimeout);
  m_pTestTimeoutTimer->start(m_nTestTimeout);

	// now start qt main loop
	m_app->exec();

	// when signal is got
  // then we continue here.
  // check if connected to backend
  if (!m_Sensor->isConnectedToBackend())
  {
        qDebug("readingChanged =  false");
        qCritical() << "Sensor is not connected to a backend. A sensor that has not been connected to a backend cannot do anything useful.";
  }	

	m_Sensor->stop();
	disconnect(m_Sensor,
			SIGNAL(sensorError (int)),
			this,
			SLOT(sensorError (int)));
	disconnect(m_Sensor,
			SIGNAL(readingChanged ()),
			this,
			SLOT(readingChanged()));

	if (m_is_readingChanged)
	{
	   qDebug() << "Sensor Test: Passed";
	   iRet = 1;
	}
	else
	{
	   qDebug() << "Sensor Test: Failed";
	}
	m_app->exit();
	return iRet;
}

/* For Sensor callbacks */
// This slot  is called when an error code is set on the sensor.
// Note that some errors will cause the sensor to stop working.
// You should call isActive() to determine if the sensor is still running.
void SensorsTest::sensorError( int error )
{
	qDebug() << "sensorError " <<  QString::number(error);
	if (!m_Sensor->isActive () )
	{
		qCritical() << "Sensor not active, serious error";
		m_app->exit();
	}
	else
	{
		qDebug() << "Sensor still active, still waiting values from sensor";
	}

}

// This slot is emitted when the reading has changed.

void SensorsTest::readingChanged()
{
	qDebug() << "Sensor Changed";
	QtMobility::QSensorReading* accelerometerReading = m_Sensor->reading ();
	qDebug() << "Sensor has " << QString::number(accelerometerReading->valueCount ()) <<  " values";

  int i;
	for ( i=0; i < accelerometerReading->valueCount (); i++ )
	{
	    qDebug() <<  "Value " <<  QString::number(i) << accelerometerReading->value(i).toDouble() <<  "Real";
	}
	m_is_readingChanged = true;
	m_app->exit();	
}

/* OnTestTimeout */
// 
void SensorsTest::OnTestTimeout()
{
    qDebug() << "Test timeout";
    Stop();
}

/* Stop */
// Stop sensor test
void SensorsTest::Stop()
{
  qDebug() << "Stop test";
  if (m_pFailTimeoutTimer)
  {
	  delete m_pFailTimeoutTimer;
	  m_pFailTimeoutTimer = NULL;
  } 

  if (m_pTestTimeoutTimer)
  {
	  delete m_pTestTimeoutTimer;
    m_pTestTimeoutTimer = NULL;
    m_app->exit();
  }
}

/* OnFailTimeout */
// 
void SensorsTest::OnFailTimeout()
{
   qCritical() << "Test fail timeout";
   Stop();
}
