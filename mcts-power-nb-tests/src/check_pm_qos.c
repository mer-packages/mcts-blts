/*
 * Copyright (C) 2009 Intel Corporation.
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
 * Authors:                                                               
 *              Gong,Zhipeng  (zhipeng.gong@intel.com)
 *
 */
 
 


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sysfs.h"

#define MAX_C_STATE 8
#define DEVICE_LATENCY_FILE "/dev/cpu_dma_latency"


#define DEBUG(fmt, args...) printf(fmt, ##args)

/*
	find c state latency
	open cpu_dma_latency and write the latency-1 
	stay for a long while to check the stability
	check the usage and time of each c state
	close cpu_dma_latency
	
*/
struct cpu_cstate {
	unsigned long long usage[MAX_C_STATE];
	unsigned long long time[MAX_C_STATE];
	int latency[MAX_C_STATE];
};

static void get_cpu_cstate(struct cpu_cstate *c, int cstate_num)
{
	int i;
	
	for (i = 0; i <= cstate_num; i++) {
		cpu_c_state_usage(i, &c->usage[i]);
		cpu_c_state_time(i, &c->time[i]);
		cpu_c_state_latency(i, &c->latency[i]);
		printf("c state%d latency:%d usage:%lld time:%lld\n", i, c->latency[i], c->usage[i], c->time[i]);
	}

}

int main(int argc, char *argv[])
{
	int latency;
	char deepest_cstate[10];
	char deepest_cstate_name[20];
	int cstate;
	int file;
	int index;	
	int rc = 0;
	int sleep_time = 5;
	int i;
	struct cpu_cstate old_state, new_state;

	if (argc < 2) {
		printf("usage: %s dma_latency sleep_time\n", argv[0]);
		exit(2);
	}

	latency = atoi(argv[1]);

	if (argc == 3) {
		sleep_time = atoi(argv[2]);
	}
	printf("sleep_time is %d\n", sleep_time);

	cstate = cpu_c_statenumber_check();
	if (cstate == -1) {
		printf("fail to find the deepest c state");
		return 2;
	}
	sprintf(deepest_cstate, "state%d", cstate);
	cpu_c_state_name(cstate, deepest_cstate_name, sizeof deepest_cstate_name);
	printf("the deepest C staet is %s at %s\n", deepest_cstate_name, deepest_cstate);
	get_cpu_cstate(&old_state, cstate);

	for (index = 0; index <= cstate; index++) {
		if (old_state.latency[index] > latency)
			break;
	}

	printf("index is %d \n", index);
	
	if ((file = open(DEVICE_LATENCY_FILE, O_WRONLY)) == -1) {
		perror(DEVICE_LATENCY_FILE);
		return -1;
	}
	DEBUG("Write latency %d to file %s\n", latency, DEVICE_LATENCY_FILE);
	if (write(file, &latency, sizeof(latency)) == -1){
		perror("write");
		close(file);
		return 1;
	}
	sleep(sleep_time);
	
	close(file);
	get_cpu_cstate(&new_state, cstate);


	//-1 is actually maximum allowed latency, so only check the deepest state usage
	//-2 shall not be allowed
	if (latency <= -1) {
		if ((new_state.usage[cstate] - old_state.usage[cstate]) > 3 ) {
			printf("pass state%d usage is changed, old: %llu, new: %llu\n", cstate,
				old_state.usage[cstate], new_state.usage[cstate]);
			return 0;
		}
		return -1;
	}


	for (i = index; i <= cstate; i++) {
		DEBUG("Compare state%d usage, old: %llu, new: %llu\n", i,
			old_state.usage[i], new_state.usage[i]);
		
		if ((new_state.usage[i] - old_state.usage[i]) > 3 ) {
			printf("error state%d usage is changed, old: %llu, new: %llu\n", i + 1,
				old_state.usage[i], new_state.usage[i]);
			rc = 1;
		}

	}	

	return rc;
}

