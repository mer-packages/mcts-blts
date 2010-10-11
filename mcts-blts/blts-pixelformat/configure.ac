#                                               -*- Autoconf -*-

# configure.ac -- Autoconf script for blts-pixelformat
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
AC_INIT([blts-pixelformat], [0.1.2], [pasi.matilainen@digia.com])
AC_CONFIG_SRCDIR([src/libswscale/swscale.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
AM_MAINTAINER_MODE

BLTS_PIXELFORMAT_LIBRARY_NAME=bltspixelformat

#release versioning
BLTS_PIXELFORMAT_MAJOR_VERSION=0
BLTS_PIXELFORMAT_MINOR_VERSION=1
BLTS_PIXELFORMAT_MICRO_VERSION=1

BLTS_PIXELFORMAT_LIBRARY_VERSION=1:0:0

AC_SUBST(BLTS_PIXELFORMAT_LIBRARY_VERSION)

PACKAGE=$BLTS_PIXELFORMAT_LIBRARY_NAME
AC_SUBST(BLTS_PIXELFORMAT_LIBRARY_NAME)

BLTS_PIXELFORMAT_VERSION=$BLTS_PIXELFORMAT_MAJOR_VERSION.$BLTS_PIXELFORMAT_MINOR_VERSION.$BLTS_PIXELFORMAT_MICRO_VERSION
BLTS_PIXELFORMAT_RELEASE=$BLTS_PIXELFORMAT_MAJOR_VERSION.$BLTS_PIXELFORMAT_MINOR_VERSION
AC_SUBST(BLTS_PIXELFORMAT_RELEASE)
AC_SUBST(BLTS_PIXELFORMAT_VERSION)

VERSION=$BLTS_PIXELFORMAT_VERSION

PKG_CHECK_MODULES([BLTSCOMMON],[bltscommon])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([m], [acosf])

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

# Checks for library functions.
# libavutil uses these
AC_CHECK_FUNCS_ONCE(llrint lrint lrintf round roundf truncf)

AC_SUBST(BLTSCOMMON_CFLAGS)
AC_SUBST(BLTSCOMMON_LDFLAGS)
AC_SUBST(BLTSCOMMON_LIBS)

AC_CONFIG_FILES([     \
	Makefile      \
	src/Makefile  \
	src/libavcodec/Makefile \
	src/libavutil/Makefile \
	src/libswscale/Makefile \
	bltspixelformat.pc \
	])
AC_OUTPUT

