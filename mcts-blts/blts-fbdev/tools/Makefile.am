##
# Copyright (C) 2013 Jolla Ltd.
# Contact: Matti Kosola <matti.kosola@jollamobile.com>
# License: GPLv2
##

AM_CFLAGS = $(GLIB_CFLAGS) -Wall -Werror

bin_PROGRAMS = \
	brightness-tool

brightness_tool_SOURCES = \
	brightness_tool.c

brightness_tool_LDFLAGS = \
	$(BLTS_FBDEV_LDFLAGS)

brightness_tool_LDADD = \
	$(GLIB_LIBS)

tooldir = $(bindir)
tool_SCRIPTS = \
	guess_settings.sh

EXTRA_DIST = \
	$(tool_SCRIPTS)
