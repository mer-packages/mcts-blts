TARGET = min-mwts-buteo
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
<<<<<<< HEAD
=======
CONFIG += plugin
>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
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

