# Makefile.am - Automake script for mcts-display-tests
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
#
#  Zheng Kui <kui.zheng@intel.com>

AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -fPIC

requiredlibs = -lm -lrt

h_sources = \
	display_common.h \
	display_helper.h

c_sources = \
	display_helper.c \
	display_enumerate_tests.c \
	display_rotate_tests.c \
	crtc_position_tests.c \
	crtc_modes_tests.c \
	crtc_scale_tests.c \
	display_backlight_tests.c \
	display_hotplug_tests.c 


bin_PROGRAMS =	mcts-display-tests

mcts_display_tests_SOURCES = $(h_sources) $(c_sources) mcts_display_tests.c
mcts_display_tests_CFLAGS = $(AM_CFLAGS) $(PACKAGE_CFLAGS)
mcts_display_tests_LDADD = $(requiredlibs) $(PACKAGE_LIBS)

