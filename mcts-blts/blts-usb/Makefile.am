# Makefile.am - Automake script for blts-usb-tests
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

SUBDIRS = common driver-common host-driver peripheral-driver src
pkgdata_DATA = tests.xml

EXTRA_DIST = \
	autogen.sh \
	blts-usb-tests.changes \
	$(pkgdata_DATA) \
	host-driver/Makefile \
	host-driver/hostdrv.c \
	host-driver/test_hostdrv.c \
	peripheral-driver/Makefile \
	peripheral-driver/composite.c \
	peripheral-driver/composite.h \
	peripheral-driver/epautoconf.c \
	peripheral-driver/ep_config.c \
	peripheral-driver/ep_config.h \
	peripheral-driver/gadget_chips.h \
	peripheral-driver/gadgetdrv.c \
	peripheral-driver/gadgetdrv.h \
	peripheral-driver/test_gadgetdrv.c \
	peripheral-driver/usbstring.c
