TARGET = mwts-feedback-cli
TEMPLATE = app
VERSION = 0.0.1

CONFIG += qt
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += debug
#CONFIG += release
CONFIG += mobility
MOBILITY = feedback

PKGCONFIG += QtFeedback


MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        main.cpp

HEADERS += \
        ../src/FeedbackTest.h

INCLUDEPATH += ../src


LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-feedback


target.path = /usr/bin
INSTALLS += target

