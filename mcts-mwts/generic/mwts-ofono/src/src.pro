TARGET = mwts-ofono
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network dbus core

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

CONFIG += link_pkgconfig

SOURCES += \
    ofonotest.cpp

HEADERS += \
    ofonotest.h

LIBS += -lgstphotography-0.10 -lmwts-common -lofono-qt
	
target.path = /usr/lib
INSTALLS += target
