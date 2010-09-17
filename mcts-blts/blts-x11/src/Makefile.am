# Makefile.am - Automake script for blts-x11
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

AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -fPIC

requiredlibs = -lm -lrt -lX11 -lXv -lXtst -lXfixes -lXcomposite -lXrender -lXrandr -lXdamage -lXi

if WANT_MIN
min_libs = -lminutils -lmintmapi -lminevent
else
min_libs =
endif

h_sources = \
	blts_x11_util.h \
	xinput_tests.h \
	xdamage_tests.h \
	xrender_tests.h \
	xrandr_tests.h \
	xrecord_tests.h \
	xcomposite_tests.h \
	xtest_tests.h \
	xvideo_tests.h

c_sources = \
	blts_x11_util.c \
	xinput_tests.c \
	xdamage_tests.c \
	xrender_tests.c \
	xrandr_tests.c \
	xrecord_tests.c \
	xcomposite_tests.c \
	xtest_tests.c \
	xvideo_tests.c

library_includedir = $(includedir)/blts
#library_include_HEADERS = $(h_sources)

if WANT_MIN

lib_LTLIBRARIES = libbltsx11.la
libbltsx11_la_SOURCES = $(h_sources) $(lib_h_sources) $(c_sources) $(lib_c_sources) min_scripter_mod.c
libbltsx11_la_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS) -fPIC
libbltsx11_la_LIBADD = $(requiredlibs) $(BLTS_COMMON_LIBS)
libbltsx11_la_LDFLAGS = $(AM_LDFLAGS) -shared -module

endif

bin_PROGRAMS =	blts_x11_test \
				blts_xrender_bench

blts_x11_test_SOURCES = $(h_sources) $(c_sources) \
	blts_x11_test.c
blts_x11_test_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS)
blts_x11_test_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS)

blts_xrender_bench_SOURCES = blts_x11_util.c xrender_bench.c
blts_xrender_bench_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS)
blts_xrender_bench_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS)

