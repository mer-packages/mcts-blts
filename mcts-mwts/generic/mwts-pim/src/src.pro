TARGET = mwts-pim
VERSION = 0.0.2

TEMPLATE = lib
CONFIG += debug
CONFIG += warn_on
CONFIG += mobility
QT += network
MOBILITY = organizer versit contacts

PKGCONFIG += QtVersitOrganizer
PKGCONFIG += QtVersit

MOC_DIR = ../tmp
OBJECTS_DIR = ../tmp

PRECOMPILED_HEADER = stable.h

SOURCES += \
        PimTest.cpp \
        organizertest.cpp \
        contactstest.cpp \
        versit.cpp \
        pimcontactdetailmanager.cpp \
        pimorganizeritemmanager.cpp

HEADERS += \
        PimTest.h \
        organizertest.h \
        contactstest.h \
        versit.h \
        PimItem.h \
        PimDataStore.h \
        pimcontactdetailmanager.h \
        pimorganizeritemmanager.h

LIBS += -lmwts-common
	
target.path = /usr/lib
INSTALLS += target

RESOURCES += \
    pimtest.qrc
