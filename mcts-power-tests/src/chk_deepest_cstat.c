/*
 * Copyright (C) 2009 Intel Corporation.
 * DESCR: check whether CPU stay at the deepest C state when system is idle
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
#include <unistd.h>
#include <stdlib.h>

#include "sysfs.h"

int main (int argc, char ** argv){
	char *idle0_time1 = NULL;
	char *idlen_time1 = NULL;
	char *idle0_time2 = NULL;
	char *idlen_time2 = NULL;

	char deepest_cstate[10];
	char deepest_cstate_name[20];
	int cstate = cpu_c_statenumber_check();
	if (cstate == -1) {
		printf("fail to find the deepest c state");
		return 3;
	}
	sprintf(deepest_cstate, "state%d", cstate);
	cpu_c_state_name(cstate, deepest_cstate_name, sizeof deepest_cstate_name);
	printf("the deepest C staet is %s at %s\n", deepest_cstate_name, deepest_cstate);

	
	sleep (5);
	idle0_time1 = cpu_c_sys_check (1, "state0");
	idlen_time1 = cpu_c_sys_check (1, deepest_cstate);
	if (idlen_time1 == NULL) {
		printf("time is not available\n");
		return 3;
	}
	sleep (10);
	idle0_time2 = cpu_c_sys_check (1, "state0");
	idlen_time2 = cpu_c_sys_check (1, deepest_cstate);
	if (idlen_time2 == NULL) {
		printf("time is not available\n");
		return 3;
	}

	printf("before sleep C0 state %s, Cn state %s\n", idle0_time1, idlen_time1);
	printf("after sleep C0 state %s, Cn state %s\n", idle0_time2, idlen_time2);

	if (abs (atoll (idle0_time2) - atoll (idle0_time1)) > 10) {
		printf ("C0 state time is too long when idle! \n");
		return ERROR;
	} else {
		if (abs (atoll (idlen_time2) - atoll (idlen_time1)) < 5000000) {
			printf ("Cn state time is not long enough when idle! \n");
			return ERROR;
		}
	}
	return SUCCESS;
}
