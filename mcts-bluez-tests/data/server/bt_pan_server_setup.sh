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
#       Guo, Zhaojuan  <zhaojuan.guo@@intel.com>
# Date Created: 2010/12/02
#
# Modifications:
#          Modificator  Date
#          Content of Modification

# DESC:build pan server envieroment

brctl addbr pan1

ifconfig pan1 192.168.0.1

echo 1 > /proc/sys/net/ipv4/ip_forward 

iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

iptables -A FORWARD -i pant1 -j ACCEPT

