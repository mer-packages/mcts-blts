# Makefile.am -- Automake script
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

AM_CFLAGS = $(BLTS_FBDEV_CFLAGS) -Wall -Werror

requiredlibs = \
	$(BLTS_FBDEV_LIBS) \
	-lm -lrt

h_sources = \
	blts-fbdev-defs.h \
	blts-fbdev-utils.h \
	blts-fbdev-blanking.h \
	blts-fbdev-backlight.h \
	fbdev_fute.h

c_sources = \
	blts-fbdev-utils.c \
	blts-fbdev-blanking.c \
	blts-fbdev-backlight.c \
	fbdev_fute.c

bin_PROGRAMS = blts-fbdev-tests

blts_fbdev_tests_SOURCES = \
	$(h_sources) \
	$(c_sources) \
	blts-fbdev-cli.c

blts_fbdev_tests_LDFLAGS = \
	$(BLTS_FBDEV_LDFLAGS)

blts_fbdev_tests_LDADD = $(requiredlibs)
