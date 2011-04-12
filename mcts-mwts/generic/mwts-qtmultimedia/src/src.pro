TARGET = mwts-qtmultimedia
VERSION = 0.1.4

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += mobility

QT += network
QT += opengl
MOBILITY = multimedia


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        AudioRecorderTest.cpp \
        ImageViewerTest.cpp \
        FMRadioTest.cpp \
        MultimediaTest.cpp

HEADERS += \
        AudioRecorderTest.h \
        ImageViewerTest.h \
        FMRadioTest.h \
        MultimediaTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
