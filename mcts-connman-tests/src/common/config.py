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
cm_apset_server = "192.168.2.201"

# The full path of the apset script in control server
cm_apset_server_path= "/usr/local/bin/apset/cisco/apset"

# The essid of AP, this must be same as in apset script
cm_apset_ap_essid = "shz13-otc-cisco-cm"


# For data transfer testing, we need a peer machine for upload/download
# Since different test environment has different upload/download method
# this test suite just defines configurable varibles for each technology
# Tester should re-configure these varibles, otherwise the result is wrong
#cm_bt_download = 'scp 192.168.1.2:/usr/share/mcts-connman-tests/10M /tmp'
#cm_bt_upload = 'scp /usr/share/mcts-connman-tests/10M 192.168.1.2:/tmp'
cm_bt_download = 'scp 127.0.0.1:/usr/share/mcts-connman-tests/10M /tmp'
cm_bt_upload = 'scp /usr/share/mcts-connman-tests/10M 127.0.0.1:/tmp'

cm_eth_download = 'scp 192.168.2.160:/usr/share/mcts-connman-tests/10M /tmp'
cm_eth_upload = 'scp /usr/share/mcts-connman-tests/10M 192.168.2.160:/tmp'
# For 3G, We will directly connect to Internet. So we can download
# from Internet. But tester need find a way to upload to Internet so 
# that upload test case works
# Download from Internet, ChangeLog-2.6.33 is about 7.5MB
cm_3g_download = 'wget http://www.kernel.org/pub/linux/kernel/v2.6/ChangeLog-2.6.33'
# Upload to local, please revise accordingly so that to upload
# to Internet automatically
cm_3g_upload = 'echo put /usr/share/mcts-connman-tests/10M /tmp/10M |sftp 127.0.0.1'

# For WiFi, we test data transfer for WEP and PSK. These essids are only used
# for data transfer. Test cases in test set WiFiFeature still need a cisco AP
cm_ap_essid_wep64 = 'shz13-otc-netgear-cm' # AP set to shared hidden wep64
cm_ap_wep64_key = '1111000000'
cm_ap_essid_rsn = 'shz13-otc-netgear2-cm' # AP set to open broadcast RSN PSK
cm_ap_rsn_key = 'sharedsecret'
cm_wifi_download = 'scp 192.168.1.6:/usr/share/mcts-connman-tests/10M /tmp'
cm_wifi_upload = 'scp /usr/share/mcts-connman-tests/10M 192.168.1.6:/tmp'





