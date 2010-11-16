#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script
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
AC_INIT([blts-fbdev-tests], [0.0.7], [mika.niinisto@digia.com])
AM_INIT_AUTOMAKE([gnu])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

# Checks for libraries.
AC_CHECK_LIB([m], [acosf])
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([minevent], [min_event_create])
AC_CHECK_LIB([mintmapi], [tm_printf])
AC_CHECK_LIB([minutils], [mip_get_uint])

AM_CONDITIONAL([WANT_MIN], [test "x$ac_cv_lib_mintmapi_tm_printf" == "xyes"])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h stdlib.h string.h sys/ioctl.h sys/time.h unistd.h])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memset munmap strdup strerror strtol])

# Checks for common, needed packages
PKG_CHECK_MODULES(BLTS_FBDEV, [
        bltscommon
])

configdir=`pkg-config bltscommon --variable=configdir`

AC_SUBST(BLTS_FBDEV_CFLAGS)
AC_SUBST(BLTS_FBDEV_LDFLAGS)
AC_SUBST(BLTS_FBDEV_LIBS)

AC_SUBST(configdir)

AC_OUTPUT([
	Makefile
	src/Makefile
	min-scripts/Makefile
	data/Makefile
])
