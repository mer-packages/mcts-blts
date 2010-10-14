#!/usr/bin/python

# Copyright (C) 2010 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU Lesser General Public License,
# version 2.1, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
# Authors:
#              Tang, Shao-Feng <shaofeng.tang@intel.com>
#

config_islastfm { 
    TARGET = lastfm_swclientservice_islastfm
    SOURCES += lastfm_swclientservice_islastfm.cpp
    HEADERS += lsw_testcases.h
}

config_openview { 
    TARGET = lastfm_swclientservice_openview
    SOURCES += lastfm_swclientservice_openview.cpp
    HEADERS += lsw_testcases.h
}

config_view_monitor { 
    TARGET = lastfm_swclientitemview_monitor
    SOURCES += lastfm_swclientitemview_monitor.cpp\
    	itemview_monitor.cpp
    HEADERS += lsw_testcases.h\
    	itemview_monitor.h
}

config_view_monitor_ma { 
    TARGET = lastfm_swclientitemview_monitor_ma
    SOURCES += lastfm_swclientitemview_monitor.cpp\
    	itemview_monitor_machel.cpp
    HEADERS += lsw_testcases.h\
    	itemview_monitor.h
}

config_refresh_view{
	TARGET = lastfm_swclientitemview_refresh_view
    SOURCES += lastfm_swclientitemview_refresh_view.cpp\
    	itemview_monitor.cpp
    HEADERS += lsw_testcases.h\
    	itemview_monitor.h
}

config_stop_view{
	TARGET = lastfm_swclientitemview_stop_view
    SOURCES += lastfm_swclientitemview_stop_view.cpp\
    	itemview_monitor.cpp
    HEADERS += lsw_testcases.h\
    	itemview_monitor.h
}

config_close_view{
	TARGET = lastfm_swclientitemview_close_view
    SOURCES += lastfm_swclientitemview_close_view.cpp\
    	itemview_monitor.cpp
    HEADERS += lsw_testcases.h\
    	itemview_monitor.h
}

QT += dbus
CONFIG += debug \
    link_pkgconfig \
    qdbus
TEMPLATE = app
PKGCONFIG += libsocialweb-qt
INCLUDEPATH += /usr/include/libsocialweb