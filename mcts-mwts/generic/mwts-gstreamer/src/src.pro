TARGET = mwts-gstreamer
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
	gstreamer.cpp

HEADERS += \
	gstreamer.h


LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
