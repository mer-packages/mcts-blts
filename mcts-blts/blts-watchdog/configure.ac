#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for blts-watchdog
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
AC_INIT(blts-watchdog-tests, 0.0.10, jussi.saavalainen@digia.com)
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/blts_watchdog_test.c])
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

# Library configs
PKG_CHECK_MODULES(BLTS_COMMON, [
        bltscommon
])

AC_SUBST(BLTS_COMMON_CFLAGS)
AC_SUBST(BLTS_COMMON_LIBS)

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([	\
	Makefile	\
	src/Makefile	\
	data/Makefile	\
	tools/Makefile	\
	])
AC_OUTPUT

