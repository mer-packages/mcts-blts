#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([rendercheck],
        1.3,
        [anholt@FreeBSD.org],
        rendercheck)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)
XORG_DEFAULT_OPTIONS

AC_PROG_CC
AC_PROG_SED


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([err.h])

AC_ARG_ENABLE(werror,        AS_HELP_STRING([--enable-werror],
				  [Treat warnings as errors (default: disabled)]),
			        [WERROR=$enableval], [WERROR=no])

# Checks for pkg-config packages
PKG_CHECK_MODULES(RC, xrender,
		  [xrender_found_with_pkgconfig=yes],
		  [xrender_found_with_pkgconfig=no])

if test "$xrender_found_with_pkgconfig" = "no"
then
	AC_PATH_XTRA
	if test "x$no_x" = "xyes"
	then
		AC_MSG_ERROR([X is required, but it was either disabled or not found.])
	fi

	RC_CFLAGS="$X_CFLAGS"
	RC_LIBS="$X_LIBS -lXrender -lXext -lX11"
fi

if test "x$GCC" = "xyes" -a "x$WERROR" = "xyes"; then
	RC_CFLAGS="$RC_CFLAGS -Werror"
fi
AC_SUBST(RC_CFLAGS)
AC_SUBST(RC_LIBS)

AC_CONFIG_FILES([Makefile
                 man/Makefile])

AC_OUTPUT
