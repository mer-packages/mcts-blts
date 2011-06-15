TARGET = min-mwts-pim
TEMPLATE = lib
VERSION = 0.0.2

CONFIG += dll
CONFIG += plugin
CONFIG += warn_on
CONFIG += debug

CONFIG += mobility
MOBILITY = organizer versit contacts

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-pim

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTS.files = data/tests.xml
TESTS.path = /usr/share/mwts-pim-generic-tests
INSTALLS += TESTS

target.path = /usr/lib/min
INSTALLS += target

