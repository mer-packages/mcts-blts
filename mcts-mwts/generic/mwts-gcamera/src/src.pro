TARGET = mwts-gcamera
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

CONFIG += link_pkgconfig
PKGCONFIG += \
	glib-2.0 \
	dbus-1 \
    gstreamer-0.10

SOURCES += \
    GCameraTest.cpp

HEADERS += \
    GCameraTest.h

LIBS += -lgstphotography-0.10 -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
