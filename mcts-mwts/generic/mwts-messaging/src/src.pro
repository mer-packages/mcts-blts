TARGET				= mwts-messaging
VERSION				= 0.0.1
TEMPLATE			= lib


CONFIG				+= debug
CONFIG				+= warn_on
CONFIG				+= link_pkgconfig
CONFIG				+= mobility
MOBILITY			 = messaging

QT					+= network
QT					+= gui

MOC_DIR				 = ../tmp
OBJECTS_DIR			 = ../tmp

PRECOMPILED_HEADER	 = stable.h

SOURCES				+= MessagingTest.cpp
HEADERS				+= MessagingTest.h

LIBS				+= -lmwts-common

target.path = /usr/lib
INSTALLS += target
