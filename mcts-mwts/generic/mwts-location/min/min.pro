TARGET = min-mwts-location
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += warn_on
CONFIG += debug
CONFIG += mobility

MOBILITY += location

QT += network

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src /usr/include/qt4/QtLocation

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-location

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTSXML.files = data/tests.xml
TESTSXML.path = /usr/share/mwts-location-scripts
INSTALLS += TESTSXML


target.path = /usr/lib/min
INSTALLS += target

