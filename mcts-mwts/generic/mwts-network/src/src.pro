TARGET = mwts-network
VERSION = 1.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += link_pkgconfig
CONFIG += qtestlib

QT += network
QT += dbus
#CONFIG += mobility
#MOBILITY = bearer

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	NetworkTest.cpp \
	ftpwindow.cpp

HEADERS += \
	NetworkTest.h \
	ftpwindow.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
