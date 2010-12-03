TARGET = min-mwts-network
TEMPLATE = lib
VERSION = 0.0.6

CONFIG += dll
CONFIG += warn_on
CONFIG += debug
CONFIG += link_pkgconfig
CONFIG += qtestlib

QT += network

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-network

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/mwts-network.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

ALLTESTS.files = data/tests.xml
ALLTESTS.path = /usr/share/mwts-network-scripts
INSTALLS += ALLTESTS

#TESTS.files = data/tests.xml
#TESTS.path = /usr/share/mwts-network-tests
#INSTALLS += TESTS

target.path = /usr/lib/min
INSTALLS += target
