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

config_get_service { 
    TARGET = core_get_services
    SOURCES += core_get_services.cpp
}
config_is_online { 
	TARGET = core_is_online
	SOURCES += core_is_online.cpp
    
}
config_hide_item { 
    TARGET = core_hide_item
    SOURCES += core_hide_item.cpp
}

QT += dbus
CONFIG += debug \
    link_pkgconfig \
    qdbus
TEMPLATE = app
PKGCONFIG += libsocialweb-qt
INCLUDEPATH += /usr/include/libsocialweb
HEADERS = lsw_testcases.h
