#  Copyright (C) 2010 Intel Corporation
#  
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#  Makefile.am - Automake script for blts-opengles2-perf
#  For MeeGo mcts OpenVG testing
#  Author: 
#      Zheng Kui (kui.zheng@intel.com)



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


