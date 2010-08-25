libexec_PROGRAMS = geoclue-test

noinst_DATA = \
	geoclue-test.xml

nodist_geoclue_test_SOURCES = \
	geoclue-test-glue.h

BUILT_SOURCES = \
	$(nodist_geoclue_test_SOURCES)

geoclue_test_SOURCES = 	\
	geoclue-test.c     

geoclue_test_CFLAGS = \
	-I$(top_srcdir)     \
	-I$(top_builddir)   \
	$(GEOCLUE_CFLAGS)

geoclue_test_LDADD = \
	$(GEOCLUE_LIBS)    


providersdir = $(datadir)/geoclue-providers
providers_DATA = geoclue-test.provider

servicedir = $(datadir)/dbus-1/services
service_in_files = org.freedesktop.Geoclue.Providers.Test.service.in
service_DATA = $(service_in_files:.service.in=.service)

$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

CLEANFILES = \
	stamp-geoclue-test-glue.h

EXTRA_DIST = 			\
	$(service_in_files)	\
	$(providers_DATA)	\
	$(noinst_DATA)

DISTCLEANFILES = 	\
	$(service_DATA)	\
	$(nodist_geoclue_test_SOURCES)

geoclue-test-glue.h: stamp-geoclue-test-glue.h
	@true

stamp-geoclue-test-glue.h: geoclue-test.xml
	$(DBUS_BINDING_TOOL) --prefix=geoclue_test --mode=glib-server $< > xgen-$(@F) \
	&& (cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%)) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)
