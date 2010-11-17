TARGET = mwts-bluetooth
VERSION = 0.0.1
TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
QT += network \
    dbus
MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp
CONFIG += link_pkgconfig

# PKGCONFIG += \
# glib-2.0 \
PRECOMPILED_HEADER = stable.h
SOURCES += BluetoothTest.cpp \
    BluetoothSocket.cpp \
    bluetoothdbus.cpp \
    dbus-headers/agentrelay.cpp \
    dbus-headers/agent.cpp \
    dbus-headers/ObexClientAgent.cpp \
    dbus-headers/ObexClientAgentRelay.cpp \
    dbus-headers/ObexServerAgent.cpp \
    dbus-headers/ObexServerAgentRelay.cpp \
    bluetoothhci.cpp \
    BluetoothApiInterface.cpp \
    BluetoothObex.cpp
HEADERS += BluetoothTest.h \
    BluetoothSocket.h \
    bluetoothdbus.h \
    dbus-headers/agentrelay.h \
    dbus-headers/agent.h \
    dbus-headers/ObexClientAgent.h \
    dbus-headers/ObexClientAgentRelay.h \
    dbus-headers/ObexServerAgent.h \
    dbus-headers/ObexServerAgentRelay.h \
    bluetoothhci.h \
    BluetoothApiInterface.h \
    BluetoothObex.h \
    BluetoothCommonTypes.h
LIBS += -lmwts-common \
    -lbluetooth
target.path = /usr/lib
INSTALLS += target
