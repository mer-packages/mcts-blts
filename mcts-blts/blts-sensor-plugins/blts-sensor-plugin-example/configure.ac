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
AC_INIT(blts-sensor-plugin-example, 0.1.3, pasi.matilainen@digia.com)
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/sensor-plugin-example.c])
AC_CONFIG_HEADER([config.h])

PLUGIN_LIBRARY_NAME=blts-sensor-plugins

PKG_CHECK_MODULES([BLTSCOMMON],[bltscommon])

configdir=`pkg-config bltscommon --variable=configdir`

AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_SUBST(BLTSCOMMON_CFLAGS)
AC_SUBST(BLTSCOMMON_LDFLAGS)
AC_SUBST(BLTSCOMMON_LIBS)
AC_SUBST(configdir)

AC_CONFIG_FILES([Makefile
	src/Makefile
	data/Makefile])

AC_OUTPUT
