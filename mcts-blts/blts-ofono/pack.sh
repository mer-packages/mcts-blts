#!/bin/sh
#
# Packaging script for a BLTS test case
#

# Using autoconf and automake to package the sources
if [ -x ./autogen.sh ]; then
    ./autogen.sh
    ./configure
else
    autoreconf --force -i -s
    ./configure
fi

if [ -f Makefile ]; then
    make dist
else
    echo "No Makefile! Cannot make a source distribution!"
    exit -1
fi

# Determine the destination dir
dest=""

if [ "x$RPM_BUILD_DIR" = "x" ]; then
    dest="$HOME/rpmbuild/SOURCES"
else
    dest="$RPM_BUILD_DIR/SOURCES"
fi

if [ ! -d $dest ]; then
    mkdir -p $dest
fi

# Then, copy all packaged sources to build dir
cp *.tar.gz $dest

# Then, build the stuff
rpmbuild -ba $* *.spec
