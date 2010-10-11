#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for blts-common
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

AC_PREREQ(2.61)
AC_INIT([blts-common],[0.3.7],[jussi.saavalainen@digia.com])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/blts_log.c])
AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
AM_MAINTAINER_MODE

BLTS_COMMON_LIBRARY_NAME=bltscommon

#release versioning
BLTS_COMMON_MAJOR_VERSION=0
BLTS_COMMON_MINOR_VERSION=3
BLTS_COMMON_MICRO_VERSION=5

BLTS_COMMON_LIBRARY_VERSION=1:0:0

AC_SUBST(BLTS_COMMON_LIBRARY_VERSION)

PACKAGE=$BLTS_COMMON_LIBRARY_NAME
AC_SUBST(BLTS_COMMON_LIBRARY_NAME)

BLTS_COMMON_VERSION=$BLTS_COMMON_MAJOR_VERSION.$BLTS_COMMON_MINOR_VERSION.$BLTS_COMMON_MICRO_VERSION
BLTS_COMMON_RELEASE=$BLTS_COMMON_MAJOR_VERSION.$BLTS_COMMON_MINOR_VERSION
AC_SUBST(BLTS_COMMON_RELEASE)
AC_SUBST(BLTS_COMMON_VERSION)

VERSION=$BLTS_COMMON_VERSION


AC_PROG_CC
AC_PROG_LIBTOOL

AC_PROG_YACC
AC_PROG_LEX

AM_SANITY_CHECK

AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_HEADERS([string.h])


AC_CONFIG_FILES([Makefile \
	src/Makefile \
	bltscommon.pc])
AC_OUTPUT
