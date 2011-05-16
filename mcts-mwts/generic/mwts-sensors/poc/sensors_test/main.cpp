#include <QApplication>
#include <QtDebug>
#include <QDebug>
#include <QStringList>
#include <QList>

#include <iostream>

//#include "SuperSensor.h"
#include "supersensor.h"


QTM_USE_NAMESPACE

int main(int argc, char *argv[]) {
     
	//QApplication app(argc, argv);

	int i = atoi(argv[1]);

	/*
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
	*/

	SuperSensor* s = new SuperSensor();

	s->Initialize();

	s->startToFuck(SuperSensor::AccelerometerType);

	/*switch (i)
	{
	case 0:
		s = new SuperSensor(SuperSensor::AccelerometerType);
		break;
	case 1:
		s = new SuperSensor(SuperSensor::AmbientLightSensorType);
		break;
	case 2:
		s = new SuperSensor(SuperSensor::CompassType);
		break;
	case 3:
		s = new SuperSensor(SuperSensor::LightSensorType);
		break;
	case 4:
		s = new SuperSensor(SuperSensor::MagnetometerType);
		break;
	case 5:
		s = new SuperSensor(SuperSensor::MagnetometerType);
		break;
	case 6:
		s = new SuperSensor(SuperSensor::OrientationSensorType);
		break;
	case 7:
		s = new SuperSensor(SuperSensor::ProximitySensorType);
		break;
	case 8:
		s = new SuperSensor(SuperSensor::RotationSensorType);
		break;
	case 9:
		s = new SuperSensor(SuperSensor::TapSensorType);
		break;
	default:
		break;
	}*/

	//SuperSensor::SensorType type = i; //SuperSensor::AccelerometerType;

	//SuperSensor* s = new SuperSensor("QTapSensor");
//SuperSensor s(type);
	//s->start();



s->Start();

	return 0;
		//return app.exec();
}
