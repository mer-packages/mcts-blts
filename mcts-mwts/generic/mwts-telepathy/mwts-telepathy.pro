TEMPLATE = subdirs
SUBDIRS = src min doc cli

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

CLIENT.files = data/*.client
CLIENT.path = /usr/share/telepathy/clients/
INSTALLS += CLIENT

MMSMESSAGES.files = data/mms/*.mms
MMSMESSAGES.path = /usr/lib/tests
INSTALLS += MMSMESSAGES