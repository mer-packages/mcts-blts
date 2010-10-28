/* fbdev_fute.h -- Frame buffer functional tests

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

#ifndef FBDEV_FUTE_H
#define FBDEV_FUTE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <linux/fb.h>

#include "blts-fbdev-defs.h"

struct fb_fix_screeninfo *fetch_fb_fix_screeninfo(int fb_fd);
struct fb_var_screeninfo *fetch_fb_var_screeninfo(int fb_fd);
void log_print_fb_fix_screeninfo(struct fb_fix_screeninfo *fsi);
void log_print_fb_var_screeninfo(struct fb_var_screeninfo *vsi);
int fute_fb_open_fetch_info_close (blts_fbdev_data *data);

#endif /* FBDEV_FUTE_H */
