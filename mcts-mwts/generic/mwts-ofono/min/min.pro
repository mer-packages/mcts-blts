TARGET = min-mwts-ofono
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += plugin
CONFIG += warn_on
CONFIG += debug

QT += network core dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

CONFIG += link_pkgconfig

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-ofono

target.path = /usr/lib/min
INSTALLS += target
