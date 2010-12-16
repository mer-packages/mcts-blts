TARGET = min-mwts-feedback
TEMPLATE = lib
VERSION = 0.0.1

CONFIG += dll
CONFIG += warn_on
CONFIG += debug

CONFIG += mobility
MOBILITY = feedback

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-feedback

target.path = /usr/lib/min
INSTALLS += target


OTHER_FILES += \
    ../scripts/mwts-feedback.cfg \
    ../scripts/mwts-feedback.min.conf \
    ../scripts/mwts-feedback-nft.cfg \
    ../scripts/tests.xml

