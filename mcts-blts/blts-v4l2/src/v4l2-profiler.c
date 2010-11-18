/* v4l2-profiler.c -- Functionality to profile v4l2 ioctl calls

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <blts_log.h>
#include "v4l2-xvideo.h"
#include "v4l2-profiler.h"

#define MAX_PROFILED_IOCTLS 256

static pthread_mutex_t prof_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	char* str;
	int value;
} ioctl_s;

static ioctl_s ioctl_calls[MAX_PROFILED_IOCTLS];
static int profiling_enabled = 0;

void ioctl_start_profiling()
{
	if(profiling_enabled)
		ioctl_stop_profiling();

	memset(ioctl_calls, 0, sizeof(ioctl_calls));
	profiling_enabled = 1;
}

void ioctl_stop_profiling()
{
	int t = 0;

	if(!profiling_enabled)
		return;

	profiling_enabled = 0;
	while(ioctl_calls[t].str)
	{
		free(ioctl_calls[t].str);
		t++;
	}
	memset(ioctl_calls, 0, sizeof(ioctl_calls));
}

void ioctl_print_profiling_data()
{
	int t = 0;

	if(!profiling_enabled)
		return;

	BLTS_DEBUG("\nProfiling data:\n");
	while(ioctl_calls[t].str)
	{
		BLTS_DEBUG("%s: %d\n", ioctl_calls[t].str, ioctl_calls[t].value);
		t++;
	}
}

void ioctl_profile(const char* name)
{
	int t = 0;

	pthread_mutex_lock(&prof_mutex);

	if(!profiling_enabled)
		goto cleanup;

	while(ioctl_calls[t].str)
	{
		if(!strcmp(ioctl_calls[t].str, name))
		{
			ioctl_calls[t].value++;
			goto cleanup;
		}
		t++;
	}

	if(t >= MAX_PROFILED_IOCTLS)
	{
		BLTS_DEBUG("MAX_PROFILED_IOCTLS reached - ioctl %s omitted!\n", name);
		goto cleanup;
	}

	ioctl_calls[t].str = strdup(name);
	ioctl_calls[t].value++;

cleanup:
	pthread_mutex_unlock(&prof_mutex);
}
