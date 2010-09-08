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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define __USE_GNU
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define BUF_LEN 4096
#define TIMER_FLAG "Local timer"

volatile int ct_stop_workload = 0;
sem_t sem;

void *ct_cpu_workload_thread(void *cpu) {
	cpu_set_t mask;
	if ((int)cpu == 1) {
		printf("run cpu benchmark at CPU0\n");
		CPU_ZERO (&mask);
		CPU_SET (0, &mask);
		sched_setaffinity (0, sizeof(cpu_set_t), &mask);
	}
	printf("......start cpu workload......\n");
	sem_post(&sem);
    while (!ct_stop_workload) {
            long long i = 2;
            int j = 32;
            while (j--) {
                    i = i * 2;
            }
    }
	printf("......stop cpu workload......\n");
	if ((int) cpu == 1) {
		CPU_ZERO(&mask);
		sched_setaffinity(0, sizeof(cpu_set_t), &mask);
	}
    return NULL;
}

int read_timer_interrupts(unsigned long *timer1, unsigned long *timer2)
{
	FILE *fd;
	char buffer[BUF_LEN];
	char str1[BUF_LEN],str4[BUF_LEN];

	if ((fd = fopen("/proc/interrupts", "r")) == NULL) {
		perror("open");
		exit(2);
	}	
	
	while ((fgets(buffer, BUF_LEN, fd)) != NULL) {
		if (strstr(buffer, TIMER_FLAG) != NULL) {
			sscanf(buffer, "%s %lu %lu %s", str1, timer1, timer2, str4);			
			printf("\n-%s- -%lu- -%lu- -%s-\n", str1, *timer1, *timer2, str4);
		}
	}
	fclose(fd);
	return 0;
}


int main(int argc, char *argv[])
{
	pthread_t threadid;
	unsigned long timer1_before, timer2_before, timer1_after, timer2_after;
	int rc;
	int config_hz;
	sem_init(&sem, 0, 0);
	clock_t clock_before, clock_after;

	if (argc != 2) {
		printf("usage: %s config_hz\n", argv[0]);
		exit(2);
	}
	config_hz=atoi(argv[1]);
	if (pthread_create(&threadid, NULL, ct_cpu_workload_thread, (void *)1) != 0) {
		perror("create");
		exit(2);
	}
	sem_wait(&sem);
	printf("read timer interrupts\n");
	read_timer_interrupts(&timer1_before,&timer2_before);
	clock_before = clock();
	sleep(1);
	clock_after = clock();
	read_timer_interrupts(&timer1_after,&timer2_after);
	ct_stop_workload = 1;
	printf("before\t %lu %lu\n", timer1_before, timer2_before);
	printf("after\t %lu %lu\n", timer1_after, timer2_after);
	printf("diff\t %lu %lu\n", timer1_after - timer1_before, timer2_after - timer2_before);
	printf("clock\t %lu %lu %lu\n", (unsigned long)clock_before, 
			(unsigned long)clock_after, (unsigned long)(clock_after - clock_before));

	pthread_join(threadid, NULL);
	if (timer1_after - timer1_before >= ((config_hz * 90) / 100)) {
		rc = 0;
	} else {
		rc = 1;
	}
	return rc;
}
