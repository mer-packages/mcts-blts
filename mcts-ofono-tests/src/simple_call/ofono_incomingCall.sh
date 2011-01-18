#!/bin/sh
# Copyright (C) 2008-2010, Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.  
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307 USA.
#
# Authors:
#       Li,Zhigang  <zhigang.li@intel.com>
#
#
#!/bin/sh

baseDir=$(cd "$(dirname "$0")";pwd)
echo $baseDir


dbus-send --session --print-reply --dest=org.ofono.phonesim / org.ofono.phonesim.Script.SetPath string:${baseDir}

dbus-send --session --print-reply --dest=org.ofono.phonesim / org.ofono.phonesim.Script.Run string:call.js




if [ $? -ne 0 ]; then
	echo "error in hangup call"
	exit 1
else
	exit 0
fi
