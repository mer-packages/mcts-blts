TARGET = mwts-filesystem-cli
TEMPLATE = app
VERSION = 0.0.1

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += release

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        main.cpp

HEADERS += \
	../src/FilesystemTest.h

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lmintmapi -lmwts-common -lmwts-filesystem


target.path = /usr/bin
INSTALLS += target

