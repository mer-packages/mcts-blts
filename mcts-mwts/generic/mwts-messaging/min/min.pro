TARGET				 = min-mwts-messaging
TEMPLATE			 = lib
VERSION				 = 0.0.1

CONFIG				+= dll
CONFIG				+= warn_on
CONFIG				+= debug
CONFIG				+= mobility
MOBILITY			 = messaging

MOC_DIR				 = ../tmp
OBJECTS_DIR			 = ../tmp

SOURCES				+= functions.cpp \
					   interface.cpp

INCLUDEPATH			+= ../src

LIBS				+= -L../src
LIBS				+= -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-messaging

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTS.files = data/tests.xml
TESTS.path = /usr/share/mwts-messaging-tests
INSTALLS += TESTS

ALLTESTS.files = data/tests.xml
ALLTESTS.path = /usr/share/mwts-messaging-scripts
INSTALLS += ALLTESTS

target.path = /usr/lib/min
INSTALLS += target

