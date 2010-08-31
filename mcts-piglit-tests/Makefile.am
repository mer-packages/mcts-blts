defdir = $(datadir)/$(PACKAGE)
dist_def_DATA = tests-general.xml tests-texture.xml tests-fbo.xml tests-asm-ARBvp1.0.xml tests-asm-ARBfp1.0.xml tests-glslparsertest.xml tests-shaders.xml tests-glx.xml tests-bugs.xml tests.xml
docdir = /opt/$(PACKAGE)
dist_doc_DATA = README LICENSE

install-exec-hook:
	cd piglit; cmake .;make
	mkdir -p $(DESTDIR)/opt/$(PACKAGE)/bin; cp -a piglit/bin $(DESTDIR)/opt/$(PACKAGE)
	mkdir -p $(DESTDIR)/opt/$(PACKAGE)/tests; cp -a piglit/tests $(DESTDIR)/opt/$(PACKAGE)
