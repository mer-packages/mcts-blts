TARGET = mwts-telepathy-cli
TEMPLATE = app

CONFIG += qt

CONFIG += qdbus
CONFIG += link_pkgconfig
CONFIG += link_prl
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += debug

QT += network


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

HEADERS += 

SOURCES += main.cpp

INCLUDEPATH += ../src

PKGCONFIG += TelepathyQt4

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-telepathy


target.path = /usr/bin
INSTALLS += target

