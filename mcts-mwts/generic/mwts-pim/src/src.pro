TARGET = mwts-pim
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += mobility
QT += network
MOBILITY = organizer versit contacts

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        PimTest.cpp

HEADERS += \
        PimTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target

RESOURCES += \
    pimtest.qrc
