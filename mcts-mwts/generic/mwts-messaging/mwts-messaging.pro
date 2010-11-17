TEMPLATE = subdirs
SUBDIRS = src min cli

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

DATA.files = data/*.txt
DATA.path = /usr/lib/tests
INSTALLS += DATA