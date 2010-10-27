/* ogles2_perf_test.c -- Command line interface for GLES2 tests

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
#include <blts_cli_frontend.h>

#include "ogles2_helper.h"
#include "test_blitter.h"
#include "test_common.h"
#include "compositor_runner.h"
#include "ogles2_conf_file.h"

const char* config_filename = "/usr/lib/tests/blts-opengles2-tests/blts-opengles2-perf.cnf";

static void blts_gles2_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-t execution_time_in_seconds] [-w window_width] [-h window_height]"
		"[-d depth] [-c]"
		,
		"-t: Maximum execution time of each test in seconds (default: 10s)\n"
		"-w: Used window width. If 0 uses desktop width. (default: 0)\n"
		"-h: Used window height. If 0 uses desktop height. (default: 0)\n"
		"-d: Used window depth. 16, 24 or 32. If 0 uses desktop depth. (default: 0)\n"
		"-c: Run the tests with minimal compositor\n"
		);
}

static void* blts_gles2_argument_processor(int argc, char **argv)
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
		else if(strcmp(argv[t], "-w") == 0)
		{
			if(++t >= argc) return NULL;
			params->w = atoi(argv[t]);
		}
		else if(strcmp(argv[t], "-h") == 0)
		{
			if(++t >= argc) return NULL;
			params->h = atoi(argv[t]);
		}
		else if(strcmp(argv[t], "-d") == 0)
		{
			if(++t >= argc) return NULL;
			params->d = atoi(argv[t]);
		}
		else if(strcmp(argv[t], "-c") == 0)
		{
			params->use_compositor = 1;
		}
		else
		{
			return NULL;
		}
	}

	blts_cli_set_timeout((params->execution_time + 30) * 1000);

	return params;
}

static void blts_gles2_teardown(void *user_ptr)
{
	if(user_ptr)
	{
		test_execution_params* params = user_ptr;
		if(params->use_compositor)
		{
			stop_compositor();
		}

		free(user_ptr);
	}
}

static int exec_test(void* user_ptr, int test_num)
{
	test_execution_params* params = user_ptr;
	int ret = 0;

	if(read_config(config_filename, &params->config))
	{
		BLTS_ERROR("Failed to read configuration file\n");
		return 1;
	}

	if(params->use_compositor)
	{
		if(start_compositor(params->config.compositor_cmd))
		{
			BLTS_ERROR("Failed to start compositor. Running without it.\n");
		}
	}

	switch(test_num)
	{
	/* smoke tests */
	case 1:
		ret = test_enum_glextensions(params);
		break;
	case 2:
		ret = test_enum_eglextensions(params);
		break;
	case 3:
		ret = test_enum_eglconfigs(params);
		break;
	case 4:
		ret = test_simple_tri(params);
		break;

	/* graphics benchmarks */
	case 5:
		params->flag = 0;
		ret = test_blitter(params);
		break;
	case 6:
		params->flag = T_FLAG_BLEND;
		ret = test_blitter(params);
		break;
	case 7:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS;
		ret = test_blitter(params);
		break;
	case 8:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS;
		ret = test_blitter(params);
		break;
	case 9:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|
			T_FLAG_PARTICLES;
		ret = test_blitter(params);
		break;
	case 10:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|
			T_FLAG_ROTATE|T_FLAG_PARTICLES;
		ret = test_blitter(params);
		break;
	case 11:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|
			T_FLAG_ZOOM|T_FLAG_ROTATE|T_FLAG_PARTICLES;
		ret = test_blitter(params);
		break;
	case 12:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|
			T_FLAG_ZOOM|T_FLAG_ROTATE|T_FLAG_BLUR|T_FLAG_PARTICLES;
		ret = test_blitter(params);
		break;
	case 13:
		params->flag = T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|
			T_FLAG_VIDEO_WIDGETS;
		ret = test_blitter(params);
		break;

	/* more synthetic benchmarks */
	case 14:
		ret = test_polygons(params);
		break;
	case 15:
		ret = test_fillrate(params);
		break;
	case 16:
		ret = test_texels(params);
		break;
	case 17:
		ret = test_frag_shader(params);
		break;
	case 18:
		ret = test_vert_shader(params);
		break;

	/* for testing filters */
	case 19:
		params->flag = T_FLAG_CONVOLUTION|T_FLAG_BLEND|T_FLAG_WIDGETS|
			T_FLAG_WIDGET_SHADOWS;
		ret = test_blitter(params);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if(params->use_compositor) stop_compositor();

	return ret;
}

static blts_cli_testcase blts_gles2_cases[] =
{
	{ "OpenGL-Enumerate GL extensions", exec_test, 20000 },
	{ "OpenGL-Enumerate EGL extensions", exec_test, 20000 },
	{ "OpenGL-Enumerate EGL configs", exec_test, 20000 },
	{ "OpenGL-Simple triangle", exec_test, 20000 },
	{ "OpenGL-Simple blit", exec_test, 20000 },
	{ "OpenGL-Blit with blend", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets with shadows", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets with shadows + particles", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets with shadows + particles + rotate", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets with shadows + particles + rotate + zoom", exec_test, 20000 },
	{ "OpenGL-Blit with blend and widgets with shadows + particles + rotate + zoom and blur all", exec_test, 20000 },
	{ "OpenGL-Blit with blend and animated widgets with shadows", exec_test, 20000 },
	{ "OpenGL-Polygons-per-second", exec_test, 20000 },
	{ "OpenGL-Fillrate test", exec_test, 20000 },
	{ "OpenGL-Texels-per-second", exec_test, 20000 },
	{ "OpenGL-Fragment shader performance", exec_test, 20000 },
	{ "OpenGL-Vertex shader performance", exec_test, 20000 },
	{ "OpenGL-Convolution filter", exec_test, 20000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli gles2_cli =
{
	.test_cases = blts_gles2_cases,
	.log_file = "blts_gles2_perf.txt",
	.blts_cli_help = blts_gles2_help,
	.blts_cli_process_arguments = blts_gles2_argument_processor,
	.blts_cli_teardown = blts_gles2_teardown
};

int main(int argc, char **argv)
{
	return blts_cli_main(&gles2_cli, argc, argv);
}

