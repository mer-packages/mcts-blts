TEMPLATE = subdirs
SUBDIRS = src min

CONF.files = data/*.conf
CONF.path = /usr/lib/tests

DATA.files = data/*
DATA.path = /usr/lib/tests/mwts-network

INSTALLS += CONF
INSTALLS += DATA

