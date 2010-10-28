/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-utils.h -- Utils for manipulating the framebuffer device

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

#ifndef BLTS_FBDEV_UTILS_H_
#define BLTS_FBDEV_UTILS_H_

/* System includes */
#include <linux/fb.h>

/* Own includes */
#include "blts-fbdev-defs.h"

/* Open and init a device */
int
blts_fbdev_init (blts_fbdev_device *device);

/* Close a device */
void
blts_fbdev_close (blts_fbdev_device *device);

/* Fetches fixed info of a device */
int
blts_fbdev_get_fixed_info (blts_fbdev_device *device);

#endif /* BLTS_FBDEV_UTILS_H_ */
