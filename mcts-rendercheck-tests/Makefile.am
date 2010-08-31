defdir = $(datadir)/$(PACKAGE)
dist_def_DATA = tests.xml
docdir = /opt/$(PACKAGE)
dist_doc_DATA = README LICENSE

install-exec-hook:
	echo "run install"
	cd rendercheck; make install
#	mkdir -p $(DESTDIR)/opt/$(PACKAGE)/bin; cp -a piglit/bin $(DESTDIR)/opt/$(PACKAGE)
#	mkdir -p $(DESTDIR)/opt/$(PACKAGE)/tests; cp -a piglit/tests $(DESTDIR)/opt/$(PACKAGE)
