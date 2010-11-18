/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* v4l2-priority.c -- V4L2 device (FD) priority tests

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
#include <linux/videodev2.h> /* v4l2 definitions */

/* Project includes */
#include <blts_log.h>

/* Own includes */
#include "v4l2-xvideo.h"
#include "v4l2-priority.h"
#include "blts-v4l2-definitions.h"
#include "camera.h"
#include "v4l2-ioctl.h"

/* Own definitions */
#define SCREEN_WIDTH		848
#define SCREEN_HEIGHT		480

/* Private func prototypes */


/* Private funcs */


/* Public funcs */

/* Base priority case, test if it even can be set */
int
v4l2_case_priority (v4l2_data *user_data, int test_num)
{
        enum v4l2_priority priority = V4L2_PRIORITY_UNSET;
        int ret = -1;

        BLTS_DEBUG ("Test number %i:\n", test_num);

        if (!open_device (user_data->device)) {
                BLTS_DEBUG("Couldn't open device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

	user_data->device->use_streaming = 1;
        if (!init_device (user_data->device, SCREEN_WIDTH, SCREEN_HEIGHT)) {
                BLTS_DEBUG("Couldn't init device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

        int w = user_data->device->format.fmt.pix.width;
        int h = user_data->device->format.fmt.pix.height;
        xvideo_init (w, h, 0);

        if (!start_capturing (user_data->device)) {
                BLTS_DEBUG("Failed to capture from device\n");
                ret = -1;
                goto ERROR;
        }

        if (!stream_image_xv (user_data->device, 50)) {
                ret = -1;
                BLTS_DEBUG("Failed to stream image\n");
                goto ERROR;
        }

        ret = ioctl_VIDIOC_G_PRIORITY (
                user_data->device->dev_fd, &priority, 0);

        stop_capturing (user_data->device);

        if (ret < 0) {
                BLTS_DEBUG("Failed to get priority: %i\n", ret);
                goto ERROR;
        }

        BLTS_DEBUG("Priority: %i\n", priority);

        if (priority == V4L2_PRIORITY_UNSET) {
                BLTS_DEBUG("Priority is still unset\n");
                ret = -1;
                goto ERROR;
        }

        ret = 0;

ERROR:
	xvideo_deinit ();
        uninit_device (user_data->device);
        close_device (user_data->device);

        return ret;
}
