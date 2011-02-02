TARGET = mwts-multimedia
VERSION = 0.0.1

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
        MultimediaTest.cpp \
        AudioPlayerTest.cpp

HEADERS += \
        AudioRecorderTest.h \
        ImageViewerTest.h \
        FMRadioTest.h \
        MultimediaTest.h \
        AudioPlayerTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target
