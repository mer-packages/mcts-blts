TARGET = min-mwts-bluetooth
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += warn_on
CONFIG += debug
QT += dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
    functions.cpp \
    interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-bluetooth

target.path = /usr/lib/min
INSTALLS += target

