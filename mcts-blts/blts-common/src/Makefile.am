# Makefile.am -- Automake script for blts-common
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
	-O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -I. \
	-DBLTS_CONFIG_DIR=\"$(configdir)\" $(XML_CFLAGS)
AM_YFLAGS = -d

# only export essential c-interfaces
h_sources = \
	blts_log.h \
	blts_timing.h \
	blts_dep_check.h \
	blts_cli_frontend.h \
	blts_params.h

# not needed in distribution
local_h_sources = \
	blts_params_local.h \
	blts_results_xml.h

c_sources = \
	blts_log.c \
	blts_timing.c \
	blts_dep_check.c \
	blts_cli_frontend.c \
	blts_params.c \
	blts_results_xml.c


BUILT_SOURCES = param-parser.h param-parser.c param-scanner.c
parser_sources = param-scanner.l param-parser.y

library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES= \
	libbltscommon.la

libbltscommon_la_LIBADD = -lrt -ldl $(XML_LIBS)
libbltscommon_la_SOURCES= $(parser_sources) $(h_sources) $(local_h_sources) $(c_sources)
libbltscommon_la_LDFLAGS= -version-info $(BLTS_COMMON_LIBRARY_VERSION)
