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
AC_INIT(blts-wlan-core-tests, 0.1.5, jussi.saavalainen@digia.com)
AM_INIT_AUTOMAKE([-Wall gnu])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/wlan-core-definitions.h])
AC_CONFIG_HEADER([config.h])

PKG_CHECK_MODULES([BLTSCOMMON],[bltscommon])

configdir=`pkg-config bltscommon --variable=configdir`

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([m], [acosf])
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([nl], [nl_handle_alloc])

AC_SUBST(BLTSCOMMON_CFLAGS)
AC_SUBST(BLTSCOMMON_LDFLAGS)
AC_SUBST(BLTSCOMMON_LIBS)

AC_SUBST(configdir)

AC_OUTPUT([
	Makefile \
	src/Makefile \
	data/Makefile \
	])
