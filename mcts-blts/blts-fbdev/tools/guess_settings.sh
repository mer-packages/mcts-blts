#!/bin/sh

##
# Copyright (C) 2013 Jolla Ltd.
# Contact: Matti Kosola <matti.kosola@jollamobile.com>
# License: GPLv2
##

BRIGHTNESS_MAX=$(/opt/tests/blts-fbdev-tests/bin/brightness-tool | grep "max_brightness=" | cut -d '=' -f2)
SUBSYSTEM_PATH=$(/opt/tests/blts-fbdev-tests/bin/brightness-tool | grep "subsystem_path=" | cut -d '=' -f2)
ESCAPED_SUBSYSTEM_PATH=$(echo $SUBSYSTEM_PATH | sed -e 's/\//\\\//g')

echo "Setting display subsystem path: $SUBSYSTEM_PATH"
echo "Setting display brightness maximum value: $BRIGHTNESS_MAX"

cat /opt/tests/blts-fbdev-tests/cnf/blts-fbdev-tests.cnf | sed -e "s/const <enter_valid_max_level_here> *$/const $BRIGHTNESS_MAX/" > /tmp/blts-fbdev-tests.cnf.tmp1
cat /tmp/blts-fbdev-tests.cnf.tmp1 | sed -e "s/const \"\/sys\/class\/backlight\/<enter_valid_sysfs_entry_here>\" *$/const \"$ESCAPED_SUBSYSTEM_PATH\"/" > /tmp/blts-fbdev-tests.cnf.tmp2

/opt/tests/blts-fbdev-tests/bin/run-binary cp /tmp/blts-fbdev-tests.cnf.tmp2 /opt/tests/blts-fbdev-tests/cnf/blts-fbdev-tests.cnf

rm /tmp/blts-fbdev-tests.cnf.tmp1
rm /tmp/blts-fbdev-tests.cnf.tmp2
