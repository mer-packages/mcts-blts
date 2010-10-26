# Makefile.am -- Makefile for blts-sensors-frontend
#
# Copyright (c) 2009-2010 Nokia Corporation.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -I.

AM_CPPFLAGS = \
	-I$(top_srcdir) -DENABLE_DEBUG $(BLTSCOMMON_CFLAGS)

h_sources = blts_sensor_plugin.h \
	blts_sensor_variable.h

c_sources = sensors-cli.c

library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

bin_PROGRAMS = blts-sensors-frontend

blts_sensors_frontend_SOURCES = $(h_sources) $(c_sources)

blts_sensors_frontend_LDADD = $(BLTSCOMMON_LIBS)
blts_sensors_frontend_LDFLAGS = $(BLTSCOMMON_LDFLAGS)
