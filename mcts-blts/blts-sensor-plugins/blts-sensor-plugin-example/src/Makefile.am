# Makefile.am -- Automake script for example sensor test plug-in
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

pkglibdir = $(libdir)/tests/blts-sensor-plugins

PLUGIN_VERSION = 1:0:0

# not needed in distribution
c_sources = \
	sensor-plugin-example.c

pkglib_LTLIBRARIES= libbltssensor_example.la

libbltssensor_example_la_LIBADD = -lbltscommon
libbltssensor_example_la_SOURCES = $(parser_sources) $(h_sources) $(local_h_sources) $(c_sources)
libbltssensor_example_la_LDFLAGS = -avoid-version -module
