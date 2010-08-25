#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2010 Intel Corporation.
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
#       Jeff Zheng <jeff.zheng@intel.com>
#
# Description: common code to check the value of a property 
#

# The control server that connects to AP, so that test script can
# set AP parameters through apset script
cm_apset_server = "cm-srv.sh.intel.com"

# The full path of the apset script in control server
cm_apset_server_path= "/usr/local/bin/apset/cisco/apset"

# The essid of AP, this must be same as in apset script
cm_apset_ap_essid = "shz13-otc-cisco-cm"

