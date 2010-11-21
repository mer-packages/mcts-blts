#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for blts-opengles2-perf
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


# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(blts-openvg-tests, 0.0.11, kui.zheng@intel.com)
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/blts_openvg_test.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([m], [acosf])
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([minevent], [min_event_create])
AC_CHECK_LIB([mintmapi], [tm_printf])
AC_CHECK_LIB([minutils], [mip_get_uint])
#AC_CHECK_LIB([GLESv2], [glClear],,AC_MSG_ERROR(cannot find GLESv2 libraries))
#AC_CHECK_LIB([EGL], [eglGetError],,AC_MSG_ERROR(cannot find EGL libraries))
AC_CHECK_LIB([bltscommon], [log_open],,[AC_MSG_ERROR([cannot find libbltscommon])])

AM_CONDITIONAL([WANT_MIN], [test "x$ac_cv_lib_mintmapi_tm_printf" == "xyes"])
AM_CONDITIONAL([LINK_GLSLC], [test "x$ac_cv_build" == "xarm-unknown-linux-gnueabi"])

# Library configs
BLTS_COMMON_CFLAGS=`pkg-config --cflags bltscommon`
BLTS_COMMON_LIBS=`pkg-config --libs bltscommon`

AC_SUBST(BLTS_COMMON_CFLAGS)
AC_SUBST(BLTS_COMMON_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER([EGL/egl.h],,AC_MSG_ERROR([cannot find egl.h]))
AC_CHECK_HEADER([VG/openvg.h],,AC_MSG_ERROR([cannot find openvg.h]))

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([     \
	Makefile      \
	src/Makefile  \
	])
AC_OUTPUT

