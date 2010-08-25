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
# Date Created: 2010/01/27
#
# Modifications:
#          Modificator  Date
#          Content of Modification
#


# the obex service is exposed in session dbus, so this will be a problem if the
# execution environment is in the root session. This script is used to execute the 
# test case in the a normal environment

set timeout 2400
set username [lindex $argv 0]
set casename [lindex $argv 1]
spawn su $username
expect "]$ "
send "./$casename"
send "\r"
expect "$ "
send "echo $?>result.log"
send "\r"
expect "$ "
send "exit"
send "\r"
expect "# "
