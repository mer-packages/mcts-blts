#!/usr/bin/expect
# Copyright (C) 2010, Intel Corporation.
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
#       Zhang Jingke  <jingke.zhang@intel.com>
# Date Created: 2010/01/13
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#

# delete file which owned by root

set timeout 2400
set root_pass [lindex $argv 0]
set filename [lindex $argv 1]
spawn su
expect "Password:"
send $root_pass
send "\r"
expect "# "
send "rm -rf $filename"
send "\r"
expect "# "
send "exit"
send "\r"
expect "]$ "
