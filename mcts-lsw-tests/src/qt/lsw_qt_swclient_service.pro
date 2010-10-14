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

config_getSN { 
    TARGET = swclient_static_get_service_name
    SOURCES += swclient_static_get_service_name.cpp
    HEADERS += lsw_testcases.h
}

config_getSI { 
    TARGET = swclient_get_service_instance
    SOURCES += swclient_get_service_instance.cpp \
    		service_parent.cpp
    HEADERS += lsw_testcases.h \
			service_parent.h
}

config_getUnSI { 
    TARGET = swclient_get_unavailable_service_instance
    SOURCES += swclient_get_unavailable_service_instance.cpp
    HEADERS += lsw_testcases.h
}

QT += dbus
CONFIG += debug \
    link_pkgconfig \
    qdbus
TEMPLATE = app
PKGCONFIG += libsocialweb-qt
INCLUDEPATH += /usr/include/libsocialweb