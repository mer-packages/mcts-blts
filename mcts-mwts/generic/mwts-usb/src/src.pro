TARGET = mwts-usb
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	UsbTest.cpp

HEADERS += \
	UsbTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
