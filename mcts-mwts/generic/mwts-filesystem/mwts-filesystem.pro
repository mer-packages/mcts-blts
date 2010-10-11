TEMPLATE = subdirs
SUBDIRS = src min cli

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

