TEMPLATE = subdirs
SUBDIRS = src min cli test
 
CONF.files = data/*.conf
CONF.path = /usr/lib/tests
INSTALLS += CONF

