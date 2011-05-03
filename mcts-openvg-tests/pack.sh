#!/bin/sh

#  Copyright (C) 2010 Intel Corporation
#  
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#  Author: 
#      Zheng Kui (kui.zheng@intel.com)
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

