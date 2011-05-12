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

 blts_openvg_test.c -- Command line interface for OpenVG tests
 Author: 
     Zheng Kui (kui.zheng@intel.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blts_cli_frontend.h>

#include "openvg_helper.h"
#include "test_common.h"

static void blts_openvg_help(const char *help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-t execution_time_in_seconds] [-w window_width] [-h window_height]"
		"[-d depth] [-c]",
		"-t: Maximum execution time of each test in seconds (default: 10s)\n"
		"-w: Used window width. If 0 uses desktop width. (default: 0)\n"
		"-h: Used window height. If 0 uses desktop height. (default: 0)\n"
		"-d: Used window depth. 16, 24 or 32. If 0 uses desktop depth. (default: 0)\n"
		"-c: Run the tests with minimal compositor\n");
}

static void *blts_openvg_argument_processor(int argc, char **argv)
{
	int t;
	test_execution_params *params = malloc(sizeof(test_execution_params));
	memset(params, 0, sizeof(test_execution_params));

	params->execution_time = 10;

	for (t = 1; t < argc; t++) {
		if (strcmp(argv[t], "-t") == 0) {
			if (++t >= argc)
				return NULL;
			params->execution_time = atoi(argv[t]);
		} else if (strcmp(argv[t], "-w") == 0) {
			if (++t >= argc)
				return NULL;
			params->w = atoi(argv[t]);
		} else if (strcmp(argv[t], "-h") == 0) {
			if (++t >= argc)
				return NULL;
			params->h = atoi(argv[t]);
		} else if (strcmp(argv[t], "-d") == 0) {
			if (++t >= argc)
				return NULL;
			params->d = atoi(argv[t]);
		} else if (strcmp(argv[t], "-c") == 0) {
			params->use_compositor = 1;
		} else {
			return NULL;
		}
	}

	blts_cli_set_timeout((params->execution_time + 30) * 1000);

	return params;
}

static int exec_test(void *user_ptr, int test_num)
{
	test_execution_params *params = user_ptr;
	int ret = 0;

	switch (test_num) {
		/* smoke tests */
	case 1:
		ret = test_enum_eglextensions(params);
		break;
	case 2:
		ret = test_enum_eglconfigs(params);
		break;
		/* function tests */
	case 3:
		ret = test_transf(params);
		break;
	case 4:
		ret = test_stroke(params);
		break;
	case 5:
		ret = test_fill(params);
		break;
	case 6:
		ret = test_clear(params);
		break;
	case 7:
		ret = test_lingrad(params);
		break;
	case 8:
		ret = test_radialgrad(params);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static blts_cli_testcase blts_openvg_cases[] = {
	{"OpenVG - Enumerate EGL extensions", exec_test, 20000},
	{"OpenVG - Enumerate EGL configs", exec_test, 20000},
	{"OpenVG - Transformation testing", exec_test, 20000},
	{"OpenVG - Stroking Path testing", exec_test, 20000},
	{"OpenVG - Filling Path testing", exec_test, 20000},
	{"OpenVG - Clearing testing", exec_test, 20000},
	{"OpenVG - Linear Gradients testing", exec_test, 20000},
	{"OpenVG - Radial Gradients testing", exec_test, 20000},
	BLTS_CLI_END_OF_LIST
};

static blts_cli openvg_cli = {
	.test_cases = blts_openvg_cases,
	.log_file = "blts_openvg_perf.txt",
	.blts_cli_help = blts_openvg_help,
	.blts_cli_process_arguments = blts_openvg_argument_processor,
//      .blts_cli_teardown = blts_openvg_teardown
};

int main(int argc, char **argv)
{
	return blts_cli_main(&openvg_cli, argc, argv);
}
