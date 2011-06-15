TARGET = min-mwts-systeminfo
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += plugin
CONFIG += warn_on
CONFIG += debug
CONFIG += mobility
MOBILITY += systeminfo

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-systeminfo


target.path = /usr/lib/min
INSTALLS += target

