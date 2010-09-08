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

import os
import commands
import time

autosuspend_time = 2
devices = []
hubs = {}
sysfs = "/sys/bus/usb/devices/"
manual_suspend_child = 1
files = ['idProduct', 'idVendor', 'manufacturer', 'product', 'busnum', 'devnum', 'power/autosuspend', 'power/level', 'power/wakeup']
def string_to_device(s):
	y={}
	y['busnum']=int(s[4:7])
	y['devnum']=int(s[15:18])
	y['vendor']=s[23:27]
	y['product']=s[28:32]
	y['name']=s[33:]
	y['child']=[]
	y['autosuspend'] = ''
	y['wakeup']=''
	y['level']=''
	return y


def get_usb_device(s):
	y={}
	y['manufacturer']=''
	y['product']=''
	for f in files:
		try:
			sysfile = open(sysfs + s + "/" + f, "r")
			y[f] = sysfile.readline().rstrip()
		except IOError:
			print "can't open files at %s/%s" % (s, f)
	y['syspath'] = sysfs + s
	y['name'] = y['manufacturer'] + ' ' + y['product']
	y['child'] = []
	y['usbfs'] = "/proc/bus/usb/%03d/%03d" % (int(y['busnum']), int(y['devnum']))
		
	return y

def find_sysfs_path(dev):
	for s in os.listdir(sysfs):
		try:
			f = open(sysfs + s + "/busnum", 'r')
			if int(f.readline().rstrip()) == dev['busnum']:
				f.close()
				f = open(sysfs + s + "/devnum", 'r')
				if int(f.readline().rstrip()) == dev['devnum']:
					print 'found bus %d dev %d at ' % (dev['busnum'], dev['devnum']) + sysfs + s
					dev['syspath'] = sysfs + s
					get_power_state(dev)
					return sysfs + s			
		except IOError:
			pass
		finally:
			f.close()
	return ''

def init_usb_devices():
	for s in os.listdir(sysfs):
		if s.find("usb") != -1 or s.find(":") == -1:
			y = get_usb_device(s)
			if y != None:
				devices.append(y)
	for x in devices:
		if x['devnum'] == '1':
			hubs[x['busnum']]= x
	for x in devices:
		if x['devnum'] != '1':
			hubs[x['busnum']]['child'].append(x)
			
def prep_for_autosuspend(dev):
	try:
		f = open(dev['syspath'] + "/power/autosuspend", "w")
		f.write(str(autosuspend_time))
		f.close()
		f = open(dev['syspath'] + "/power/wakeup", "w")
		if dev['power/wakeup'] != '':
			f.write('enabled')
		f.close()
		f = open(dev['syspath'] + "/power/level", "w")
		f.write('auto')
		f.close()
	except IOError:
		print "can't open power/autosuspend or power/level or power/wakeup"
		return -1
	return 0


def check_autosuspend(dev):
	try:
		f = open(dev['syspath'] + "/power/active_duration", 'r')
		old_active_duration = int (f.readline().rstrip())
		time.sleep(2)
		f.seek(0)
		new_active_duration = int (f.readline().rstrip())
		f.close()
		print 'old active duration: %d  new active duration:%d' % (old_active_duration, new_active_duration)		
		if new_active_duration == old_active_duration:
			return 0
		else:
			return -1
	except IOError:
		print "can't open power/active_duration"
	return -1


def autosuspend_device(dev):
	time.sleep(autosuspend_time + 1)	
	if check_autosuspend(dev) == 0:
		print 'succeed to autosuspend device'
		return 0
	else:
		print 'fail to autosuspend device'
		return -1


def autoresume_device(dev):
	f = open(dev['usbfs'], 'r')
	i = check_autosuspend(dev)
	f.close()
	if  i == -1:
		print 'succeed to autoresume device'
		return 0
	else:
		print 'fail to autoresume device'
		return -1

	
def manual_suspend_device(dev):
	try:
		f = open(dev['syspath'] + "/power/level", "w")
		f.write('suspend')
		f.close()
		if check_autosuspend(dev) == 0:
			print 'succeed to suspend device'
			return 0
		else:
			print 'fail to suspend device'
			return -1		
	except IOError:
		print "can't open  %s/power/level" % dev['syspath']
		return -1
	return 0


def manual_resume_device(dev):
	try:
		f = open(dev['syspath'] + "/power/level", "w")
		f.write('on')
		f.close()
		if check_autosuspend(dev) == -1:
			print 'succeed to resume device'
			return 0
		else:
			print 'fail to resume device'
			return -1		
	except IOError:
		print "can't open power/level"
		return -1
	return 0
			


def check_hub_with_child(dev):
	if dev['child'] == []:
		return -1
	else:
		for x in dev['child']:
			try:
				f = open(x['syspath'] + "/power/level", "w")
				f.write('on')
				f.close()
			except IOError:
				print "can't open power/autosuspend or power/level or power/wakeup"	
			finally:
				break;
				
		if prep_for_autosuspend(x) == 0 :
			time.sleep(autosuspend_time + 1)	
			if check_autosuspend(dev) == -1:
				print " the hub doesn't go to autosuspend state"
				return 0
			else:
				print 'error!!! the hub go to autosuspend state'
				return -1
		
		return -1		
			

def check_for_hub(dev, manual_suspend):
	for x in dev['child']:
		if manual_suspend == 1:
			if manual_suspend_device(x) != 0:
				return -1
		else:
			if prep_for_autosuspend(x) != 0 or autosuspend_device(x) != 0:
				return -1
	return 0

def check_autosuspend_is_enabled(dev):
	if int(dev['power/autosuspend']) < 0:
		print 'error!!! power/autosuspend is not greater than/equal 0'
		return -1
	if dev['power/wakeup'] == 'disabled' :
		print 'error!!! power/wake is not empty or enabled'
		return -2
	if dev['power/level'] != 'auto':
		print 'error!!! power/level is not "auto"'
		return -3
	return 0


def manual_suspend_while_working(dev):
	f = open(dev['usbfs'], 'r')
	rc = manual_suspend_device(dev)	
	f.close()
	return rc;	

def auto_suspend_while_working(dev):
	f = open(dev['usbfs'], 'r')
	rc = -1
	if prep_for_autosuspend(dev) == 0 and autosuspend_device(dev) == -1:
		rc = 0	
	f.close()
	return rc;	


def get_usbdev_by_id(vendor, product):
	result = []
	for d in devices:
		if d['idVendor'] == vendor and d['idProduct'] == product:
			result.append(d)
	return result

def restore_device_status():
	for dev in devices:
		try:
			f = open(dev['syspath'] + "/power/autosuspend", "w")
			f.write(dev['power/autosuspend'])
			f.close()
			f = open(dev['syspath'] + "/power/wakeup", "w")
			if dev['power/wakeup'] != '':
				f.write(dev['power/wakeup'])
			f.close()
			f = open(dev['syspath'] + "/power/level", "w")
			f.write(dev['power/level'])
			f.close()
		except IOError:
			print "can't open power/autosuspend or power/level or power/wakeup"	
	return 0


