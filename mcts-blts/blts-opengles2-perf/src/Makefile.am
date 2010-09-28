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

AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) -I.. -fPIC

requiredlibs = -lm -lrt -lEGL -lGLESv2 -lX11

if WANT_MIN
min_libs = -lminutils -lmintmapi -lminevent
else
min_libs =
endif

h_sources = \
	ogles2_helper.h \
	test_common.h \
	test_blitter.h \
	ogles2_conf_file.h \
	compositor_runner.h

c_sources = \
	ogles2_helper.c \
	ogles2_conf_file.c \
	compositor_runner.c \
	test_simple_tri.c \
	test_enum_glextensions.c \
	test_enum_eglextensions.c \
	test_enum_eglconfigs.c \
	test_polygons.c \
	test_frag_shader.c \
	test_vert_shader.c \
	test_texels.c \
	test_fillrate.c \
	test_blitter.c

library_includedir = $(includedir)/blts
#library_include_HEADERS = $(h_sources)

if WANT_MIN

lib_LTLIBRARIES = libbltsopengles2perf.la
libbltsopengles2perf_la_SOURCES = $(h_sources) $(lib_h_sources) $(c_sources) $(lib_c_sources) min_scripter_mod.c
libbltsopengles2perf_la_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS) -fPIC
if LINK_GLSLC
libbltsopengles2perf_la_LIBADD = $(requiredlibs) -lglslcompiler $(BLTS_COMMON_LIBS)
else
libbltsopengles2perf_la_LIBADD = $(requiredlibs) $(BLTS_COMMON_LIBS)
endif
libbltsopengles2perf_la_LDFLAGS = $(AM_LDFLAGS) -shared -module

endif

bin_PROGRAMS = \
	blts-opengles2-tests \
	blts-minimal-compositor

blts_opengles2_tests_SOURCES = $(h_sources) $(c_sources) ogles2_perf_test.c
blts_opengles2_tests_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS)
blts_opengles2_tests_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS)

blts_minimal_compositor_SOURCES = ogles2_helper.c minimal_compositor.c
blts_minimal_compositor_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS)
blts_minimal_compositor_LDADD = $(requiredlibs) -lXfixes -lXcomposite -lXdamage $(BLTS_COMMON_LIBS)

