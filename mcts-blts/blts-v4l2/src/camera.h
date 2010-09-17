/* camera.c -- V4L2 commands

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

   based on freerly distributed V4L2 example

*/

#ifndef CAMERA_H_
#define CAMERA_H_


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>			/* getopt_long() */

#include <fcntl.h>			/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>		/* for videodev2.h */

#include <linux/videodev2.h>
#include <linux/fb.h>

#include <jpeglib.h>			/* data to file */

#define BLTS_WATERMARK "blts-test-asset"
// Added in 2.6.30-rc8
#ifndef V4L2_CTRL_FLAG_WRITE_ONLY
#define V4L2_CTRL_FLAG_WRITE_ONLY 0x0040
#endif

// Added in 2.6.32-rc6
#ifndef V4L2_CTRL_TYPE_STRING
#define V4L2_CTRL_TYPE_STRING 7
#endif

typedef enum
{
	IO_METHOD_NONE,
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

struct buffer
{
	void *		  start;
	size_t		  length;
	unsigned queued;
};

typedef struct
{
	char *			dev_name;  /* configurable */
	int				dev_fd;

	struct buffer 	*buffers;
	unsigned int	n_buffers;
	unsigned int	io_method;  /* configurable */

	boolean			use_progress;
	boolean			use_debug;
	boolean			use_streaming;

	unsigned use_poll;
	unsigned capture_started;

	boolean			do_snapshot;

	struct v4l2_format requested_format;  /* configurable */

	struct v4l2_capability 	capability;
	struct v4l2_cropcap 	cropcap;
	struct v4l2_crop 		crop;
	struct v4l2_format 		format;

	char fmt_a;			//used for creating filename
	char fmt_b;			// from format
	char fmt_c;
	char fmt_d;

	float frame_rate;
	float calculated_frame_rate;

} v4l2_dev_data;


#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define boolean unsigned short
#define TRUE 1
#define FALSE 0

#define LOOPS 200	// default number of loops to measure FPS
#define V4L2_DEVICE_INIT_FRAMES		50	// amount of frames that device needs to settle
#define REQUESTED_BUFFERS_COUNT 4

typedef boolean (*V4L2VideoInputFunc)(v4l2_dev_data *dev, int ret_to_check, int errno_to_check, struct v4l2_input* input);

boolean open_device (v4l2_dev_data *dev);
boolean close_device (v4l2_dev_data *dev);
boolean init_device (v4l2_dev_data *dev, int req_cam_width, int req_cam_height);
void print_aspect_ratio(int w, int h);
boolean start_capturing (v4l2_dev_data *dev);
boolean mainloop(v4l2_dev_data *dev,int loops);
boolean do_snapshot(v4l2_dev_data *dev, char * filename);
boolean mainloop_stream(v4l2_dev_data *dev,int loops, int sx, int sy, void *fb);
boolean stop_capturing (v4l2_dev_data *dev);
boolean uninit_device (v4l2_dev_data *dev);

int query_capabilites(v4l2_dev_data *dev);
int enum_controls(v4l2_dev_data *dev);
int enum_inputs(v4l2_dev_data *dev);
int enum_formats(v4l2_dev_data *dev);
boolean device_enumeration(v4l2_dev_data *dev);

int framebuffer_close(int fd, void *data, int screen_w, int screen_h);
int framebuffer_open(const char* fb_name, void** fb_addr, int* screen_w, int* screen_h);
int framebuffer_size(int screen_w, int screen_h);
int framebuffer_info(int fb, struct fb_var_screeninfo *si);

char *create_picture_filename(v4l2_dev_data *dev);
int read_jpeg_image(char *filename);

int crop_corners(v4l2_dev_data *dev, int loops);
int stream_image_xv(v4l2_dev_data *dev, int loops);

int get_framerate_fps(v4l2_dev_data *dev, float* fps);
int get_framerate(v4l2_dev_data *dev, int *numerator, int *denominator, float* fps);
int set_framerate_fps(v4l2_dev_data *dev, float* fps);
int set_framerate(v4l2_dev_data *dev, int numerator, int denominator, float* fps);

int driver_debug(v4l2_dev_data *dev);

int has_output(v4l2_dev_data *dev, int index);
int has_input(v4l2_dev_data *dev, int index);

int try_output(v4l2_dev_data *dev, int index);
int get_output(v4l2_dev_data *dev, int *index);
int try_input(v4l2_dev_data *dev, int index);
int get_input(v4l2_dev_data *dev, int *index);


boolean do_get_standards(v4l2_dev_data *dev, const int check_ret, const int check_errno, struct v4l2_input* input);
boolean do_set_standards(v4l2_dev_data *dev, const int check_ret, const int check_errno, struct v4l2_input* input);

boolean loop_through_video_inputs(v4l2_dev_data *dev, V4L2VideoInputFunc fp);

#endif /*CAMERA_H_*/
