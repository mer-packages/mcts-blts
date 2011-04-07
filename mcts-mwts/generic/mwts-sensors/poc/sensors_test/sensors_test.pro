TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += debug warm_on


CONFIG += mobility
MOBILITY = sensors
 

# Input
HEADERS += mysensor.h
SOURCES += main.cpp mysensor.cpp
