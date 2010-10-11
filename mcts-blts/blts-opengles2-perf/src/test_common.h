/* test_common.h -- Common definitions

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

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <limits.h>

#define MAX_CONV_MAT_SIZE 128

typedef struct
{
	int scale_images_to_window;
	int desktop_count;
	int layer_count;
	int widget_count;
	int particle_count;
	int video_widget_tex_width;
	int video_widget_tex_height;
	int video_widget_generation_freq;
	int scroll_speed;
	EGLint* config_attr;
	char compositor_cmd[PATH_MAX];
	float convolution_mat[MAX_CONV_MAT_SIZE];
	int convolution_mat_size;
	float convolution_mat_divisor;
} test_configuration_file_params;

typedef struct
{
	int execution_time;
	int w;
	int h;
	int d;
	int flag;
	int use_compositor;
	test_configuration_file_params config;
} test_execution_params;

int test_polygons(test_execution_params* params);
int test_frag_shader(test_execution_params* params);
int test_vert_shader(test_execution_params* params);
int test_texels(test_execution_params* params);
int test_fillrate(test_execution_params* params);
int test_simple_tri(test_execution_params* params);
int test_enum_glextensions(test_execution_params* params);
int test_enum_eglextensions(test_execution_params* params);
int test_enum_eglconfigs(test_execution_params* params);
int test_blitter(test_execution_params* params);

#endif // TEST_COMMON_H

