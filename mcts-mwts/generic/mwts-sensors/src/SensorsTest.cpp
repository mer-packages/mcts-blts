/* SensorsTest.cpp
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

#include <QAccelerometer>
#include <QAmbientLightSensor>
#include <QCompass>
#include <QMagnetometer>
#include <QOrientationSensor>
#include <QProximitySensor>
#include <QRotationSensor>
#include <QTapSensor>
#include <QSensorReading>
#include "stable.h"
#include "SensorsTest.h"

QTM_USE_NAMESPACE

SensorsTest::SensorsTest()
{
	MWTS_ENTER;
}

SensorsTest::~SensorsTest()
{
	MWTS_ENTER;
}


void SensorsTest::OnInitialize()
{
	MWTS_ENTER;

	MWTS_LEAVE;
}

void SensorsTest::OnUninitialize()
{
	MWTS_ENTER;

	MWTS_LEAVE;
}



int SensorsTest::TestAccelerometer()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QAccelerometer(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestAmbientLightSensor()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QAmbientLightSensor(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}


int SensorsTest::TestCompass()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QCompass(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestMagnetometer()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QMagnetometer(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestOrientationSensor()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QOrientationSensor(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestProximitySensor()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QProximitySensor(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestRotationSensor()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QRotationSensor(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestTapSensor()
{
	MWTS_ENTER;
	int ret;

	m_Sensor = new QtMobility::QTapSensor(this);
	ret = TestSensor();

	delete m_Sensor;
	m_Sensor = NULL;

	MWTS_LEAVE;
	return ret;
}

int SensorsTest::TestSensor()
{
	MWTS_ENTER;
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
	
	// now start qt main loop
	MWTS_DEBUG("Starting the qt-main loop");
	Start();

	// when signal is got
    // then we continue here.
    MWTS_DEBUG("Main loop stopped");

    // check if connected to backend
    if (!m_Sensor->connectedToBackend())
    {
        g_pResult->StepPassed("readingChanged", false);
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

	g_pResult->StepPassed("readingChanged", m_is_readingChanged);
	MWTS_LEAVE;
	return 0;
}




/* For Sensor callbacks */
// This slot  is called when an error code is set on the sensor.
// Note that some errors will cause the sensor to stop working.
// You should call isActive() to determine if the sensor is still running.

void SensorsTest::sensorError( int error )
{
	MWTS_ENTER;
	MWTS_DEBUG("sensorError " + QString::number(error));

	if (!m_Sensor->isActive () )
	{
		qCritical() << "Sensor not active, serious error";
		Stop();
	}
	else
	{
		MWTS_DEBUG("Sensor still active, still waiting values from sensor");
	}

	MWTS_LEAVE;
}

// This slot is emitted when the reading has changed.

void SensorsTest::readingChanged()
{
	MWTS_ENTER;
	MWTS_DEBUG("Sensor Changed");

	QtMobility::QSensorReading* accelerometerReading = m_Sensor->reading ();
	MWTS_DEBUG("Accelerometer has " + QString::number(accelerometerReading->valueCount ()) + " values");

    int i;
	for ( i=0; i < accelerometerReading->valueCount (); i++ )
	{
	    g_pResult->AddMeasure( "Value " + QString::number(i), accelerometerReading->value(i).toDouble(), "Real" );
	}
	m_is_readingChanged = true;
	Stop();

	MWTS_LEAVE;
}
