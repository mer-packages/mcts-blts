#ifndef MYS_H
#define MYS_H

#include <QSensor>
#include <QTapSensor>

QTM_USE_NAMESPACE

class MySensor : public QObject {

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

	MySensor(MySensor::SensorType);

 public slots:
	void onActiveChanged ();
	void onAvailableSensorsChanged ();
	void onBusyChanged ();
	void onReadingChanged ();
	void onSensorError ( int error );

	void show();

	void printSensorsTypes();

private:
	QSensor* s;
	SensorType sType;

 };

#endif // MYS_H
