TARGET = mwts-ofono
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network core dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

CONFIG += link_pkgconfig
PKGCONFIG += dbus-1 

SOURCES += \
    ofonotest.cpp \
    simmanagertest.cpp \
    voicecallmanagertest.cpp

HEADERS += \
    ofonotest.h \
    simmanagertest.h \
    voicecallmanagertest.h

LIBS += -lmwts-common -lofono-qt

target.path = /usr/lib
INSTALLS += target
