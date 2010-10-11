# Makefile -- Makefile for blts host driver
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

USER_CFLAGS := -O2 -g -Wall -Wextra -std=gnu99 `pkg-config --cflags bltscommon` -fno-builtin-sscanf
USER_CFLAGS += -I../common
USER_LIBS := -lrt -lpthread $(shell pkg-config --libs bltscommon)

ifeq (exists, $(shell test -d /usr/src/kernel-headers && echo exists))
KERNELDIR ?= /usr/src/kernel-headers
else ifeq (exists, $(shell test -d /lib/modules/`uname -r`/build && echo exists))
KERNELDIR ?= /lib/modules/`uname -r`/build
else
KERNELDIR ?= /lib/modules/`ls /lib/modules |awk '{print $1; exit}'`/build
endif

CHECKPATCH ?= $(KERNELDIR)/scripts/checkpatch.pl

all: module test_hostdrv

obj-m := blts_usb_host.o
blts_usb_host-objs := hostdrv.o ../driver-common/sysfs_log.o

module: hostdrv.c
	@[ -d $(KERNELDIR) ] || ( echo "Kernel module build environment not found" ; exit 1 )
	@echo "++  Checking kernel module"
	-$(CHECKPATCH) --no-tree --emacs --file hostdrv.c ../driver-common/sysfs_log.c
	@echo "++  Building kernel module"
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

module-sparse: hostdrv.c
	$(MAKE) -C $(KERNELDIR) C=2 M=`pwd` modules

test_hostdrv: test_hostdrv.c
	$(CC) -o test_hostdrv $(USER_CFLAGS) test_hostdrv.c $(USER_LIBS)

install: blts_usb_host.ko
	@echo "++  Installing into $(DESTDIR)"
	mkdir -p $(DESTDIR)/usr/lib/tests/blts-usb-tests
	cp blts_usb_host.ko $(DESTDIR)/usr/lib/tests/blts-usb-tests/

distclean: clean

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
	$(RM) Module.markers modules.order
	$(RM) test_hostdrv
	$(RM) tags TAGS

TAGS: hostdrv.c
	etags hostdrv.c

tags: hostdrv.c
	ctags hostdrv.c

.phony: all clean distclean install
