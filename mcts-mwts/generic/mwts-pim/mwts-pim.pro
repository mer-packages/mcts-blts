TEMPLATE = subdirs
SUBDIRS = src min

VCARD.files = data/*.vcf
VCARD.path = /usr/lib/tests/mwts-pim
INSTALLS += VCARD

ICAL.files = data/*.ics
ICAL.path = /usr/lib/tests/mwts-pim
INSTALLS += ICAL

CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

