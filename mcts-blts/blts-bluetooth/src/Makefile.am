# Makefile.am - Automake script for blts-bluetooth
#
#  Copyright (C) 2000-2010, Nokia Corporation.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

AM_CFLAGS = \
	-O2 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. \
	$(PACKAGE_CFLAGS) -fno-builtin-sscanf

requiredlibs = -lm -lrt $(PACKAGE_LIBS)

if WANT_MIN
min_libs = -lminutils -lmintmapi -lminevent
else
min_libs =
endif

h_sources = \
	bt_fute.h \
	bt_ctx.h \
	hci.h \
	l2cap.h \
	rfcomm.h \
	gen_client.h \
	gen_server.h \
	ext_tools.h \
	pairing.h \
	info.h

c_sources = \
	bt_fute.c \
	bt_ctx.c \
	hci.c \
	l2cap.c \
	rfcomm.c \
	gen_client.c \
	gen_server.c \
	ext_tools.c \
	pairing.c \
	info.c

if WANT_MIN

lib_LTLIBRARIES = libbltsbluetooth.la
libbltsbluetooth_la_SOURCES = $(h_sources) $(c_sources) min_scripter_mod.c
libbltsbluetooth_la_CFLAGS = $(AM_CFLAGS) -fPIC
libbltsbluetooth_la_LIBADD = $(requiredlibs) $(min_libs)
libbltsbluetooth_la_LDFLAGS = $(AM_LDFLAGS) -shared -module

endif

bin_PROGRAMS = blts-bluetooth-tests

blts_bluetooth_tests_SOURCES = $(h_sources) $(c_sources) bt_fute_cli.c
blts_bluetooth_tests_CFLAGS = $(AM_CFLAGS)
blts_bluetooth_tests_LDADD = $(requiredlibs)
