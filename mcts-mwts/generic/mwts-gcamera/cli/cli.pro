TARGET = mwts-gcamera-cli
TEMPLATE = app
VERSION = 0.0.1

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += release
CONFIG += debug
CONFIG += console

QT += dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

CONFIG += link_pkgconfig
PKGCONFIG += \
    glib-2.0 \
    dbus-1 \
    gstreamer-0.10

SOURCES += \
        main.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lmwts-common -lgstphotography-0.10 -lmwts-gcamera

target.path = /usr/bin
INSTALLS += target

