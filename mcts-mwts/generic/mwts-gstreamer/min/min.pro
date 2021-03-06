TARGET = min-mwts-gstreamer
TEMPLATE = lib
VERSION = 0.0.1

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
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-gstreamer

target.path = /usr/lib/min
INSTALLS += target

