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

MIN_CONFIG.version = Versions
MIN_CONFIG.files = ../scripts/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = ../scripts/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTS.files = ../scripts/tests.xml
TESTS.path = /usr/share/mwts-systeminfo-scripts
INSTALLS += TESTS

target.path = /usr/lib/min
INSTALLS += target

