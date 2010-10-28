TARGET = mwts-sensors
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network

CONFIG += mobility
MOBILITY = sensors


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	SensorsTest.cpp

HEADERS += \
	SensorsTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
HEADERS.version = Versions
HEADERS.files = SensorsTest.h
HEADERS.path = /usr/include
INSTALLS += HEADERS
