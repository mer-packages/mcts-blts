TARGET = min-mwts-telepathy
TEMPLATE = lib

CONFIG = plugin
CONFIG += qt

CONFIG += qdbus
CONFIG += link_pkgconfig
CONFIG += build_all
CONFIG += lib_bundle
CONFIG += warn_on
CONFIG += debug

#QT += network

# TelepathyQt is currently out of sync with current Qt release.
# Every now and then, try to compile without this depedency.
# If it compiles, it can be removed for good.
DEFINES += HAVE_QDBUSVARIANT_OPERATOR_EQUAL

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
	functions.cpp \
	interface.cpp

INCLUDEPATH += ../src

PKGCONFIG += TelepathyQt4

LIBS+= -L../src
LIBS += -lminutils -lmintmapi -lminevent -lmwts-common -lmwts-telepathy

MIN_CONFIG.version = Versions
MIN_CONFIG.files = data/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

TESTS.files = data/tests.xml
TESTS.path = /usr/share/mwts-telepathy-tests
ALLTESTS.files = data/alltests.xml
ALLTESTS.path = /usr/share/mwts-telepathy-scripts
INSTALLS += TESTS
INSTALLS += ALLTESTS

target.path = /usr/lib/min
INSTALLS += target

