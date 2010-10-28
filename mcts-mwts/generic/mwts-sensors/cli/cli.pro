TARGET = mwts-sensors-cli
TEMPLATE = app
VERSION = 0.0.1

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += release

CONFIG += mobility
MOBILITY = sensors


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        main.cpp

HEADERS += \
	../src/SensorsTest.h

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lmintmapi -lmwts-common -lmwts-sensors


target.path = /usr/bin
INSTALLS += target

