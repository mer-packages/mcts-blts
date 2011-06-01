TARGET = min-mwts-common
TEMPLATE = lib
VERSION = 1.2.4

CONFIG += dll
QT += network dbus

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH = ../src
LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_CONFIG_OVERRIDE.files = data/min.conf
MIN_CONFIG_OVERRIDE.path = /etc/min.d
INSTALLS += MIN_CONFIG_OVERRIDE

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

DESKTOP.files = data/*.desktop
DESKTOP.path = /usr/share/applications
INSTALLS += DESKTOP

target.path = /usr/lib/min
INSTALLS += target
