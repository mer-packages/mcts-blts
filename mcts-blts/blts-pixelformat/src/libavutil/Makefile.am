# Makefile.am -- Automake script for libavutil
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

AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -I.

# not needed in distribution
local_h_sources = \
	pixfmt.h \
	avutil.h \
	bswap.h \
	common.h \
	internal.h \
	mathematics.h \
	mem.h \
	rational.h \
	timer.h \
	intfloat_readwrite.h

c_sources = \
	intfloat_readwrite.c \
	mathematics.c \
	mem.c \
	rational.c

library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

noinst_LTLIBRARIES= \
	libavutil.la

libavutil_la_LIBADD = -lm -lbltscommon
libavutil_la_SOURCES= $(parser_sources) $(h_sources) $(local_h_sources) $(c_sources)
libavutil_la_LDFLAGS=
