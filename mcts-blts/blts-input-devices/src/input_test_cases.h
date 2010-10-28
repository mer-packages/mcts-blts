/* input_test_cases.h -- input device test cases

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

#ifndef INPUT_TEST_CASES_H
#define INPUT_TEST_CASES_H

#include <stdio.h>
#include <limits.h>

#define MAX_DEVICES 32

struct key_mapping {
	char device[PATH_MAX];
	unsigned code;
	char description[256];
};

struct input_data {
	unsigned num_devices;
	char devices[MAX_DEVICES][PATH_MAX];
	char pointer_device[PATH_MAX];
	struct key_mapping key;
	int no_grab;
	int swap_xy;
	int scr_width;
	int scr_height;
};

int test_device(const char *device);

int input_enumerate_test(void* user_ptr, int test_num);
int input_key_test(void *user_ptr, int test_num);
int input_pointer_test(void *user_ptr, int test_num);
int input_single_touch_test(void *user_ptr, int test_num);
int input_multi_touch_test(void *user_ptr, int test_num);

#endif /* INPUT_TEST_CASES_H */

