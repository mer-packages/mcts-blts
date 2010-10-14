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
lsw_SCRIPTS = 
lswdir = /opt/$(PACKAGE)/key_mgr
CFLAGS=`pkg-config --cflags glib-2.0 gobject-2.0 libsocialweb-keystore libsocialweb-keyfob`
LDFLAGS+=`pkg-config --libs glib-2.0 gobject-2.0 libsocialweb-keystore libsocialweb-keyfob`
lsw_PROGRAMS = keystore_testcase keyfob_sync_testcase keyfob_testcase
EXTRA_DIST = $(lsw_SCRIPTS) $(lsw_PROGRAMS)
