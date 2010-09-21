/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* v4l2-stream-quality.c -- Stream quality setting test

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
#include "v4l2-stream-quality.h"
#include "blts-v4l2-definitions.h"
#include "camera.h"
#include "v4l2-ioctl.h"
#include "v4l2-xvideo.h"

/* Own definitions */
#define SCREEN_WIDTH		848
#define SCREEN_HEIGHT		480

/* Private func prototypes */


/* Private funcs */
static int
_get_high_quality_on (v4l2_dev_data *dev)
{
        int ret = -1;
        struct v4l2_streamparm params;

	memset (&params, 0, sizeof (params));
	params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl_VIDIOC_G_PARM (dev->dev_fd, &params, 0);

	if (ret < 0) {
                LOG ("Failed to get capture params from device!\n");
		return ret;
	}

        if (params.parm.capture.capturemode & V4L2_MODE_HIGHQUALITY) {
                LOG ("High quality mode on.\n");
                return 1;
        }

        LOG ("High quality mode off.\n");
        return 0;
}

static int
_set_high_quality_on (v4l2_dev_data *dev, int status)
{
        int ret = -1;
        struct v4l2_streamparm params;

	memset (&params, 0, sizeof (params));
	params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl_VIDIOC_G_PARM (dev->dev_fd, &params, 0);

	if (ret < 0) {
		LOG ("Failed to get capture params!\n");
		return ret;
	}

        if (status > 0) {
                LOG ("Setting high quality mode on.\n");
                params.parm.capture.capturemode |= V4L2_MODE_HIGHQUALITY;
        } else {
                LOG ("Setting high quality mode off.\n");
                params.parm.capture.capturemode &= ~V4L2_MODE_HIGHQUALITY;
        }

        ret = ioctl_VIDIOC_S_PARM (dev->dev_fd, &params, 0);

        if (ret < 0) {
                LOG ("Failed to set capture params!\n");
                return ret;
        }

        return 0;
}

/* Public funcs */

/* Set up a stream and get a default quality setting */
int
v4l2_case_get_stream_quality (v4l2_data *user_data, int test_num)
{
        int ret = -1;

        BLTS_DEBUG ("Test number %i:\n", test_num);

        if (!open_device (user_data->device)) {
                LOG ("Couldn't open device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

	user_data->device->use_streaming = 1;
        if (!init_device (user_data->device, SCREEN_WIDTH, SCREEN_HEIGHT)) {
                LOG ("Couldn't init device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

        int w = user_data->device->format.fmt.pix.width;
        int h = user_data->device->format.fmt.pix.height;
        xvideo_init (w, h, 0);

        if (!start_capturing (user_data->device)) {
                LOG ("Failed to capture from device\n");
                ret = -1;
                goto ERROR;
        }

        if (_get_high_quality_on (user_data->device) < 0) {
                ret = -1;
                LOG ("Failed to get stream quality mode!\n");
                goto ERROR;
        }

        if (!stream_image_xv (user_data->device, 50)) {
                ret = -1;
                LOG ("Failed to stream image\n");
                goto ERROR;
        }

        ret = 0;

ERROR:
        stop_capturing (user_data->device);
	xvideo_deinit ();
        uninit_device (user_data->device);
        close_device (user_data->device);

        return ret;
}

/* Set up a stream and set a quality setting, and verify it's used */
int
v4l2_case_set_stream_quality (v4l2_data *user_data, int test_num)
{
        int ret = -1;

        BLTS_DEBUG ("Test number %i:\n", test_num);

        if (!open_device (user_data->device)) {
                LOG ("Couldn't open device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

	user_data->device->use_streaming = 1;
        if (!init_device (user_data->device, SCREEN_WIDTH, SCREEN_HEIGHT)) {
                LOG ("Couldn't init device %s\n", user_data->device->dev_name);
                goto ERROR;
        }

        int w = user_data->device->format.fmt.pix.width;
        int h = user_data->device->format.fmt.pix.height;
        xvideo_init (w, h, 0);

        if (_set_high_quality_on (user_data->device, 1) < 0) {
                ret = -1;
                LOG ("Failed to get stream quality mode!\n");
                goto ERROR;
        }

        if (!start_capturing (user_data->device)) {
                LOG ("Failed to capture from device\n");
                ret = -1;
                goto ERROR;
        }

        if (!stream_image_xv (user_data->device, 50)) {
                ret = -1;
                LOG ("Failed to stream image\n");
                goto ERROR;
        }

        ret = _get_high_quality_on (user_data->device);

        if (ret < 1) {
                ret = -1;
                LOG ("Device doesn't report a high quality mode!\n");
                goto ERROR;
        }

        ret = 0;

ERROR:
        stop_capturing (user_data->device);
	xvideo_deinit ();
        uninit_device (user_data->device);
        close_device (user_data->device);

        return ret;
}
