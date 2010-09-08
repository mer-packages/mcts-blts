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


import time
import os
import commands

timer_flag = "Local timer interrupts"
interrupt_file = open("/proc/interrupts", "r")
cpu_num = len(interrupt_file.readline().split())
config_hz = 300


def get_timer_interrupt():
	interrupt_file.seek(0)
	ti = []
	for s in interrupt_file:
		if s.find(timer_flag) > 0:
			for n in s.split():
				if n.isdigit():
					ti.append(int(n))
			break
	return ti

def get_config_hz():
	global config_hz
	try:
		for f in os.listdir('/boot'):
			if f.find("config-"+os.uname()[2]) != -1:
				for s in open("/boot/" + f, "r"):
					if s.find("CONFIG_HZ=") != -1:
						print 'In config file /boot/%s find %s ' % (f, s)
						config_hz = int(s[10:].rstrip())
	except IOError:
		print 'fail to open /boot/config-*'
		pass	
	
if __name__ == "__main__":
	old_tis = get_timer_interrupt()
	time.sleep(1)
	new_tis = get_timer_interrupt()
	interrupt_file.close()


	get_config_hz()
	print config_hz

	result=[]
	for i in range(len(old_tis)):
		diff = new_tis[i] - old_tis[i]
		print 'CPU %d timer interrupt diff: %d-%d=%d' % (i, new_tis[i], old_tis[i], diff)
		if (diff > config_hz):
			print 'timer interrupt diff is greater than CONFIG_HZ after 1 seconds'
			result.append(1)
		else:
			print 'timer interrupt diff is lower than CONFIG_HZ after 1 seconds'
			result.append(0)

	for i in result:
		if i > 0:
			exit(1)

	exit(0)

		

