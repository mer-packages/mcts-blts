TEMPLATE=app
TARGET= SensorTest
CONFIG += debug
CONFIG += warn_on
CONFIG += console
QT=core
CONFIG+=mobility
MOBILITY+=sensors
INCLUDEPATH += //usr/include/qt4/QtGui \
	../../../src/sensors \
	/usr/include/QtSensors 
SOURCES += main.cpp \
	accel.cpp
HEADERS += accel.h

CONFIG+=strict_flags

target.path = /opt/mcts-sensorfw-tests/
INSTALLS += target

TESTS.files = tests.xml
TESTS.path = /opt/mcts-sensorfw-tests/
INSTALLS += TESTS

