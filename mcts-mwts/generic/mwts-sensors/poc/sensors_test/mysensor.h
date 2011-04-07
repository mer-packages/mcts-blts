#ifndef MYS_H
#define MYS_H

#include <QSensor>
#include <QTapSensor>

QTM_USE_NAMESPACE

class MySensor : public QObject {

     Q_OBJECT

 public:
	MySensor();


 public slots:
	void onActiveChanged ();
	void onAvailableSensorsChanged ();
	void onBusyChanged ();
	void onReadingChanged ();
	void onSensorError ( int error );

	void show();

private:
	QSensor* s;

 };

#endif // MYS_H
