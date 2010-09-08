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
 
 

#ifndef _SYSFS_H
#define _SYSFS_H

#define FILE_NAME_MAX 256
#define LINE_LEN 1024

#define SUCCESS    0
#define ERROR      1

#define FREQ_NUM_MAX 8

int cpu_p_sys_check(int cpu, char *file, char *value);
int cpu_p_sys_write(int cpu, char *file, char *value);
char * cpu_p_sys_get_line (int cpu, char *file);
void *cpu_workload_thread(void *cpu);
unsigned long run_cpu_workload(int cpu);
int cpu_c_governor_check(char *file, char *value);
char * cpu_c_sys_check(int cpu, char *entry);
int parse_avail_freq(char *avai_freq_arr[], char **max_freq, char **min_freq, int *freq_num);
int cpu_c_statenumber_check(void);
void cpu_c_state_name(int cstate, char *name, int len);
void cpu_c_state_usage(int cstate, unsigned long long  *usage);
void cpu_c_state_time(int cstate, unsigned long long  *time);
void cpu_c_state_latency(int cstate, int *latency);
extern volatile int stop_workload;
#endif
