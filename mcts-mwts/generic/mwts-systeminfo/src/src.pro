TARGET = mwts-systeminfo
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network
CONFIG += mobility
MOBILITY += systeminfo

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
    SystemInfoTest.cpp

HEADERS += \
    SystemInfoTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
