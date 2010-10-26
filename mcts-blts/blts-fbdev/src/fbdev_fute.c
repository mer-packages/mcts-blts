/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* fbdev_fute.c -- Frame buffer functional tests

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <blts_log.h>

/* Own includes */
#include "fbdev_fute.h"
#include "blts-fbdev-defs.h"
#include "blts-fbdev-utils.h"

struct fb_fix_screeninfo glob_fb_fix_info;
struct fb_var_screeninfo glob_fb_var_info;

/* Return pointer to shared buffer with updated variable frame buffer information. Don't free. */
struct fb_var_screeninfo *
fetch_fb_var_screeninfo (int fb_fd)
{
        FUNC_ENTER();

	BLTS_TRACE("Variable info..");
	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &glob_fb_var_info) < 0) {
		BLTS_LOGGED_PERROR("Variable screen info ioctl() failed");
                FUNC_LEAVE();
		return NULL;
	}
	BLTS_TRACE("ok.\n");
        FUNC_LEAVE();
	return &glob_fb_var_info;
}

void log_print_fb_fix_screeninfo(struct fb_fix_screeninfo *fsi)
{
	assert(fsi);
	/* comments courtesy of linux/fb.h */
	BLTS_DEBUG("====================================\n"
	       "Fixed screen info\n"
	       "------------------------------------\n"
	       "id          = %s\n"                /* identification string eg "TT Builtin" */
	       "smem_start  = %lu\n"       /* Start of frame buffer mem */
	       /* (physical address) */
	       "smem_len    = %u\n"		/* Length of frame buffer mem */
	       "type        = %u\n"		/* see FB_TYPE_*		*/
	       "type_aux    = %u\n"		/* Interleave for interleaved Planes */
	       "visual      = %u\n"		/* see FB_VISUAL_*		*/
	       "xpanstep    = %u\n"		/* zero if no hardware panning  */
	       "ypanstep    = %u\n"		/* zero if no hardware panning  */
	       "ywrapstep   = %u\n"	/* zero if no hardware ywrap    */
	       "line_length = %u\n"	/* length of a line in bytes    */
	       "mmio_start  = %lu\n"	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	       "mmio_len    = %u\n"		/* Length of Memory Mapped I/O  */
	       "accel       = %u\n"		/* Indicate to driver which	*/
	       /*  specific chip/card we have	*/
	       "reserved[]  = [%u, %u, %u]\n"		/* Reserved for future compatibility */
	       ,
	       fsi->id,
	       fsi->smem_start, fsi->smem_len,
	       fsi->type, fsi->type_aux, fsi->visual,
	       fsi->xpanstep, fsi->ypanstep, fsi->ywrapstep,
	       fsi->line_length,
	       fsi->mmio_start, fsi->mmio_len,
	       fsi->accel,
	       fsi->reserved[0],fsi->reserved[1],fsi->reserved[2]);
}

void log_print_fb_var_screeninfo(struct fb_var_screeninfo *vsi)
{
	assert(vsi);
	/* comments courtesy of linux/fb.h */
	BLTS_DEBUG("====================================\n"
	       "Variable screen info\n"
	       "------------------------------------\n"
	       "xres           = %u\n"			/* visible resolution		*/
	       "yres           = %u\n"
	       "xres_virtual   = %u\n"		/* virtual resolution		*/
	       "yres_virtual   = %u\n"
	       "xoffset        = %u\n"			/* offset from virtual to visible */
	       "yoffset        = %u\n"			/* resolution			*/
	       "bits_per_pixel = %u\n"		/* guess what			*/
	       "grayscale      = %u\n"		/* != 0 Graylevels instead of colors */
	       "red            = (%u, %u, %u)\n"	/* bitfield in fb mem if true color, */
	       "green          = (%u, %u, %u)\n"	/* else only length is significant */
	       "blue           = (%u, %u, %u)\n"
	       "transp         = (%u, %u, %u)\n"	/* transparency			*/
	       "nonstd         = %u\n"			/* != 0 Non standard pixel format */
	       "activate       = %u\n"			/* see FB_ACTIVATE_*		*/
	       "height         = %u\n"			/* height of picture in mm    */
	       "width          = %u\n"			/* width of picture in mm     */
	       "accel_flags    = %u\n"		/* (OBSOLETE) see fb_info.flags */
	       /* Timing: All values in pixclocks, except pixclock (of course) */
	       "pixclock       = %u\n"		/* pixel clock in ps (pico seconds) */
	       "left_margin    = %u\n"		/* time from sync to picture	*/
	       "right_margin   = %u\n"		/* time from picture to sync	*/
	       "upper_margin   = %u\n"		/* time from sync to picture	*/
	       "lower_margin   = %u\n"
	       "hsync_len      = %u\n"		/* length of horizontal sync	*/
	       "vsync_len      = %u\n"		/* length of vertical sync	*/
	       "sync           = %u\n"			/* see FB_SYNC_*		*/
	       "vmode          = %u\n"			/* see FB_VMODE_*		*/
	       "rotate         = %u\n"			/* angle we rotate counter clockwise */
	       "reserved[]     = [%u, %u, %u, %u, %u]\n",	/* Reserved for future compatibility */
	       vsi->xres, vsi->yres, vsi->xres_virtual, vsi->yres_virtual, vsi->xoffset, vsi->yoffset,
	       vsi->bits_per_pixel, vsi->grayscale,
	       vsi->red.offset, vsi->red.length, vsi->red.msb_right,
	       vsi->green.offset, vsi->green.length, vsi->green.msb_right,
	       vsi->blue.offset, vsi->blue.length, vsi->blue.msb_right,
	       vsi->transp.offset, vsi->transp.length, vsi->transp.msb_right,
	       vsi->nonstd, vsi->activate,
	       vsi->height, vsi->width,
	       vsi->accel_flags,
	       vsi->pixclock,
	       vsi->left_margin, vsi->right_margin, vsi->upper_margin, vsi->lower_margin,
	       vsi->hsync_len, vsi->vsync_len,
	       vsi->sync, vsi->vmode, vsi->rotate,
	       vsi->reserved[0],vsi->reserved[1],vsi->reserved[2],vsi->reserved[3],vsi->reserved[4]);
}


/* -------------------------------------------------------------------------- */
/* Test cases -> */
/* -------------------------------------------------------------------------- */

/* Open frame buffer device, fetch & print info, and close the device */
int
fute_fb_open_fetch_info_close (blts_fbdev_data *data)
{
        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted right!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        /* TODO:
         * - Move to utils, might be needed in other cases too
         */
	log_print_fb_fix_screeninfo (&data->device->fixed_info);

	struct fb_var_screeninfo *vsi;

	if (!(vsi = fetch_fb_var_screeninfo (data->device->fd))) {
                BLTS_ERROR ("Error: Failed to get variable screen info!\n");
		goto ERROR;
	}

	log_print_fb_var_screeninfo (vsi);

        FUNC_LEAVE();

        return 0;

ERROR:

        FUNC_LEAVE();

        return -1;
}
