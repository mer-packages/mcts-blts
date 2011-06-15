TEMPLATE = subdirs
SUBDIRS = src min tools #cli

CONF.files = config/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

MIN_CONFIG.version = Versions
MIN_CONFIG.files = scripts/*.min.conf
MIN_CONFIG.path = /etc/min.d
INSTALLS += MIN_CONFIG

MIN_SCRIPTS.files = scripts/*.cfg
MIN_SCRIPTS.path = /usr/lib/min
INSTALLS += MIN_SCRIPTS

ALLTESTS.files = scripts/tests.xml
ALLTESTS.path = /usr/share/mwts-bluetooth-generic-tests
INSTALLS += ALLTESTS
