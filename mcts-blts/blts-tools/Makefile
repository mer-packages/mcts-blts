##
# Copyright (C) 2013 Jolla Ltd.
# Contact: Matti Kosola <matti.kosola@jollamobile.com>
# License: GPLv2
##

CC = gcc
CFLAGS = -g -Wall -Werror
TARGET = run-blts-root
INSTALL = install -c
INSTALLDIR = install -d

# The directory to install tools
sbindir = /usr/sbin

.PHONY: all
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: install
install:
	$(INSTALLDIR) $(DESTDIR)$(sbindir)
	$(INSTALL) $(TARGET) $(DESTDIR)$(sbindir)/$(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET)
