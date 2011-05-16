#include "mysensor.h"
#include <QtDebug>
#include <iostream> 
#include <QSensorReading>
#include <QTapReading>

#include <QAccelerometer>
#include <QAccelerometerReading>

//#include <QAmbientLightSensor>
//#include <QAmbientLightSensorReading>

#include <QCompass>
#include <QCompassReading>

#include <QRotationSensor>
#include <QRotationReading>

#include <QLightSensor>
#include <QLightReading>

#include <QProximitySensor>
#include <QProximityReading>

#include <QOrientationSensor>
#include <QOrientationReading>

#include <QMagnetometer>
#include <QMagnetometerReading>

#include <QTimer>

MySensor::MySensor(MySensor::SensorType type) : sType(type)
{
	//sType = type;
//

	printSensorsTypes();


	switch (sType)
	{
		case AccelerometerType:
			s = new QAccelerometer(this);
			qDebug() << "Accelerometer sensor initialized";
			break;
		case AmbientLightSensorType:
			//s = new QAmbientLightSensor(this);
			//qDebug() << "Ambient light sensor initialized";
			//break;
		case CompassType:
			s = new QCompass(this);
			qDebug() << "Compass sensor initialized";
			break;
		case GyroscopeType:
			//s = new QGyroscope(this);
			break;
		case LightSensorType:
			s = new QLightSensor(this);
			qDebug() << "Light sensor initialized";
			break;
		case MagnetometerType:
			s = new QMagnetometer(this);
			qDebug() << "Magnetometer sensor initialized";
			break;
		case OrientationSensorType:
			s = new QOrientationSensor(this);
			qDebug() << "Orientation sensor initialized";
			break;
		case ProximitySensorType:
			s = new QProximitySensor(this);
			qDebug() << "Proximity sensor initialized";
			break;
		case RotationSensorType:
			s = new QRotationSensor(this);
			qDebug() << "Rotation sensor initialized";
			break;
		case TapSensorType:
			s = new QTapSensor(this);
			qDebug() << "Tap sensor initialized";
			break;
		default:
			s = new QAccelerometer(this);
			qDebug() << " Default initialization accelerometer";
			break;
	}

	//s = new QTapSensor(this);
	//s = new QAccelerometer(this);
	//s = new QCompass(this);
	//s = new QRotationSensor(this);
	//s = new QLightSensor(this);
	//s = new QProximitySensor(this);
	//s = new QOrientationSensor(this);
	//s = new QMagnetometer(this);


	connect(s, SIGNAL(activeChanged()), this, SLOT(onActiveChanged()));
	connect(s, SIGNAL(availableSensorsChanged()), this, SLOT(onAvailableSensorsChanged()));
	connect(s, SIGNAL(busyChanged()), this, SLOT(onBusyChanged()));
	connect(s, SIGNAL(readingChanged()), this, SLOT(onReadingChanged()));
	connect(s, SIGNAL(sensorError(int)), this, SLOT(onSensorError(int)));

	QTimer* t = new QTimer(this);
	connect(t, SIGNAL(timeout()), this, SLOT(show()));
	t->start(1000);

	s->start();

}

void MySensor::onActiveChanged()
{
	qDebug() << "active";
}

void MySensor::onAvailableSensorsChanged()
{
	qDebug() << "available";
}

void MySensor::onBusyChanged()
{
	qDebug() << "busy";
}

void MySensor::printSensorsTypes()
{
	QList<QByteArray> list = QSensor::sensorTypes();
	qDebug() << "Sensor list:";
	foreach (QByteArray a, list)
		qDebug() << a;

}

void MySensor::onReadingChanged()
{
	switch (sType)
	{
		case AccelerometerType:
		{
			QAccelerometer* s1 = dynamic_cast<QAccelerometer*>(s);
			QAccelerometerReading* reading = s1->reading();
			qDebug() << "x " << reading->x() << ", y " << reading->y() << ", z " << reading->z();
			break;
		}
		case AmbientLightSensorType:
		{
			break;
		}
		case CompassType:
		{
			QCompass* s1 = dynamic_cast<QCompass*>(s);
			QCompassReading* reading = s1->reading();
			qDebug() << "azimuth " << reading->azimuth() << ", calibration level " << reading->calibrationLevel();
			break;
		}
		case GyroscopeType:

			break;
		case LightSensorType:
		{
			QLightSensor* s1 = dynamic_cast<QLightSensor*>(s);
			QLightReading* reading = s1->reading();
			qDebug() << "lux " << reading->lux();
			break;
		}
		case MagnetometerType:
		{
			QMagnetometer* s1 = dynamic_cast<QMagnetometer*>(s);
			QMagnetometerReading* reading = s1->reading();
			qDebug() << "calibration level " << reading->calibrationLevel() << ", x " << reading->x() << ", y " << reading->y() << ", z" << reading->z();
			break;
		}
		case OrientationSensorType:
		{
			QOrientationSensor* s1 = dynamic_cast<QOrientationSensor*>(s);
			QOrientationReading* reading = s1->reading();
			qDebug() << "orientation " << reading->orientation();
			break;
		}
		case ProximitySensorType:
		{
			QProximitySensor* s1 = dynamic_cast<QProximitySensor*>(s);
			QProximityReading* reading = s1->reading();
			qDebug() << "close " << reading->close();
			break;
		}
		case RotationSensorType:
		{
			QRotationSensor* s1 = dynamic_cast<QRotationSensor*>(s);
			QRotationReading* reading = s1->reading();
			qDebug() << "x " << reading->x() << ", y " << reading->y() << ", z " << reading->z();
			break;
		}
		case TapSensorType:
		{
			QTapSensor* s1 = dynamic_cast<QTapSensor*>(s);
			QTapReading* reading = s1->reading();
			qDebug() << "tap direction " << reading->tapDirection() << ", is double tap " << reading->isDoubleTap();
			break;
		}
		default:
			break;
	}

	//QTapSensor* s1 = dynamic_cast<QTapSensor*>(s);
	//QTapReading* reading1 = s1->reading();
	//qDebug() << "tap direction " << reading1->tapDirection() << ", is double tap " << reading1->isDoubleTap();

	//QAccelerometer* s1 = dynamic_cast<QAccelerometer*>(s);
	//QAccelerometerReading* reading1 = s1->reading();
	//qDebug() << "x " << reading1->x() << ", y " << reading1->y() << ", z " << reading1->z();

	//QCompass* s1 = dynamic_cast<QCompass*>(s);
	//QCompassReading* reading1 = s1->reading();
	//qDebug() << "azimuth " << reading1->azimuth() << ", calibration level " << reading1->calibrationLevel();

	//QRotationSensor* s1 = dynamic_cast<QRotationSensor*>(s);
	//QRotationReading* reading1 = s1->reading();
	//qDebug() << "x " << reading1->x() << ", y " << reading1->y() << ", z " << reading1->z();

	//QLightSensor* s1 = dynamic_cast<QLightSensor*>(s);
	//QLightReading* reading1 = s1->reading();
	//qDebug() << "lux " << reading1->lux();

	//QProximitySensor* s1 = dynamic_cast<QProximitySensor*>(s);
	//QProximityReading* reading1 = s1->reading();
	//qDebug() << "close " << reading1->close();

	//QProximitySensor* s1 = dynamic_cast<QProximitySensor*>(s);
	//QProximityReading* reading1 = s1->reading();
	//qDebug() << "close " << reading1->close();

	//QOrientationSensor* s1 = dynamic_cast<QOrientationSensor*>(s);
	//QOrientationReading* reading1 = s1->reading();
	//qDebug() << "orientation " << reading1->orientation();

	//QMagnetometer* s1 = dynamic_cast<QMagnetometer*>(s);
	//QMagnetometerReading* reading1 = s1->reading();
	//qDebug() << "calibration level " << reading1->calibrationLevel() << ", x " << reading1->x() << ", y " << reading1->y() << ", z" << reading1->z();
}

void MySensor::onSensorError(int error)
{
	qDebug() << "error occurs, code: " << error;
}


void MySensor::show()
{
	if (s->isConnectedToBackend())
		qDebug() << "connected";
	else
		qDebug() << "not connected";

	if (s->isActive())
		qDebug() << "active";
	else
		qDebug() << "not active";

	if (s->isBusy())
		qDebug() << "busy";
	else
		qDebug() << "not busy";

	qDebug() << "error code " << s->error();
}


