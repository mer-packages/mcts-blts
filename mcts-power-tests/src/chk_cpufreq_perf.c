/*
 * Copyright (C) 2009 Intel Corporation.
 * DESCR:check cpufreq performance governor
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
#include <stdlib.h>
#include <unistd.h>

#include "sysfs.h"

int main (int argc, char ** argv){
	char *avai_freq_arr[FREQ_NUM_MAX];
	char *max_freq = NULL;
	char *min_freq = NULL;

	if (parse_avail_freq(avai_freq_arr,&max_freq,&min_freq, NULL) == ERROR) {
		printf("Fail to read scaling_available_frequencies\n");
		return ERROR;
	}
#if 0
	if (cpu_p_sys_write (0, "scaling_governor", "userspace") != SUCCESS){
		return ERROR;
	}
	
	if (cpu_p_sys_write (0, "scaling_setspeed", min_freq) != SUCCESS){
		return ERROR;
	}

	if (cpu_p_sys_check (0, "scaling_cur_freq", min_freq) != SUCCESS){
		return ERROR;
	}
#endif

	if (cpu_p_sys_write (1, "scaling_governor", "performance") != SUCCESS){
		return ERROR;
	}
	
	if (cpu_p_sys_check (1, "scaling_governor", "performance") != SUCCESS) {
		return ERROR;
	} else {
		printf ("set governor to performance successfully! \n");
	}

	if (cpu_p_sys_check (1, "scaling_cur_freq", max_freq) != SUCCESS){
		return ERROR;
	} else {
		printf ("performance governor has taken effective! The frequency is %s now \n", max_freq);
	}

	if (cpu_p_sys_check (1, "cpuinfo_cur_freq", max_freq) != SUCCESS){
		return ERROR;
	} else {
		printf ("All cpus' frequencies have been updated! \n");
	}
	return SUCCESS;	
}
