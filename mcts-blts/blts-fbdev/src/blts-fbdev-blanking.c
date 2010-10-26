/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-blanking.c -- Framebuffer blanking tests

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
#include <sys/ioctl.h>
#include <unistd.h>

/* BLTS common */
#include <blts_log.h>

/* Own includes */
#include "blts-fbdev-blanking.h"
#include "blts-fbdev-utils.h"

/* Tests blanking on different values */
int
blts_fbdev_case_blanking (blts_fbdev_data *data)
{
        int blank_level;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        blank_level = FB_BLANK_UNBLANK;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to unblank!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_NORMAL;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to normal!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_UNBLANK;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to unblank!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_VSYNC_SUSPEND;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror (
                        "Failed to set blank level to vsync suspend!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_UNBLANK;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to unblank!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_HSYNC_SUSPEND;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror (
                        "Failed to set blank level to hsync suspend!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_UNBLANK;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to unblank!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_POWERDOWN;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to powerdown!\n");
                goto ERROR;
        }

        sleep (2);

        blank_level = FB_BLANK_UNBLANK;
        BLTS_TRACE ("Blank level %d\n", blank_level);

        if (ioctl (data->device->fd, FBIOBLANK, blank_level) < 0) {
                logged_perror ("Failed to set blank level to unblank!\n");
                goto ERROR;
        }

        FUNC_LEAVE();

        blts_fbdev_close (data->device);

        return 0;

ERROR:

        FUNC_LEAVE();

        blts_fbdev_close (data->device);

        return -1;
}
