TARGET = min-mwts-gcamera
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += plugin
CONFIG += warn_on
CONFIG += debug

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

CONFIG += link_pkgconfig
PKGCONFIG += \
	glib-2.0 \
	dbus-1 \
    gstreamer-0.10

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lgstphotography-0.10 -lmwts-gcamera

target.path = /usr/lib/min
INSTALLS += target

