# Makefile.am -- Makefile for blts-V4L2
#
# Copyright (c) 2009-2010 Nokia Corporation.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AM_CPPFLAGS = \
	-I$(top_srcdir) -DENABLE_DEBUG $(BLTSCOMMON_CFLAGS) $(BLTSPIXELFORMAT_CFLAGS) $(XVIDEO_CFLAGS)

bin_PROGRAMS = blts-v4l2-tests

blts_v4l2_tests_SOURCES = \
	blts-v4l2-definitions.h \
	v4l2-ioctl.h v4l2-ioctl.c \
	v4l2-profiler.h v4l2-profiler.c \
	v4l2-cli.c \
	camera.h camera.c \
	blts-bayer.h blts-bayer.c \
	v4l2-xvideo.h v4l2-xvideo.c \
	v4l2-controls.h v4l2-controls.c \
	v4l2-priority.h v4l2-priority.c \
	v4l2-stream-quality.h v4l2-stream-quality.c

blts_v4l2_tests_LDADD = -lm -lrt -ljpeg -lX11 $(BLTSCOMMON_LIBS) $(BLTSPIXELFORMAT_LIBS) $(XVIDEO_LIBS)
blts_v4l2_tests_LDFLAGS = $(BLTSCOMMON_LDFLAGS) $(BLTSPIXELFORMAT_LDFLAGS) $(XVIDEO_LDFLAGS)
