TEMPLATE = subdirs
SUBDIRS = src min

CONF.files = data/AccountsTest.conf
CONF.path = /usr/lib/tests

MIN_CONFIG.version = Versions
MIN_CONFIG.files = min/data/*.min.conf
MIN_CONFIG.path = /etc/min.d

MIN_SCRIPTS.files = min/data/*.cfg
MIN_SCRIPTS.path = /usr/lib/min

TESTS.files = min/data/tests.xml
TESTS.path = /usr/share/mwts-accounts-generic-tests

INSTALLS += MIN_CONFIG
INSTALLS += MIN_SCRIPTS
INSTALLS += TESTS
INSTALLS += CONF




