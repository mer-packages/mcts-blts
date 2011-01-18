TEMPLATE = subdirs
SUBDIRS = src min

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

TESTS.files = scripts/tests.xml
TESTS.path = /usr/share/mwts-gstreamer-tests
INSTALLS += TESTS
