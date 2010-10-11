
TEMPLATE = app
TARGET = selftest
DEPENDPATH += . ../../src
INCLUDEPATH += . ../../src
QT += network dbus xml

# Input
HEADERS += MyTest.h
SOURCES += MyTest.cpp selftest.cpp

LIBS = \
	-L../../src \
	-lmwts-common \
	-lminevent \
	-lmintmapi

