/* xvideo_test_cases.h -- XVideo test cases

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

#ifndef XVIDEO_TEST_CASES_H
#define XVIDEO_TEST_CASES_H

#include <stdio.h>

int xvideo_presence_check(void* user_ptr, int test_num);
int xvideo_enumerate_adapters(void* user_ptr, int test_num);
int xvideo_putimage(void* user_ptr, int test_num);
int xvideo_shmputimage(void* user_ptr, int test_num);
int xvideo_querybestsize(void* user_ptr, int test_num);
int xvideo_notify_events_test(void* user_ptr, int test_num);

#endif /* XVIDEO_TEST_CASES_H */

