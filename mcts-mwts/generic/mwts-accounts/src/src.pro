TARGET = mwts-accounts
VERSION = 0.0.1
TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += link_pkgconfig
QT += network
QT += core
QT += xml


LIBS += -lmwts-common
LIBS += -lsignon-qt
LIBS += -laccounts-qt

PKGCONFIG += libsignon-qt
PKGCONFIG += accounts-qt

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp
PRECOMPILED_HEADER = stable.h



#QMAKE_LIBDIR   += /usr/include/signon-qt/
#QMAKE_LIBDIR += /usr/include/accounts-qt/
#INCLUDEPATH += /usr/include/accounts-qt/
#INCLUDEPATH += /usr/include/signon-qt/

SOURCES += AccountsTest.cpp \
    signonclient.cpp
HEADERS += AccountsTest.h \
    signonclient.h



target.path = /usr/lib
INSTALLS += target
