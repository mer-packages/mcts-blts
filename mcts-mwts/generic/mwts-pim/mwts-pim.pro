TEMPLATE = subdirs
SUBDIRS = src min

VCARD.files = data/*.vcf
VCARD.path = /usr/lib/tests/mwts-pim
INSTALLS += VCARD

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

