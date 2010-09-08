#!/usr/bin/python
#
# Copyright (C) 2009 Intel Corporation.
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
#
# Authors:
#       Gong, Zhipeng <zhipeng.gong@intel.com>


import sys
import usb


result = []

if __name__ == '__main__':
	if len(sys.argv) != 2:
		print 'usage: %s device_file_name' % sys.argv[0]
		exit(-1)

	usb.init_usb_devices()
	
	for idev in open(sys.argv[1]):
		vp = idev.strip().split(":")
		print vp
		if (len(vp) == 2):
			devs = usb.get_usbdev_by_id(vp[0], vp[1])		
			if  len(devs) != 0:
				for x in devs:
					result.append(usb.check_autosuspend_is_enabled(x))
			else:
				print "can't find usbdevice %s:%s" % (vp[0], vp[1])
				#result.append(-3)

	for x in result:
		print x
		if x != 0:
			print 'error!!!'
			exit(1)	

	exit(0)
			
	


