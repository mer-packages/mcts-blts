AM_CFLAGS = -O3 -g -Wall -Wextra -std=gnu99 -I$(includedir) $(BLTS_COMMON_CFLAGS) -I.. -fPIC

requiredlibs = -lm -lrt

h_sources = usb-util.h usb-general.h hostdrv_common.h transfer-cases.h peripheral-control.h

c_sources = blts-usb-testrunner.c transfer-cases.c peripheral-control.c

CLEANFILES =

distclean-local:

bin_PROGRAMS = blts_usb_testrunner

pkgdata_DATA = \
	blts-usb-testrunner-bulk.cnf \
	blts-usb-testrunner-isoc.cnf \
	blts-usb-testrunner-int.cnf \
	ep-configuration-default.cfg \
	ep-configuration-bulk.cfg \
	ep-configuration-isoc.cfg \
	ep-configuration-int.cfg

blts_usb_testrunner_SOURCES = $(h_sources) $(c_sources)
blts_usb_testrunner_CFLAGS = $(AM_CFLAGS) $(BLTS_COMMON_CFLAGS) $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(GTHREAD_CFLAGS)
blts_usb_testrunner_LDADD = $(requiredlibs) $(BLTS_COMMON_LIBS) $(GLIB_LIBS) $(DBUS_LIBS) $(GTHREAD_LIBS)
