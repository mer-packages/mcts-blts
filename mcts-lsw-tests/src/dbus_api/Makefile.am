# Copyright (C) 2010 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
 
#AUTOMAKE_OPTIONS=foreign
SUBDIRS=lswdbus model
lsw_SCRIPTS = core_getservices_method_main.py core_online_method_main.py core_online_signal_main.py \
dbus_core_signal.py \
dbus_lastfm_banishable.py dbus_lastfm_itemview.py dbus_user_changed_signal.py  service_user_changed_main.py \
lastfm_banishable_main.py lastfm_constant.py lastfm_main.py lastfm_newitem_main.py\
qt_hide_item_item_hidden_signal.py qt_lastfm_swclientitemview_monitor_comparing_with_dbus.py qt_userchanged_signal.py
lswdir = /opt/$(PACKAGE)/dbus_api
#LDFLAGS = -l glib-2.0 gobject-2.0 gnome-keyring-1
CFLAGS=`pkg-config --cflags glib-2.0 gobject-2.0 gnome-keyring-1`
LDFLAGS+=`pkg-config --libs glib-2.0 gobject-2.0 gnome-keyring-1`
lsw_PROGRAMS = reading_user_account setting_user_account
EXTRA_DIST = $(lsw_SCRIPTS) $(lsw_PROGRAMS)
