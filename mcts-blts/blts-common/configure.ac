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
AC_INIT([blts-common],[0.4.1],[jussi.saavalainen@digia.com])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/blts_log.c])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

BLTS_COMMON_LIBRARY_VERSION=1:0:0
AC_SUBST(BLTS_COMMON_LIBRARY_VERSION)

# Configuration dir
# For some reason sysconfdir contains $prefix even if it's set to /usr
# On the other hand, this is kind of evil, --sysconfdir= configure option
# is ignored if --prefix=/usr
if test "$prefix" = "/usr" || "$prefix" = "/usr/"; then
        configdir="/etc/blts"
else
        configdir="${sysconfdir}/blts"
fi
AC_SUBST(configdir)

PKG_CHECK_MODULES(XML, libxml-2.0)
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

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
