# -------------------------------------------------
# Project created by QtCreator 2009-10-16T15:23:18
# -------------------------------------------------
QT += network
TARGET = ResourceViewer
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    incrementalplot.cpp

HEADERS += mainwindow.h \
    incrementalplot.h

FORMS += mainwindow.ui
INCLUDEPATH += /usr/local/qwt-5.2.0/include
LIBS += -L/usr/local/qwt-5.2.0/lib \
    -lqwt
