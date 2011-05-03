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

 test_common.h -- Common definitions
 Author: 
     Zheng Kui (kui.zheng@intel.com)
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
	int* config_attr;
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

int test_enum_eglextensions(test_execution_params* params);
int test_enum_eglconfigs(test_execution_params* params);
int test_transf(test_execution_params* params);
int test_stroke(test_execution_params* params);
int test_fill(test_execution_params* params);
int test_lingrad(test_execution_params* params);
int test_radialgrad(test_execution_params* params);
int test_clear(test_execution_params* params);

#endif // TEST_COMMON_H

