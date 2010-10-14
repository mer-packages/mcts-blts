#!/bin/sh

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


qmake lsw_qt_tcases.pro CONFIG+=config_get_service
make -w

#qmake lsw_qt_tcases.pro CONFIG+=config_is_online
#make -w

#qmake lsw_qt_tcases.pro CONFIG+=config_hide_item
#make -w

qmake lsw_qt_swclient_service.pro CONFIG+=config_getSN
make -w

qmake lsw_qt_swclient_service.pro CONFIG+=config_getSI
make -w

qmake lsw_qt_swclient_service.pro CONFIG+=config_getUnSI
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_islastfm
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_openview
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_view_monitor
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_view_monitor_ma
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_refresh_view
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_stop_view
make -w

qmake lsw_qt_lastfm_service.pro CONFIG+=config_close_view
make -w

qmake lsw_qt_common_service.pro CONFIG+=config_user_changed
make -w

qmake lsw_qt_common_service.pro CONFIG+=config_item_hidden
make -w

qmake lsw_qt_common_service.pro CONFIG+=config_hide_item
make -w
