TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += debug warm_on


CONFIG += mobility
MOBILITY = sensors
 

# Input
HEADERS += mysensor.h \
    supersensor.h \
    foobarclass.h
SOURCES += main.cpp mysensor.cpp \
    supersensor.cpp \
    foobarclass.cpp

LIBS += -lmwts-common
