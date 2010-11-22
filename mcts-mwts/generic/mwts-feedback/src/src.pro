TARGET = mwts-feedback
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
#QT += network

CONFIG	+= mobility
MOBILITY = feedback

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        FeedbackTest.cpp

HEADERS += \
	FeedbackTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
