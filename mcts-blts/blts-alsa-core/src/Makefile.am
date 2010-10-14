# Makefile.am - Automake script for blts-alsa-core
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
	-O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -fPIC \
	-DBLTS_CONFIG_DIR=\"$(configdir)\"

requiredlibs = -lm -lrt

#if WANT_MIN
#min_libs = -lminutils -lmintmapi -lminevent
#else
#min_libs =
#endif

h_sources = \
	alsa_async.h \
	alsa_config.h \
	alsa_control.h \
	alsa_hwdep.h \
	alsa_ioctl.h \
	alsa_pcm.h \
	alsa_profiler.h \
	alsa_test_cases.h \
	alsa_timer.h \
	alsa_util.h \
	bt_util.h \
	v4l_tuner.h

c_sources = \
	alsa_test_cases.c \
	alsa_util.c \
	alsa_ioctl.c \
	alsa_pcm.c \
	alsa_control.c \
	alsa_timer.c \
	alsa_hwdep.c \
	alsa_conf_file.c \
	alsa_profiler.c \
	alsa_async.c \
	v4l_tuner.c \
	bt_util.c

library_includedir = $(includedir)/blts
#library_include_HEADERS = $(h_sources)

#if WANT_MIN

#lib_LTLIBRARIES = libbltsalsacore.la
#libbltsalsacore_la_SOURCES = $(h_sources) $(lib_h_sources) $(c_sources) $(lib_c_sources) min_scripter_mod.c
#libbltsalsacore_la_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS) $(DBUS_GLIB_CFLAGS) -fPIC
#libbltsalsacore_la_LIBADD = $(requiredlibs) $(BLTS_COMMON_LIBS) $(DBUS_GLIB_LIBS)
#libbltsalsacore_la_LDFLAGS = $(AM_LDFLAGS) -shared -module

#endif

bin_PROGRAMS = blts-alsa-core-tests

blts_alsa_core_tests_SOURCES = $(h_sources) $(c_sources) blts_alsa_core_test.c
blts_alsa_core_tests_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS) $(DBUS_GLIB_CFLAGS)
blts_alsa_core_tests_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS) $(DBUS_GLIB_LIBS)
