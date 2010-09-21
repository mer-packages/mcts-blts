/* blts_x11_test.c -- Command line interface

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
#include <errno.h>
#include <blts_cli_frontend.h>

#include "blts_x11_util.h"
#include "xcomposite_tests.h"
#include "xdamage_tests.h"
#include "xinput_tests.h"
#include "xrandr_tests.h"
#include "xrecord_tests.h"
#include "xrender_tests.h"
#include "xtest_tests.h"
#include "xvideo_tests.h"

typedef struct
{
	int execution_time;
} test_execution_params;

static void blts_x11_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-t execution_time_in_seconds]"
		,
		"-t: Maximum execution time of each test in seconds (default: 10s)\n"
		);
}

static void* blts_x11_argument_processor(int argc, char **argv)
{
	int t;
	test_execution_params* params = malloc(sizeof(test_execution_params));
	memset(params, 0, sizeof(test_execution_params));

	params->execution_time = 10;

	for(t = 1; t < argc; t++)
	{
		if(strcmp(argv[t], "-t") == 0)
		{
			if(++t >= argc) return NULL;
			params->execution_time = atoi(argv[t]);
		}
		else
		{
			return NULL;
		}
	}

	blts_cli_set_timeout((params->execution_time + 10) * 1000);

	return params;
}

static void blts_x11_teardown(void *user_ptr)
{
	if(user_ptr)
	{
		free(user_ptr);
	}
}

static int exec_test(void* user_ptr, int test_num)
{
	test_execution_params* params = user_ptr;
	int ret = 0;

	switch(test_num)
	{
	case 1:
		ret = x11_dep_check();
		break;
	case 2:
		ret = create_simple_window(params->execution_time);
		break;
	case 3:
		ret = enumerate_x_extensions();
		break;
	case 4:
		ret = xvideo_init_test(params->execution_time);
		break;
	case 5:
		ret = xtest_move_cursor(params->execution_time);
		break;
	case 6:
		ret = xcomposite_get_widow_contents();
		break;
	case 7:
		ret = xrecord_capture_mouse_movement(params->execution_time);
		break;
	case 8:
		ret = xrender_draw_rectangle(params->execution_time);
		break;
	case 9:
		ret = xrandr_rotate_screen(params->execution_time);
		break;
	case 10:
		ret = xdamage_monitor_region(params->execution_time);
		break;
	case 11:
		ret = xinput_list_devices();
		break;
	default:
		ret = -EINVAL;
		break;
	}
	
	return ret;
}

static blts_cli_testcase blts_x11_cases[] =
{
	{ "X11-X11 presence check", exec_test, 20000 },
	{ "X11-Create simple window", exec_test, 20000 },
	{ "X11-Enumerate X extensions", exec_test, 20000 },
	{ "X11-XVideo init", exec_test, 20000 },
	{ "X11-XTest move cursor", exec_test, 20000 },
	{ "X11-XComposite get window contents", exec_test, 20000 },
	{ "X11-XRecord capture mouse movement", exec_test, 20000 },
	{ "X11-XRender draw rectangle", exec_test, 20000 },
	{ "X11-XRandR rotate screen", exec_test, 20000 },
	{ "X11-XDamage monitor region", exec_test, 20000 },
	{ "X11-XInput enumerate devices", exec_test, 20000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli x11_cli =
{
	.test_cases = blts_x11_cases,
	.log_file = "blts_x11.txt",
	.blts_cli_help = blts_x11_help,
	.blts_cli_process_arguments = blts_x11_argument_processor,
	.blts_cli_teardown = blts_x11_teardown
};

int main(int argc, char **argv)
{
	return blts_cli_main(&x11_cli, argc, argv);
}

