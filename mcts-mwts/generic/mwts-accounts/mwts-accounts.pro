TEMPLATE = subdirs
SUBDIRS = src min

CONF.files = data/AccountsTest.conf
CONF.path = /usr/lib/tests

MIN_CONFIG.version = Versions
MIN_CONFIG.files = min/data/*.min.conf
MIN_CONFIG.path = /etc/min.d

MIN_SCRIPTS.files = min/data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min

ALLTESTS.files = data/tests.xml
ALLTESTS.path = /usr/share/mwts-accounts-scripts


INSTALLS += MIN_CONFIG
INSTALLS += MIN_SCRIPTS
INSTALLS += CONF

