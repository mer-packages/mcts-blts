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

<<<<<<< HEAD
=======
TESTS.files = min/data/tests.xml
TESTS.path = /usr/share/mwts-accounts-generic-tests
INSTALLS +=TESTS
>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
