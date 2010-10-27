/* compositor_runner.c -- Startup code for compositor

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
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <blts_log.h>

static pthread_t runner_thread = 0;

void *runner_thread_function(void *ptr)
{
	const char* cmd = (const char*)ptr;
	BLTS_DEBUG("Starting compositor '%s'\n", cmd);
	int exit_stat = system(cmd);
	if(WIFEXITED(exit_stat))
	{
		int child_exit_val = WEXITSTATUS(exit_stat);
		if (child_exit_val != 0)
		{
			BLTS_ERROR("Compositor returned %d\n",child_exit_val);
		}
	}
	else
	{
		BLTS_DEBUG("Compositor terminated unexpectedly.\n");
		if(WIFSIGNALED(exit_stat))
		{
			BLTS_DEBUG("Terminated on signal %d\n",WTERMSIG(exit_stat));
		}
		else
		{
			BLTS_DEBUG("Unknown termination reason.\n");
		}
	}
	return NULL;
}

int start_compositor(char* cmd)
{
	int ret;
	ret = pthread_create(&runner_thread, NULL, &runner_thread_function, cmd);
	/* TODO: wait until compositor is up and running */
	sleep(2);
	return ret;
}

int stop_compositor()
{
	/* TODO: Implement */
	return 0;
}

