/* timing.c -- Timer handling

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <time.h>


#include "blts_timing.h"

struct timer_t {
	int running;
	struct timespec starttime;
	struct timespec stoptime;
};


static struct timer_t timer;


/* result = left - right, return 0 if non-negative */
static int ts_diff(struct timespec *result,
		   struct timespec *left,
		   struct timespec *right)
{
	struct timespec a = { .tv_sec = left->tv_sec, .tv_nsec = left->tv_nsec };
	struct timespec b = { .tv_sec = right->tv_sec, .tv_nsec = right->tv_nsec };
	if(a.tv_nsec < b.tv_nsec)
	{
		long tmp_sec = (b.tv_nsec - a.tv_nsec) / 1000000000L + 1L;
		b.tv_nsec -= 1000000000L * tmp_sec;
		b.tv_sec += tmp_sec;
	}
	if(a.tv_nsec - b.tv_nsec > 1000000000L)
	{
		long tmp_sec = (a.tv_nsec - b.tv_nsec) / 1000000000L;
		b.tv_nsec += 1000000000L * tmp_sec;
		b.tv_sec -= tmp_sec;
	}

	result->tv_sec = a.tv_sec - b.tv_sec;
	result->tv_nsec = a.tv_nsec - b.tv_nsec;

	return (a.tv_sec < b.tv_sec);
}



int timing_start()
{
	if(timer.running) return 0;
	if(clock_gettime(CLOCK_REALTIME,&timer.starttime) < 0)
	{
		perror("Timer start");
		return -1;
	}
	timer.running = 1;
	return 0;
}

int timing_stop()
{
	if(!timer.running) return -1;
	if(clock_gettime(CLOCK_REALTIME,&timer.stoptime) < 0)
	{
		perror("Timer stop");
		return -1;
	}
	timer.running = 0;
	return 0;
}

/* return number of seconds timer has run*/
double timing_elapsed()
{
	struct timespec diff;
	if(timer.running)
	{
		struct timespec now;
		clock_gettime(CLOCK_REALTIME,&now);
		ts_diff(&diff,&now,&timer.starttime);
	}
	else
	{
		ts_diff(&diff,&timer.stoptime,&timer.starttime);
	}

	return (diff.tv_sec + diff.tv_nsec * 1E-9);
}
