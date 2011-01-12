TARGET = min-mwts-buteo
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += plugin
CONFIG += warn_on
CONFIG += debug
CONFIG += link_pkgconfig

QT += network
QT += core
QT += xml


PKGCONFIG += syncfwclient
#PKGCONFIG += accounts-qt

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-buteo

LIBS += -lsynccommon \
    -lsyncprofile \
    -lsyncpluginmgr \
    -lsyncfwclient \


target.path = /usr/lib/min
INSTALLS += target

