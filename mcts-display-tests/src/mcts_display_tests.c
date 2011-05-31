/*
 Copyright (C) 2010 Intel Corporation
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 Author: 
     Zheng Kui (kui.zheng@intel.com)
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <blts_cli_frontend.h>

#include "display_helper.h"
#include "display_common.h"

typedef struct
{
	int execution_time;
} test_execution_params;

static void blts_display_help (const char *help_msg_base)
{
	fprintf (stdout, help_msg_base, "[-t execution_time_in_seconds]",
		"-t: Maximum execution time of each test in seconds (default: 10s)\n");
}

static void *blts_display_argument_processor (int argc, char **argv)
{
	int t;
	test_execution_params *params = malloc (sizeof (test_execution_params));
	memset (params, 0, sizeof (test_execution_params));

	params->execution_time = 10;

	for (t = 1; t < argc; t++)
	{
		if (strcmp (argv[t], "-t") == 0)
		{
			if (++t >= argc)
				return NULL;
			params->execution_time = atoi (argv[t]);
		}
		else
		{
			return NULL;
		}
	}

	blts_cli_set_timeout ((params->execution_time + 10) * 1000);

	return params;
}

static void blts_display_teardown (void *user_ptr)
{
	if (user_ptr)
	{
		free (user_ptr);
	}
}

static int exec_test (void *user_ptr, int test_num)
{
	test_execution_params *params = user_ptr;
	int ret = 0;

	switch (test_num)
	{
	case 1:
		ret = test_enumerate (params->execution_time);
		break;
	case 2:
		ret = test_modes (params->execution_time);
		break;
	case 3:
		ret = test_backlight (params->execution_time);
		break;
	case 4:
		ret = test_rotate (params->execution_time);
		break;
	case 5:
		ret = test_position (params->execution_time);
		break;
	case 6:
		ret = test_scale (params->execution_time);
		break;
	case 7:
		ret = test_hotplug (params->execution_time);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static blts_cli_testcase blts_display_cases[] = {
	{"Display-Enumerate Screen/Crtc/Output/Modeline test", exec_test, 20000},
	{"Display-Set Crtc Modeline test", exec_test, 20000},
	{"Display-Set Backlight test", exec_test, 20000},
	{"Display-Rotation test", exec_test, 60000},
	{"Display-Set Crtc Position test", exec_test, 20000},
	{"Display-Crtc Scaling test", exec_test, 20000},
	{"Display-Hotplug external display", exec_test, 20000},
	BLTS_CLI_END_OF_LIST
};

static blts_cli display_cli = {
	.test_cases = blts_display_cases,
	.log_file = "blts_display.txt",
	.blts_cli_help = blts_display_help,
	.blts_cli_process_arguments = blts_display_argument_processor,
	.blts_cli_teardown = blts_display_teardown
};

int main (int argc, char **argv)
{
	return blts_cli_main (&display_cli, argc, argv);
}
