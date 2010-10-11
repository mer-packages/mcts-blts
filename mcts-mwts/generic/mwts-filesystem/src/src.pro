TARGET = mwts-filesystem
VERSION = 0.0.1

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on

QT += network

CONFIG += link_pkgconfig

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	FilesystemTest.cpp

HEADERS += \
	FilesystemTest.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target

ASSET_CONFIG.files = data/FilesystemTest.conf
ASSET_CONFIG.path = /usr/lib/tests/
INSTALLS += ASSET_CONFIG

HEADERS.version = Versions
HEADERS.files = FilesystemTest.h
#HEADERS.path = /usr/include/MwtsFilesystem 
HEADERS.path = /usr/include
INSTALLS += HEADERS

