#include "mysensor.h"
#include <QtDebug>
#include <iostream> 
#include <QSensorReading>
#include <QTapReading>
#include <QTimer>

MySensor::MySensor()
{

	s = new QTapSensor(this);
	//s = new QSensor("TapSensor");

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

void MySensor::onReadingChanged()
{
	QTapSensor* s1 = dynamic_cast<QTapSensor*>(s);
	QTapReading* reading1 = s1->reading();
	qDebug() << reading1->tapDirection();
	//qDebug() << "Can read now";
}

void MySensor::onSensorError(int error)
{
	qDebug() << "error code " << error;
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
		qDebug() << "nto busy";

	qDebug() << "error code " << s->error();
}


