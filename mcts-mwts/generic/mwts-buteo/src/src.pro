TARGET = mwts-buteo
VERSION = 0.0.1
TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += link_pkgconfig
QT += network
QT += core
QT += xml


LIBS += -lsyncfwclient

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp
PRECOMPILED_HEADER = stable.h

PKGCONFIG += syncfwclient

LIBS += -lsynccommon \
    -lsyncprofile \
    -lsyncpluginmgr \
    -lsyncfwclient 

#INCLUDEPATH += /usr/include/signon-qt/

SOURCES += ButeoTest.cpp 
HEADERS += ButeoTest.h 

target.path = /usr/lib
INSTALLS += target
