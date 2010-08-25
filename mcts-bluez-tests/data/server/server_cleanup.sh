#!/bin/sh
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
#       Wan Shuang  <shuang.wan@intel.com>
# Date Created: 2010/01/18
#
# Modifications:
#          Modificator  Date
#          Content of Modification

agent_pid=`ps aux | grep auto_accept_agent | grep -v grep | awk '{print $2}'`

if [ -n "$agent_pid" ]; then
    kill -KILL $agent_pid
    echo "Pairing agent released"
fi

rm server_tmp -rf
echo "Temporary directory server_tmp for file transfer test has been removed."

