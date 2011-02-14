TARGET = mwts-location
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
#CONFIG += link_pkgconfig
QT += network
CONFIG += mobility

MOBILITY = location

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
    LocationTest.cpp \
    TestDriver.cpp

HEADERS += \
    LocationTest.h \
    TestDriver.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
