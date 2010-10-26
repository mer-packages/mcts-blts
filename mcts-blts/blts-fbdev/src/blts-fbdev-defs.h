/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-defs.h -- Common definitions for frame buffer functional tests

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

#ifndef BLTS_FBDEV_DEFS_H_
#define BLTS_FBDEV_DEFS_H_

#include <linux/fb.h>

/* Device specific data */
typedef struct {
        char   *name;
        int     fd;
        struct  fb_fix_screeninfo fixed_info;
        void   *buffer; /* Device buffer, mmaped and unmapped */
} blts_fbdev_device;

/* Common test client data */
typedef struct {
        blts_fbdev_device *device;
        char   *backlight_subsystem;
        int     minimum_light;
        int     maximum_light;
} blts_fbdev_data;

/* Test case list */
enum {
        BLTS_FBDEV_CASE_OPEN_CLOSE = 1,
        BLTS_FBDEV_CASE_BLANKING,
        BLTS_FBDEV_CASE_BACKLIGHT_LIMITS,
        BLTS_FBDEV_CASE_BACKLIGHT_LINEAR,
        BLTS_FBDEV_CASE_BACKLIGHT_LOGARITHMIC,
};

#endif /* BLTS_FBDEV_DEFS_H_ */
