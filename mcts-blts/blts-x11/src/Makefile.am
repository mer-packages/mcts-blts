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

requiredlibs = -lm -lrt

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

bin_PROGRAMS =	blts-x11-tests \
				blts-xrender-bench

blts_x11_tests_SOURCES = $(h_sources) $(c_sources) blts_x11_test.c
blts_x11_tests_CFLAGS = $(AM_CFLAGS) $(PACKAGE_CFLAGS)
blts_x11_tests_LDADD = $(requiredlibs) $(PACKAGE_LIBS)

blts_xrender_bench_SOURCES = blts_x11_util.c xrender_bench.c
blts_xrender_bench_CFLAGS = $(AM_CFLAGS) $(PACKAGE_CFLAGS)
blts_xrender_bench_LDADD = $(requiredlibs) $(PACKAGE_LIBS)

