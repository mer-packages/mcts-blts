#include <QApplication>
#include <QtDebug>
#include <QDebug>
#include <QStringList>
#include <QList>

#include <iostream>

#include "mysensor.h"


QTM_USE_NAMESPACE

int main(int argc, char *argv[]) {
     
	QApplication app(argc, argv);

	//MySensor* s = new MySensor("QTapSensor");
MySensor s;
	//s->start();


    	return app.exec();
}
