TARGET = min-mwts-qtmultimedia
TEMPLATE = lib
VERSION = 0.1.4

CONFIG += plugin
CONFIG += warn_on
CONFIG += debug
CONFIG += mobility
MOBILITY = multimedia


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-qtmultimedia

target.path = /usr/lib/min
INSTALLS += target

