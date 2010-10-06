#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for blts-bluetooth
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

AC_PREREQ([2.61])
AC_INIT([blts-bluetooth-tests], [0.2.8], [jussi.saavalainen@digia.com])
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/hci.c])
AC_CONFIG_HEADER([config.h])

# Library configs
PKG_CHECK_MODULES(BLTS_COMMON, [
	bltscommon
])

configdir=`pkg-config bltscommon --variable=configdir`

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

PKG_CHECK_MODULES([PACKAGE],[bluez bltscommon])

AM_CONDITIONAL([WANT_MIN], [test "x$ac_cv_lib_mintmapi_tm_printf" == "xyes"])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memset strdup strerror strtol])

AC_SUBST(configdir)

AC_CONFIG_FILES([     \
	Makefile      \
	src/Makefile  \
	data/Makefile\
	])
AC_OUTPUT
