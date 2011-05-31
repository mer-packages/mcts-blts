#ifndef SUPERSENSOR_H
#define SUPERSENSOR_H

#include <QWidget>
#include <QSensor>
#include <QTapSensor>

#include "foobarclass.h"

QTM_USE_NAMESPACE

class SuperSensor : public FooBarClass
//class SuperSensor : public MwtsTest
{
    Q_OBJECT

public:

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

   //SuperSensor(SuperSensor::SensorType);
   //explicit SuperSensor(SuperSensor::SensorType, QObject *parent = 0);
   SuperSensor();

   void startToFuck(SuperSensor::SensorType);

public slots:
   void onActiveChanged ();
   void onAvailableSensorsChanged ();
   void onBusyChanged ();
   void onReadingChanged ();
   void onSensorError ( int error );

   void show();

   void printSensorsTypes();

   void OnInitialize();
   void OnUninitialize();

private:
   QSensor* s;
   SensorType sType;

};

#endif // SUPERSENSOR_H
