/*
 * Copyright (C) 2009 Intel Corporation.
 * DESCR: check cpuidle governor
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 * 
 *                                                                        
 * Authors:                                                               
 *		Gong,Zhipeng  (zhipeng.gong@intel.com)
 *
 */

#include <stdio.h>
#include "sysfs.h"

int main (int argc, char ** argv){

	if (cpu_c_governor_check ("current_governor_ro", "menu") == SUCCESS) {
		printf("current_governor_ro is set to menu\n");
		return SUCCESS;
	}else {
		printf("current_governor_ro isn't set to menu\n");
		return ERROR;
	}
}
