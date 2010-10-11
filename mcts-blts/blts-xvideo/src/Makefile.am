# Makefile.am - Automake script for blts-xvideo-core
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
	xvideo_test_cases.h \
	xvideo_util.h

c_sources = \
   xvideo_test_cases.c \
   xvideo_util.c

library_includedir = $(includedir)/blts
#library_include_HEADERS = $(h_sources)

bin_PROGRAMS = blts_xvideo_test

blts_xvideo_test_SOURCES = $(h_sources) $(c_sources) blts_xvideo_test.c
blts_xvideo_test_CFLAGS = $(AM_CFLAGS) $(BLTSCOMMON_CFLAGS) $(BLTSPIXELFORMAT_CFLAGS) $(XVIDEO_CFLAGS)
blts_xvideo_test_LDFLAGS = $(BLTSCOMMON_LDFLAGS) $(XVIDEO_LDFLAGS) $(BLTSPIXELFORMAT_LDFLAGS)
blts_xvideo_test_LDADD = $(requiredlibs) $(BLTSCOMMON_LIBS) $(BLTSPIXELFORMAT_LIBS) $(XVIDEO_LIBS)

