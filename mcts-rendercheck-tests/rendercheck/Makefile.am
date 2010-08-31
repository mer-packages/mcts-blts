SUBDIRS = man

bin_PROGRAMS = rendercheck

rendercheck_SOURCES = \
	main.c \
	ops.c \
	rendercheck.h \
	tests.c \
	t_blend.c \
	t_bug7366.c \
	t_composite.c \
	t_dstcoords.c \
	t_fill.c \
	t_gradient.c \
	t_repeat.c \
	t_srccoords.c \
	t_tsrccoords.c \
	t_tsrccoords2.c \
	t_triangles.c

AM_CFLAGS = $(RC_CFLAGS) $(CWARNFLAGS)
rendercheck_LDADD = $(RC_LIBS)

MAINTAINERCLEANFILES = ChangeLog INSTALL
EXTRA_DIST = \
	doc/AddingNewTests \
	doc/TODO \
        autogen.sh

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
