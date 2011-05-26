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
AC_INIT([blts-bluetooth-tests], [0.2.16], [jussi.saavalainen@digia.com])
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/hci.c])
AC_CONFIG_HEADER([config.h])

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

PKG_CHECK_MODULES([PACKAGE],[bluez bltscommon])

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

# Check for BTLE API support in Bluez
AC_SEARCH_LIBS([hci_le_create_conn], [bluetooth], [AC_DEFINE([HAVE_BTLE_API], [1], [Defined if BTLE API available])],[],[])

# Check if LE scan functions take a timeout parameter or not (post-4.89 Bluez
# feature). Note that this will also pass with an implicit declaration warning
# if LE is not available.
AC_LANG_ASSERT([C])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
		],
		[hci_le_set_scan_enable(0,0,0,0);])],
	[AC_DEFINE([LE_SCAN_HAS_TO], [1], [Defined if BTLE scan takes a timeout parameter])], [])

AC_SUBST(configdir)

AC_CONFIG_FILES([     \
	Makefile      \
	src/Makefile  \
	data/Makefile\
	])
AC_OUTPUT
