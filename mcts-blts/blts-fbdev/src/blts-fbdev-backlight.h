/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-backlight.h -- Framebuffer backlight tests

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

#ifndef BLTS_FBDEV_BACKLIGHT_H_
#define BLTS_FBDEV_BACKLIGHT_H_

/* Verify limit values based off config */
int
blts_fbdev_case_backlight_verify (void *test_data, int test_num);

/* Linearly tests different backlight values */
int
blts_fbdev_case_backlight_linear (void *test_data, int test_num);

/* Logarithmically tests different backlight values */
int
blts_fbdev_case_backlight_logarithmic (void *test_data, int test_num);

#endif /* BLTS_FBDEV_BLANKING_H_ */
