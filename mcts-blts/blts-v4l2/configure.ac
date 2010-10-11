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

AC_PREREQ(2.59)
AC_INIT(blts-v4l2-tests, 0.3.4, mikko.ojala@digia.com)
AM_INIT_AUTOMAKE(blts-v4l2-tests, 0.3.3)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/camera.c])
AC_CONFIG_HEADER([config.h])

V4L2_LIBRARY_NAME=libmin-v4l2

#release versioning
V4L2_MAJOR_VERSION=0
V4L2_MINOR_VERSION=1
V4L2_MICRO_VERSION=0

V4L2_LIBRARY_VERSION=0:1:0

AC_SUBST(V4L2_LIBRARY_VERSION)

PACKAGE=$V4L2_LIBRARY_NAME
AC_SUBST(V4L2_LIBRARY_NAME)

V4L2_VERSION=$V4L2_MAJOR_VERSION.$V4L2_MINOR_VERSION.$V4L2_MICRO_VERSION
V4L2_RELEASE=$V4L2_MINOR_VERSION
AC_SUBST(V4L2_RELEASE)
AC_SUBST(V4L2_VERSION)

VERSION=$V4L2_VERSION

PKG_CHECK_MODULES([BLTSCOMMON],[bltscommon])
PKG_CHECK_MODULES([BLTSPIXELFORMAT],[bltspixelformat])
PKG_CHECK_MODULES([XVIDEO],[xv])

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
#AC_CHECK_LIB([minevent], [min_event_create])
#AC_CHECK_LIB([mintmapi], [tm_printf])
#AC_CHECK_LIB([minutils], [mip_get_uint])

# --- PKG_CHECK_MODULES does this already ---
# Library configs
# BLTS_COMMON_CFLAGS=`pkg-config --cflags bltscommon`
# BLTS_COMMON_LIBS=`pkg-config --libs bltscommon`

AC_SUBST(BLTSPIXELFORMAT_CFLAGS)
AC_SUBST(BLTSPIXELFORMAT_LDFLAGS)
AC_SUBST(BLTSPIXELFORMAT_LIBS)

AC_SUBST(BLTSCOMMON_CFLAGS)
AC_SUBST(BLTSCOMMON_LDFLAGS)
AC_SUBST(BLTSCOMMON_LIBS)

AC_SUBST(XVIDEO_CFLAGS)
AC_SUBST(XVIDEO_LDFLAGS)
AC_SUBST(XVIDEO_LIBS)

#AM_CONDITIONAL([WANT_MIN], [test "x$ac_cv_lib_mintmapi_tm_printf" == "xyes"])


AC_OUTPUT([Makefile \
	src/Makefile \
	src/min/Makefile\
	data/Makefile])
