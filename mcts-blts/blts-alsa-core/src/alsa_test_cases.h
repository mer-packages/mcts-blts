/* alsa_test_cases.h -- ALSA core test cases

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

#ifndef ALSA_TEST_CASES_H
#define ALSA_TEST_CASES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alsa_config.h"
#include "alsa_util.h"

#define CLI_FLAG_PROFILING 1

typedef struct
{
	int num_pcm_params;
	pcm_params* params[MAX_STREAMS];
	alsa_control_settings** control_ref;
	alsa_control_settings** def_control_ref;
	v4l_tuner_settings tuner;
	bluetooth_audio_settings btaudio;
} testcase_params;

typedef struct
{
	unsigned int flags;
	testcase_params* testcases;
	alsa_configuration config;
} alsa_cli_params;

int alsa_presence_check(void* user_ptr, int test_num);
int alsa_open_close_pcm(void* user_ptr, int test_num);
int alsa_open_close_timer(void* user_ptr, int test_num);
int alsa_open_close_controls(void* user_ptr, int test_num);
int alsa_enumerate_control_elements(void* user_ptr, int test_num);
int alsa_open_close_hwdeps(void* user_ptr, int test_num);
int alsa_play_rec_pcm(void* user_ptr, int test_num);
int alsa_power_management(void* user_ptr, int test_num);
int alsa_enumerate_ctl_devices(void* user_ptr, int test_num);
int alsa_add_remove_ctl_element(void* user_ptr, int test_num);

#endif /* ALSA_TEST_CASES_H */

