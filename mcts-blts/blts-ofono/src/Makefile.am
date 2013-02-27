# Makefile.am -- Automake script for blts-ofono
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

AM_CFLAGS = -O3 -g -Werror -Wall -Wextra -std=gnu99 -I$(includedir) \
	$(PACKAGE_CFLAGS) -I.. -fPIC

requiredlibs = -lm -lrt

h_sources = sms-cases.h call-cases.h ofono-modem-bindings.h ofono-general.h \
	ofono-util.h call-forwarding.h sim-cases.h call-volume-cases.h \
	call-meter-cases.h call-barring.h dual-call-cases.h modem-cases.h \
	multiparty-call-cases.h ofono-call-bindings.h radio-settings.h \ 
	data-context.h call-settings.h

c_sources = blts-ofono.c sms-cases.c call-cases.c ofono-util.c \
	call-forwarding.c sim-cases.c call-volume-cases.c call-meter-cases.c \
	call-barring.c dual-call-cases.c modem-cases.c multiparty-call-cases.c \
	radio-settings.c data-context.c call-settings.c

generated_sources = signal-marshal.h signal-marshal.c

signal-marshal.h: signal-marshal.list
	glib-genmarshal --header signal-marshal.list --prefix=_blts_ $< > $@

signal-marshal.c: signal-marshal.list signal-marshal.h
	{ echo '#include "signal-marshal.h"' && \
	glib-genmarshal --body signal-marshal.list --prefix=_blts_ $< ; } > $@

# DBUS binding tool rules
org-ofono-Modem.h: $(top_builddir)/xml/org-ofono-Modem.xml
	$(DBUS_BINDING_TOOL) --mode=glib-client --prefix=org_ofono_Modem $< > $@

bin_PROGRAMS = blts-ofono-tests

# configdir is checked from configure
configdir = $(libdir)/cnf/
config_DATA = blts-ofono-tests.cnf

blts_ofono_tests_SOURCES = $(generated_sources) $(h_sources) $(c_sources)
blts_ofono_tests_CFLAGS = $(AM_CFLAGS)
blts_ofono_tests_LDADD = $(requiredlibs) $(PACKAGE_LIBS)

EXTRA_DIST = \
	signal-marshal.list \
	$(config_DATA)

BUILT_SOURCES = \
	$(generated_sources) \
	org-ofono-Modem.h

CLEANFILES = \
	$(BUILT_SOURCES)
