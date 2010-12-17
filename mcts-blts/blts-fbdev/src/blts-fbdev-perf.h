/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-perf.h -- Frame buffer measurements

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

#ifndef BLTS_FBDEV_PERF_H
#define BLTS_FBDEV_PERF_H

/* Test cases */
int
blts_fbdev_case_one_pixel_read (void *test_data, int test_num);

int
blts_fbdev_case_one_pixel_write (void *test_data, int test_num);

int
blts_fbdev_case_one_pixel_readwrite (void *test_data, int test_num);

int
blts_fbdev_case_buffer_read (void *test_data, int test_num);

int
blts_fbdev_case_buffer_write (void *test_data, int test_num);

int
blts_fbdev_case_buffer_modify (void *test_data, int test_num);

#endif /* BLTS_FBDEV_PERF_H */
