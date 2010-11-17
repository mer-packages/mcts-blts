TARGET = mwts-accounts
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	AccountsTest.cpp

HEADERS += \
	AccountsTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
