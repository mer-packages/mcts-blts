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

/* System includes */
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* BLTS common */
#include <blts_log.h>

/* Own includes */
#include "blts-fbdev-utils.h"
#include "blts-fbdev-defs.h"

/* Public functions */

/* Open and init a device */
int
blts_fbdev_init (blts_fbdev_device *device)
{
        int fd = -1;

        FUNC_ENTER();

        if (!device || !device->name) {
                BLTS_ERROR ("No device name given in!\n");
                goto ERROR;
        }

        BLTS_TRACE ("Open...");

        if ((fd = open (device->name, O_RDWR)) < 0) {
                logged_perror ("Error opening frame buffer device\n");
                goto ERROR;
        }

        device->fd = fd;
        BLTS_TRACE ("ok.\n");

        BLTS_TRACE ("Getting fixed info...\n");

        if (!blts_fbdev_get_fixed_info (device)) {
                BLTS_ERROR ("Failed to get fixed info!\n");
                goto ERROR;
        }

        BLTS_TRACE ("ok.\n");
        char* buf = 0;

        BLTS_TRACE ("Mapping framebuffer device...\n");

        if ((buf = mmap (0, device->fixed_info.smem_len,
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))
            == MAP_FAILED) {
                logged_perror ("Framebuffer mmap() failed\n");
                goto ERROR;
        }

        BLTS_TRACE ("ok.\n");
        device->buffer = buf;

        FUNC_LEAVE();

        return fd;

ERROR:

        FUNC_LEAVE();

        if (device) {
                blts_fbdev_close (device);
        }

        return 0;
}

/* unmap & close the frame buffer device */
/* Close a device */
void
blts_fbdev_close (blts_fbdev_device *device)
{
        FUNC_ENTER();

        if (!device) {
                BLTS_TRACE ("No device, nothing to close\n");
                goto DONE;
        }

        if (device->buffer) {
                munmap (device->buffer, device->fixed_info.smem_len);
                device->buffer = NULL;
        }

        if (device->fd > 0) {
                close (device->fd);
                device->fd = 0;
        }

DONE:
        FUNC_LEAVE();
}

/* Fetches fixed info of a device */
int
blts_fbdev_get_fixed_info (blts_fbdev_device *device)
{
        FUNC_ENTER();

        if (!device || device->fd <= 0) {
                BLTS_ERROR ("Error: Device is not initted right\n");
                goto ERROR;
        }

	BLTS_TRACE ("Fetching info...\n");

        /* TODO: Separate ioctl module? */
	if (ioctl (device->fd, FBIOGET_FSCREENINFO, &device->fixed_info) < 0) {
		logged_perror ("Fixed screen info ioctl() failed");
                goto ERROR;
	}

        BLTS_TRACE ("ok.\n");

        FUNC_LEAVE();

        return 1;

ERROR:

        FUNC_LEAVE();

        return 0;
}
