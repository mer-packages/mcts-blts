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
 
 


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>

#include "sysfs.h"

static int find_word(const char *word, FILE *fd) {
        int rc;
        char buf[LINE_LEN] = {'\0'};
        char line[LINE_LEN] = {'\0'};
        char *c;

        while (1) {
                int i = 0, j = 0;

                c = fgets(buf, LINE_LEN-1, fd);
                if (c == NULL && ferror(fd)) {
                        rc = ERROR;
                        break;
                }

                if (c == NULL) {
                        rc = ERROR;
                        break;
                }


                do {
                        if (!isspace(buf[i]))
                                line[j++] = buf[i];
                } while(buf[++i]);

                line[j] = '\0';

                c = strstr(line, word);
                if (c) {
                        rc = SUCCESS;
                        break;
                }

        }

        return rc;
}

char *cpu_p_sys_get_line (int cpu, char *file) {
	FILE *fd;
	char *buf;
	char p_file[FILE_NAME_MAX] = {'\0'};

	if (cpu) {
		sprintf(p_file, "/sys/devices/system/cpu/cpu%i/cpufreq/%s",
                        cpu - 1, file);
                fd = fopen (p_file, "r");
                if (fd) {
			buf = (char *)malloc (LINE_LEN * sizeof(char));
			fgets (buf, LINE_LEN, fd);
			fclose (fd);
			return buf;
                } else {
			printf ("Fail to open file: %s \n", p_file);
			return NULL;
		}
	} else {
		printf ("cpu shoud >=1 \n");
		return NULL;
	}
}

int cpu_p_sys_write (int cpu, char *file, char *value) {
        int result;
        char p_file[FILE_NAME_MAX] = {'\0'};
        char p_dir[FILE_NAME_MAX] = {'\0'};
        DIR *dir;
        struct dirent *dirent;
		FILE *fd;

        result = SUCCESS;
        if (cpu) {      
			sprintf(p_file, "/sys/devices/system/cpu/cpu%i/cpufreq/%s",
				cpu - 1, file);
			fd = fopen (p_file, "w");
			if (!fd) {
				printf ("failed to open %s\n", p_file);
				result = ERROR;
			}

			fprintf (fd, "%s", value);
			if (ferror (fd)){
				printf ("Fail to write value %s to file %s \n", value, p_file);
				result = ERROR;
			}	 
			fclose (fd);
			return result;
        }

        sprintf(p_dir, "/sys/devices/system/cpu");
        dir = opendir(p_dir);
        if (!dir) {
                printf ("failed to open %s\n", p_dir);
                return ERROR;
        }

        while ((dirent = readdir(dir)) != NULL) {
            if (!strstr(dirent->d_name, "cpu"))
                    continue;
            if (strstr(dirent->d_name, "cpuidle"))
                    continue;

            sprintf(p_file, "%s/%s/cpufreq/%s", p_dir, dirent->d_name, file);
			fd = fopen (p_file, "w");
			if (!fd) {
                printf ("failed to open %s\n", p_file);
                result = ERROR;
                continue;
	        }
			
			fprintf (fd, "%s", value);
            if (ferror (fd)){
                printf ("Fail to write value %s to file %s \n", value, p_file);
                result = ERROR;
	         }        
			fclose (fd);
        }

        closedir(dir);
        return result;
}

int cpu_p_sys_check(int cpu, char *file, char *value) {
        int rc, result;
        char p_file[FILE_NAME_MAX] = {'\0'};
        char p_dir[FILE_NAME_MAX] = {'\0'};
        DIR *dir;
        struct dirent *dirent;
        FILE *fd;

        result = SUCCESS;
        if (cpu) {     
                sprintf(p_file, "/sys/devices/system/cpu/cpu%i/cpufreq/%s",
                        cpu - 1, file);
                fd = fopen(p_file, "r");
                if (!fd) {
                        printf ("failed to open %s\n", p_file);
                        return ERROR;
                }

                rc = find_word(value, fd);
                if (rc != SUCCESS) {
                        printf ("%s is not found in %s\n", value, p_file);
                        result = ERROR;
                }
                fclose(fd);
                return result;
        }

        sprintf(p_dir, "/sys/devices/system/cpu");
        dir = opendir(p_dir);
        if (!dir) {
                printf ("failed to open %s\n", p_dir);
                return ERROR;
        }

        while ((dirent = readdir(dir)) != NULL) {
                if (!strstr(dirent->d_name, "cpu"))
                        continue;
                if (strstr(dirent->d_name, "cpuidle"))
                        continue;

                sprintf(p_file, "%s/%s/cpufreq/%s", p_dir, dirent->d_name, file);
                fd = fopen(p_file, "r");
                if (!fd) {
                        printf ("failed to open %s\n", p_file);
                        result = ERROR;
                        continue;
                }

                rc = find_word(value, fd);
                if (rc != SUCCESS) {
                        printf ("%s is not found in %s\n", value, p_file);
                        result = ERROR;
                }
                fclose(fd);
        }

        closedir(dir);
        return result;
}

int cpu_c_governor_check(char *file, char *value) {
        int rc, result;
        char p_file[FILE_NAME_MAX] = {'\0'};
        FILE *fd;

        result = SUCCESS;
        sprintf(p_file, "/sys/devices/system/cpu/cpuidle/%s",
                file);
        fd = fopen(p_file, "r");
        if (!fd) {
                printf ("failed to open %s\n", p_file);
                return ERROR;
        }

        rc = find_word(value, fd);
        if (rc != SUCCESS) {
                printf ("%s is not found in %s\n", value, p_file);
                result = ERROR;
        }
        fclose(fd);

        return result;
}

char * cpu_c_sys_check(int cpu, char *entry){
        int result;
        char c_file[FILE_NAME_MAX] = {'\0'};
        FILE *fd;
	char *buf;

        result = SUCCESS;
        if (cpu) {      /* if cpu# is not 0, just set the single cpu */
                sprintf(c_file, "/sys/devices/system/cpu/cpu%i/cpuidle/%s/time",
                        cpu - 1, entry);
                fd = fopen(c_file, "r");
                if (fd) {
                        buf = (char *)malloc (LINE_LEN * sizeof(char));
                        fgets (buf, LINE_LEN, fd);
			fclose (fd);
                        return buf;
                } else {
                        printf ("Fail to open file: %s \n", c_file);
                        return NULL;
                }
	} else {
                printf ("cpu shoud >=1 \n");
                return NULL;
        }

}


int parse_avail_freq(char *avai_freq_arr[], char **max_freq, char **min_freq, int *freq_num) {

	char *avai_freq = cpu_p_sys_get_line (1, "scaling_available_frequencies");
	char *p;
	int i = 0;

	if (avai_freq == NULL)
		return ERROR;
	p = strtok (avai_freq, " \n\t");
	avai_freq_arr[i] = (char *)malloc (strlen(p)+1);
	strcpy (avai_freq_arr[i++], p);
	while ((p = strtok (NULL, " \n\t")) != NULL){
			avai_freq_arr[i] = (char *)malloc(strlen(p)+1);
			strcpy (avai_freq_arr[i++], p);
	}
	if (freq_num != NULL) 
		*freq_num = i;

	printf("freq number is %d\n", i);
	// the frequencies are arranged from max to min, so the first one is max freq, and the last one is min freq.
	*max_freq = avai_freq_arr[0];
	*min_freq = avai_freq_arr[i-1];
	
	while (i >0 ){
		printf("%s\n", avai_freq_arr[--i]);
	}
	return SUCCESS;
}

volatile int stop_workload = 0;

void *cpu_workload_thread(void *cpu) {
	cpu_set_t mask;
	if ((int)cpu == 1) {
		printf("run cpu workload at CPU0\n");
		CPU_ZERO (&mask);
		CPU_SET (0, &mask);
		sched_setaffinity (0, sizeof(cpu_set_t), &mask);
	}
    while (!stop_workload) {
            long long i = 2;
            int j = 32;
            while (j--) {
                    i = i * 2;
            }
    }
	if ((int) cpu == 1) {
		CPU_ZERO(&mask);
		sched_setaffinity(0, sizeof(cpu_set_t), &mask);
	}
    return NULL;
}

int cpu_c_statenumber_check(void){
	DIR *cpudir;
	DIR *dir;
	struct dirent *entry;
	char filename[128];
	int len, clevel = -1;

	cpudir = opendir("/sys/devices/system/cpu");
	if (!cpudir)
		return -1;

	/* Loop over cpuN entries */
	while ((entry = readdir(cpudir))) {
		if (strlen(entry->d_name) < 3)
			continue;

		if (!isdigit(entry->d_name[3]))
			continue;

		len = sprintf(filename, "/sys/devices/system/cpu/%s/cpuidle",
			      entry->d_name);
		dir = opendir(filename);
		if (!dir)
			return -1;

		/* For each C-state, there is a stateX directory which
		 * contains a 'usage' and a 'time' (duration) file */
		while ((entry = readdir(dir))) {
			if (strlen(entry->d_name) < 3)
				continue;
			sprintf(filename + len, "/%s/desc", entry->d_name);
			clevel++;
		}
		
		return clevel;
	}
	return clevel;

}


void cpu_c_state_name(int cstate, char *name, int len)
{
	FILE *file;
	char filename[128];
	char *f;

	name[0] = '\0';
	sprintf(filename, "/sys/devices/system/cpu/cpu0/cpuidle/state%d/name", cstate);
	file = fopen(filename, "r");
	if (!file)
		return;
	f = fgets(name, len, file);
	if (f == NULL)
		name[0] = '\0';

	fclose(file);

}

void cpu_c_state_latency(int cstate, int *latency)
{
	FILE *file;
	char filename[128];
	char *f;
	char buffer[32];

	*latency = -1;
	sprintf(filename, "/sys/devices/system/cpu/cpu0/cpuidle/state%d/latency", cstate);
	file = fopen(filename, "r");
	if (!file)
		return;
	f = fgets(buffer, sizeof buffer, file);
	if (f == NULL)
		return;
		
	*latency = atoi(buffer);
	fclose(file);

}

void cpu_c_state_usage(int cstate, unsigned long long  *usage)
{
	FILE *file;
	char filename[128];
	char *f;
	char buffer[32];

	*usage = -1;
	sprintf(filename, "/sys/devices/system/cpu/cpu0/cpuidle/state%d/usage", cstate);
	file = fopen(filename, "r");
	if (!file)
		return;
	f = fgets(buffer, sizeof buffer, file);
	if (f == NULL)
		return;
		
	*usage = strtoull(buffer, NULL, 10);
	fclose(file);

}

void cpu_c_state_time(int cstate, unsigned long long *time)
{
	FILE *file;
	char filename[128];
	char *f;
	char buffer[32];

	*time = -1;
	sprintf(filename, "/sys/devices/system/cpu/cpu0/cpuidle/state%d/time", cstate);
	file = fopen(filename, "r");
	if (!file)
		return;
	f = fgets(buffer, sizeof buffer, file);
	if (f == NULL)
		return;
		
	*time = strtoull(buffer, NULL, 10);
	fclose(file);

}

void cpu_workload(void) {
        long rounds = 20000000;

        while (rounds--) {
                long long i = 2;
                int j = 32;
                while (j--) {
                        i = i * 2;
                }
                rounds--;
        }

        return;
}

unsigned long run_cpu_workload(int cpu) {
	struct timeval tv0, tv1;
	unsigned long exec_time;
	cpu_set_t mask;
	pid_t pid;

	if (cpu) {
		pid = getpid();
		CPU_ZERO(&mask);
		CPU_SET(cpu-1, &mask);
		sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
	}

	gettimeofday(&tv0, NULL);
	cpu_workload();
	gettimeofday(&tv1, NULL);

	exec_time = (tv1.tv_sec-tv0.tv_sec)*1000000 + (tv1.tv_usec-tv0.tv_usec);
	if (cpu) {
		/* turn back to two cpu */
		CPU_ZERO(&mask);
		sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
	}

	return exec_time;
}


