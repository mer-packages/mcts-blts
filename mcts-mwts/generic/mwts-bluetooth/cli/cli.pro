TARGET = mwts-bluetooth-cli
TEMPLATE = app
VERSION = 0.0.1

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += release

QT += dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
    main.cpp

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lmwts-common -lmwts-bluetooth


target.path = /usr/bin
INSTALLS += target

