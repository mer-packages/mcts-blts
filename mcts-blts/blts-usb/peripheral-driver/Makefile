# Makefile -- Makefile for blts gadget driver
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

USER_CFLAGS := -O2 -g -Wall -Wextra -std=gnu99 -fno-builtin-sscanf -I../common
USER_LIBS := -lpthread

ifeq (exists, $(shell test -d /usr/src/kernel-headers && echo exists))
KERNELDIR ?= /usr/src/kernel-headers
else ifeq (exists, $(shell test -d /lib/modules/`uname -r`/build && echo exists))
KERNELDIR ?= /lib/modules/`uname -r`/build
else
KERNELDIR ?= /lib/modules/`ls /lib/modules |awk '{print $1; exit}'`/build
endif

CHECKPATCH ?= $(KERNELDIR)/scripts/checkpatch.pl

all: module test_gadgetdrv

obj-m := blts_gadget.o
blts_gadget-objs := gadgetdrv.o ep_config.o usbstring.o composite.o epautoconf.o ../driver-common/sysfs_log.o

module: gadgetdrv.c
	@[ -d $(KERNELDIR) ] || ( echo "Kernel module build environment not found" ; exit 1 )
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

module-checkpatch: gadgetdrv.c
	-$(CHECKPATCH) --no-tree --emacs -f gadgetdrv.c ep_config.c composite.c ../driver-common/sysfs_log.c test_gadgetdrv.c


test_gadgetdrv: test_gadgetdrv.c
	$(CC) -o test_gadgetdrv $(USER_CFLAGS) test_gadgetdrv.c $(USER_LIBS)

install: blts_gadget.ko
	@echo "++  Installing into $(DESTDIR)"
	mkdir -p $(DESTDIR)/usr/lib/tests/blts-usb-tests
	cp blts_gadget.ko $(DESTDIR)/usr/lib/tests/blts-usb-tests/

distclean: clean

distdir:

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
	$(RM) Module.markers modules.order
	$(RM) test_gadgetdrv

.phony: all clean distclean install
