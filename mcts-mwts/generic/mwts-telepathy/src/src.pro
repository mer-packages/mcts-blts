TARGET = mwts-telepathy
TEMPLATE = lib
CONFIG = dll
CONFIG += qt

CONFIG += qdbus
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += debug

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp
QT += network

PRECOMPILED_HEADER = stable.h

# Input
HEADERS += stable.h \
    TelepathyTest.h \
    Call.h \
    TextChat.h \
    Contacts.h \
    ClientHandler.h \
    PendingOperationWaiter.h 
    
SOURCES += TelepathyTest.cpp \
    Call.cpp \
    TextChat.cpp \
    Contacts.cpp \
    ClientHandler.cpp \
    PendingOperationWaiter.cpp
    
PKGCONFIG += TelepathyQt4

LIBS += -lmwts-common
target.path = /usr/lib
INSTALLS += target

DEFINES += TP_QT4_ENABLE_LOWLEVEL_API
