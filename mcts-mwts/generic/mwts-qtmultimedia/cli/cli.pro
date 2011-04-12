TARGET = mwts-qtmultimedia-cli
TEMPLATE = app
VERSION = 0.1.4

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += release
CONFIG += debug
CONFIG += console

QT += dbus
CONFIG += mobility
MOBILITY = multimedia


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

HEADERS += ../src/MultimediaTest.h
SOURCES += main.cpp

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lmwts-common -lmwts-qtmultimedia


target.path = /usr/bin
INSTALLS += target

