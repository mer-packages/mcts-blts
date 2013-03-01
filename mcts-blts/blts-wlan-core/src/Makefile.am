# Makefile.am -- Makefile for blts-wlan-core
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
	-I$(top_srcdir) -DENABLE_DEBUG \
	$(BLTSCOMMON_CFLAGS) $(NL_CFLAGS) -pthread 

bin_PROGRAMS = blts-wlan-core-tests

h_sources = \
	ieee802_11_defs.h \
	nl80211_copy.h \
	radiotap.h \
	radiotap_iter.h \
	wlan-core-adhoc.h \
	wlan-core-channel.h \
	wlan-core-cmdqueue.h \
	wlan-core-connect.h \
	wlan-core-crypto.h \
	wlan-core-debug.h \
	wlan-core-definitions.h \
	wlan-core-dhcp.h \
	wlan-core-eapol.h \
	wlan-core-eloop.h \
	wlan-core-enumerate.h \
	wlan-core-ioctl.h \
	wlan-core-monitor.h \
	wlan-core-profiler.h \
	wlan-core-scan.h \
	wlan-core-utils.h \
	wlan-core-wpa.h

c_sources = \
	wlan-core-enumerate.c \
	wlan-core-utils.c \
	wlan-core-eloop.c \
	wlan-core-scan.c \
	wlan-core-debug.c \
	wlan-core-connect.c \
	wlan-core-channel.c \
	wlan-core-dhcp.c \
	wlan-core-profiler.c \
	wlan-core-ioctl.c \
	wlan-core-cli.c \
	wlan-core-wpa.c \
	wlan-core-eapol.c \
	wlan-core-monitor.c \
	wlan-core-adhoc.c \
	wlan-core-cmdqueue.c \
	radiotap.c \
	wlan-core-crypto.c

blts_wlan_core_tests_SOURCES = $(h_sources) $(c_sources)

blts_wlan_core_tests_LDADD = -lm -lrt $(NL_LIBS) $(BLTSCOMMON_LIBS)
blts_wlan_core_tests_LDFLAGS = $(BLTSCOMMON_LDFLAGS) -pthread
