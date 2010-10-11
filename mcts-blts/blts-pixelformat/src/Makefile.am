# Makefile.am -- Automake script for blts-pixelformat
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

SUBDIRS = libavcodec libavutil libswscale
 
# only export essential c-interfaces
h_sources = blts-pixelformat.h

c_sources = \
	blts-pixelformat.c

library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES= \
	libbltspixelformat.la

libbltspixelformat_la_LIBADD = -lm -lbltscommon libavcodec/libavcodec.la libavutil/libavutil.la libswscale/libswscale.la
libbltspixelformat_la_SOURCES= $(parser_sources) $(h_sources) $(local_h_sources) $(c_sources)
libbltspixelformat_la_LDFLAGS= -version-info $(BLTS_PIXELFORMAT_LIBRARY_VERSION) 
