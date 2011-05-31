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
#include <QGyroscope>
#include <QLightSensor>
#include <QMagnetometer>
#include <QOrientationSensor>
#include <QProximitySensor>
#include <QRotationSensor>
#include <QTapSensor>

#include <QSensorReading>
#include <QAccelerometerReading>
#include <QAmbientLightReading>
#include <QCompassReading>
#include <QGyroscopeReading>
#include <QLightReading>
#include <QMagnetometerReading>
#include <QOrientationReading>
#include <QProximityReading>
#include <QRotationReading>
#include <QTapReading>

#include "stable.h"
#include "SensorsTest.h"

//QTM_USE_NAMESPACE

SensorsTest::SensorsTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

SensorsTest::~SensorsTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}


void SensorsTest::OnInitialize()
{
	MWTS_ENTER;

	/*
	defaultSensors = new QSettings("/etc/xdg/Nokia/Sensors.conf", QSettings::NativeFormat);
	qDebug() << "Dafault sensors for types are:";
	QStringList df = defaultSensors->allKeys();
	foreach (QString s, df)
		qDebug() << s << "=" << defaultSensors->value(s).toString();
	*/

	qDebug() << "Setting back default path for config files.";
	QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, "/etc/xdg");

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


	MWTS_LEAVE;
}

void SensorsTest::InitSensor(SensorsTest::SensorType type)
{
	MWTS_ENTER;

	currentSensorType = type;

	switch (type)
	{
		case AccelerometerType:
			m_Sensor = new QAccelerometer(this);
			qDebug() << "Accelerometer sensor initialized";
			break;
		case AmbientLightSensorType:
			m_Sensor = new QAmbientLightSensor(this);
			qDebug() << "Ambient light sensor initialized";
			break;
		case CompassType:
			m_Sensor = new QCompass(this);
			qDebug() << "Compass sensor initialized";
			break;
		case GyroscopeType:
			//m_Sensor = new QGyroscope(this);
			break;
		case LightSensorType:
			m_Sensor = new QLightSensor(this);
			qDebug() << "Light sensor initialized";
			break;
		case MagnetometerType:
			m_Sensor = new QMagnetometer(this);
			qDebug() << "Magnetometer sensor initialized";
			break;
		case OrientationSensorType:
			m_Sensor = new QOrientationSensor(this);
			qDebug() << "Orientation sensor initialized";
			break;
		case ProximitySensorType:
			m_Sensor = new QProximitySensor(this);
			qDebug() << "Proximity sensor initialized";
			break;
		case RotationSensorType:
			m_Sensor = new QRotationSensor(this);
			qDebug() << "Rotation sensor initialized";
			break;
		case TapSensorType:
			m_Sensor = new QTapSensor(this);
			qDebug() << "Tap sensor initialized";
			break;
		default:
			break;
	}

	connect(m_Sensor, SIGNAL(activeChanged()), this, SLOT(onActiveChanged()));
	connect(m_Sensor, SIGNAL(availableSensorsChanged()), this, SLOT(onAvailableSensorsChanged()));
	connect(m_Sensor, SIGNAL(busyChanged()), this, SLOT(onBusyChanged()));
	connect(m_Sensor, SIGNAL(readingChanged()), this, SLOT(onReadingChanged()));
	connect(m_Sensor, SIGNAL(sensorError(int)), this, SLOT(onSensorError(int)));

	//checkBackendAvailability();

	//QTimer* t = new QTimer(this);
	//connect(t, SIGNAL(timeout()), this, SLOT(debug1()));
	//t->start(1000);

	MWTS_LEAVE;
}

void SensorsTest::StartSensor()
{
	MWTS_ENTER;

	/*qDebug() << "Setting default sensor backend";
	qDebug() << defaultSensors->value("Default/" + m_Sensor->type()).toString();
	m_Sensor->setIdentifier(defaultSensors->value(m_Sensor->type()).toByteArray());
	qDebug() << m_Sensor->identifier();*/


	qDebug() << "Enabling sensor ... ";
	if (m_Sensor->start())
		qDebug() << "... true, sensor started";
	else
		qDebug() << "... false, sensors not started";

	debugMessage();

	g_pTest->Start();

	MWTS_LEAVE;
}

void SensorsTest::OnFailTimeout()
{
	m_Sensor->stop();
	qCritical() << "Did not received any sensor data - test fail!";
	g_pTest->Stop();
}

void SensorsTest::OnIdle()
{
	qDebug() << "Waiting for user interaction...";
}


/* slots */

void SensorsTest::onActiveChanged()
{
	if (m_Sensor->isActive())
		qDebug() << "Sensor is active";
	else
		qDebug() << "Sensor is not active";
}

void SensorsTest::onAvailableSensorsChanged()
{
	qDebug() << "Available sensors changed";
}

void SensorsTest::onBusyChanged()
{
	if (m_Sensor->isBusy())
		qDebug() << "Sensor is busy";
	else
		qDebug() << "Sensor is not busy";
}

void SensorsTest::onReadingChanged()
{

	QString msg;

	msg.append("\n+---------------------\n");

	switch (currentSensorType)
	{
		case AccelerometerType:
		{
			QAccelerometer* s = dynamic_cast<QAccelerometer*>(m_Sensor);
			QAccelerometerReading* reading = s->reading();
			msg.append("| Received accelerometer sensor data\n");
			msg.append("| x " + QString::number(reading->x()) + ", y " + QString::number(reading->y()) + ", z " + QString::number(reading->z()));
			break;
		}
		case CompassType:
		{
			QCompass* s = dynamic_cast<QCompass*>(m_Sensor);
			QCompassReading* reading = s->reading();
			msg.append("| Received compass sensor data\n");
			msg.append("| azimuth " + QString::number(reading->azimuth()) + ", calibration level " + QString::number(reading->calibrationLevel()));
			break;
		}
		case AmbientLightSensorType:
		{
			QAmbientLightSensor* s = dynamic_cast<QAmbientLightSensor*>(m_Sensor);
			QAmbientLightReading* reading = s->reading();
			msg.append("| Received ambient light sensor data\n");
			msg.append("| light level " + QString::number(reading->lightLevel()));
			break;
		}
		case GyroscopeType:

			break;
		case LightSensorType:
		{
			QLightSensor* s = dynamic_cast<QLightSensor*>(m_Sensor);
			QLightReading* reading = s->reading();
			msg.append("| Received light sensor data\n");
			msg.append("| lux " + QString::number(reading->lux()));
			break;
		}
		case MagnetometerType:
		{
			QMagnetometer* s = dynamic_cast<QMagnetometer*>(m_Sensor);
			QMagnetometerReading* reading = s->reading();
			msg.append("| Received magnetometer sensor data\n");
			msg.append("| calibration level " + QString::number(reading->calibrationLevel()) + ", x " + QString::number(reading->x()) + ", y " + QString::number(reading->y()) + ", z" + QString::number(reading->z()));
			break;
		}
		case OrientationSensorType:
		{
			QOrientationSensor* s = dynamic_cast<QOrientationSensor*>(m_Sensor);
			QOrientationReading* reading = s->reading();
			msg.append("| Received orientation sensor data\n");
			msg.append("| orientation " + QString::number(reading->orientation()));
			break;
		}
		case ProximitySensorType:
		{
			QProximitySensor* s = dynamic_cast<QProximitySensor*>(m_Sensor);
			QProximityReading* reading = s->reading();
			msg.append("| Received proximity sensor data");
			if (reading->close())
				msg.append(", close\n");
			else
				msg.append(", not close\n");
			break;
		}
		case RotationSensorType:
		{
			QRotationSensor* s = dynamic_cast<QRotationSensor*>(m_Sensor);
			QRotationReading* reading = s->reading();
			msg.append("| Received rotation sensor data\n");
			msg.append("| x " + QString::number(reading->x()) + ", y " + QString::number(reading->y()) + ", z " + QString::number(reading->z()));
			break;
		}
		case TapSensorType:
		{
			QTapSensor* s = dynamic_cast<QTapSensor*>(m_Sensor);
			QTapReading* reading = s->reading();
			msg.append("| Received tap sensor data\n");
			msg.append("| tap direction " + QString::number(reading->tapDirection()));
			if (reading->isDoubleTap())
				msg.append(", is double tap");
			else
				msg.append(", is not double tap");
			break;
		}
		default:
			break;
	}

	msg.append("\n+---------------------");

	qDebug() << msg;
	g_pResult->Write(msg);

	m_Sensor->stop();
	g_pTest->Stop();
}

void SensorsTest::onSensorError(int error)
{
	m_Sensor->stop();
	g_pTest->Stop();
	qCritical() << "Sensor error has occured" << error;
}

void SensorsTest::debugMessage()
{

	qDebug() << "+--------------------------------";

	qDebug() << "|Sensor type" << m_Sensor->type();

	if (m_Sensor->isConnectedToBackend())
	{
		qDebug() << "|Sensor is connected to backend";
	}
	else
	{
		qDebug() << "|Sensor is not connected to backend";
		qDebug() << "|trying to connect to backend...";
		if (m_Sensor->connectToBackend())
			qDebug() << "...true";
		else
			qDebug() << "|...false";
	}

	if (m_Sensor->isActive())
		qDebug() << "|Sensor is active";
	else
		qDebug() << "|Sensor is not active";

	if (m_Sensor->isBusy())
		qDebug() << "|Sensor is busy";
	else
		qDebug() << "|Sensor is not busy";

	qDebug() << "|error code " << m_Sensor->error();

	qDebug() << "+--------------------------------";


}

void SensorsTest::checkBackendAvailability()
{
	bool available = false;

	qDebug() << "Default sensors types are:";
	QList<QByteArray> list = QSensor::sensorTypes();
	qDebug() << "Sensor list:";
	foreach (QByteArray a, list)
	{
		qDebug() << a;
		if (a == QString(m_Sensor->type()))
			available = true;
	}

	if (available)
		qDebug() << "There is default backend for type " << m_Sensor->type();
	else
		qCritical() << "Default backend for type " << m_Sensor->type() << "does not exists";
}
