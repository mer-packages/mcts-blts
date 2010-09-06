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
manual_suspend_child = 1
excluding_devices=['1:1']

def check_usbdev_susp(x):
	print '\nfind usb device %s:%s %s at bus %s dev %s' % (vp[0], vp[1], x['name'], x['busnum'], x['devnum'])
	if x['child'] != []:
		if usb.check_for_hub(x, manual_suspend_child) != 0:
			return -1		
	if usb.prep_for_autosuspend(x) == 0:
		if usb.autosuspend_device(x) == 0 :	
			if usb.autoresume_device(x) == 0:
				return 0
		elif ignore_fail_autosuspend == True:
			return 0
	return -2

def is_excluding_devices(dev):
	for x in excluding_devices:
		if dev['busnum'] == x.split(":")[0] and dev['devnum'] == x.split(":")[1] :
			return 1
	return 0


if __name__ == '__main__':
	if len(sys.argv) < 2:
		print 'usage: %s device_file_name [True]' % sys.argv[0]
		exit(-1)

	global ignore_fail_autosuspend
	if len(sys.argv) == 3 and sys.argv[2] == 'True':
		ignore_fail_autosuspend = True
	else:
		ignore_fail_autosuspend = False
		
	usb.init_usb_devices()

	for idev in open(sys.argv[1]):
		vp = idev.strip().split(":")
		print vp
		if (len(vp) == 2):
			devs = usb.get_usbdev_by_id(vp[0], vp[1])		
			if  len(devs) != 0:
				for x in devs:
					if is_excluding_devices(x) == 0:
						result.append(check_usbdev_susp(x))
					else:
						print 'device at bus %s dev %s is excluded' %(x['busnum'], x['devnum'])
			else:
				print "can't find usbdevice %s:%s" % (vp[0], vp[1])
				#result.append(-3)
				
	usb.restore_device_status()

	for x in result:
		print x
		if x != 0:
			print 'error!!!'
			exit(1)	

	exit(0)
			
	


