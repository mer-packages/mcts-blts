TARGET = min-mwts-sensors
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += debug
CONFIG += warn_on
QT += network

CONFIG += mobility
MOBILITY = sensors

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS += -L../src -L/usr/lib
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-sensors

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTS.files = data/tests.xml
TESTS.path = /usr/share/mwts-sensors-scripts
INSTALLS += TESTS

target.path = /usr/lib/min
INSTALLS += target

