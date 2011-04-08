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
#include <QTapReading>

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

	if (m_Sensor)
	{
		delete m_Sensor;
		m_Sensor = NULL;
	}
	disconnect();

	MWTS_LEAVE;
}



/*int SensorsTest::TestAccelerometer()
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
	if (!m_Sensor->isConnectedToBackend())
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
}*/

///////////////////////////////////////////////////////////////////

void SensorsTest::InitSensor(SensorsTest::SensorType type)
{
	MWTS_ENTER;

	/*
	currentSensorType = type;
	switch (type)
	{
		case AccelerometerType:
			m_Sensor = new QAccelerometer(this);
			break;
		case AmbientLightSensorType:
			m_Sensor = new QAmbientLightSensor(this);
			break;
		case CompassType:
			m_Sensor = new QCompass(this);
			break;
		case GyroscopeType:
			//m_Sensor = new QGyroscope(this);
			break;
		case LightSensorType:
			//m_Sensor = new QLightSensor(this);
			break;
		case MagnetometerType:
			m_Sensor = new QMagnetometer(this);
			break;
		case OrientationSensorType:
			m_Sensor = new QOrientationSensor(this);
			break;
		case ProximitySensorType:
			m_Sensor = new QProximitySensor(this);
			break;
		case RotationSensorType:
			m_Sensor = new QRotationSensor(this);
			break;
		case TapSensorType:
			m_Sensor = new QTapSensor(this);
			qDebug() << "Tap sensor initialized";
			break;
		default:
			break;
	}*/

	m_Sensor = new QTapSensor(this);
	//m_Sensor = new QSensor("TapSensor");

	connect(m_Sensor, SIGNAL(activeChanged()), this, SLOT(onActiveChanged()));
	connect(m_Sensor, SIGNAL(availableSensorsChanged()), this, SLOT(onAvailableSensorsChanged()));
	connect(m_Sensor, SIGNAL(busyChanged()), this, SLOT(onBusyChanged()));
	connect(m_Sensor, SIGNAL(readingChanged()), this, SLOT(onReadingChanged()));
	connect(m_Sensor, SIGNAL(sensorError(int)), this, SLOT(onSensorError(int)));

	QTimer* t = new QTimer(this);
	connect(t, SIGNAL(timeout()), this, SLOT(debug1()));
	t->start(1000);

	//m_Sensor->connectToBackend();

	m_Sensor->start();
	g_pTest->Start();

	MWTS_LEAVE;
}

void SensorsTest::StartSensor()
{
	MWTS_ENTER;

	qDebug() << "Enabling sensor";

	//m_Sensor->start();
	//g_pTest->Start();

	MWTS_LEAVE;
}


/* slots */

void SensorsTest::onActiveChanged()
{
	qDebug() << "active changed";
}

void SensorsTest::onAvailableSensorsChanged()
{
	qDebug() << "available sensors changed";
}

void SensorsTest::onBusyChanged()
{
	qDebug() << " busy changed";
}

void SensorsTest::onReadingChanged()
{
	switch (currentSensorType)
	{
	case AccelerometerType:

		break;
	case AmbientLightSensorType:

		break;
	case CompassType:

		break;
	case GyroscopeType:

		break;
	case LightSensorType:

		break;
	case MagnetometerType:

		break;
	case OrientationSensorType:

		break;
	case ProximitySensorType:

		break;
	case RotationSensorType:

		break;
	case TapSensorType:
		{
			QTapSensor* s1 = dynamic_cast<QTapSensor*>(m_Sensor);
			QTapReading* reading1 = s1->reading();
			qDebug() << reading1->tapDirection();
			//qDebug() << "can read now";
			break;
		}
	default:
		break;
	}

}

void SensorsTest::onSensorError(int error)
{
	qDebug() << "sensor error has occured" << error;
}

void SensorsTest::debug1()
{
	if (m_Sensor->isConnectedToBackend())
		qDebug() << "isConnectedToBackend";
	else
		qDebug() << "isNOTConnectedToBackend";

	if (m_Sensor->isActive())
		qDebug() << "isActive";
	else
		qDebug() << "isNOTActive";

	if (m_Sensor->isBusy())
		qDebug() << "is Busy";
	else
		qDebug() << "isNOTBusy";

	qDebug() << "error code " << m_Sensor->error();
}

///////////////////////////////////////////////////////


/* For Sensor callbacks */
// This slot  is called when an error code is set on the sensor.
// Note that some errors will cause the sensor to stop working.
// You should call isActive() to determine if the sensor is still running.

/*void SensorsTest::sensorError( int error )
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
}*/
