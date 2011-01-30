# Makefile.am - Automake script for blts-opengles2-perf
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
#  For MeeGo mcts OpenVG testing
#  Zheng Kui (kui.zheng@intel.com)


AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -fPIC

requiredlibs = -lm -lrt -lEGL -lX11 -lOpenVG

h_sources = \
	openvg_helper.h \
	test_common.h 

c_sources = \
	openvg_helper.c \
        test_enum_eglextensions.c \
        test_enum_eglconfigs.c \
	test_transf.c  \
	test_stroke.c  \
	test_fill.c \
	test_clear.c \
	test_lingrad.c \
	test_radialgrad.c 

library_includedir = $(includedir)/blts

bin_PROGRAMS = \
	blts-openvg-tests 

blts_openvg_tests_SOURCES = $(h_sources) $(c_sources) blts_openvg_test.c
blts_openvg_tests_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS)
blts_openvg_tests_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS)


