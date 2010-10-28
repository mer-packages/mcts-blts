TEMPLATE = subdirs
SUBDIRS = src min

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

