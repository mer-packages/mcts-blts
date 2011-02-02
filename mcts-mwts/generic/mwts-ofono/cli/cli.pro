TARGET = mwts-ofono-cli
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
CONFIG += link_prl
CONFIG += qdbus

QT += network core dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h ofonotest.h

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1

SOURCES += main.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-ofono

target.path = /usr/bin
INSTALLS += target

