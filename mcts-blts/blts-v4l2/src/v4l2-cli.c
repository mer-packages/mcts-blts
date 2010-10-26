/* v4l2-cli.c -- V4L2 tests command line interface

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>
#include <blts_timing.h>
#include "v4l2-xvideo.h"
#include "v4l2-ioctl.h"
#include "v4l2-profiler.h"
#include "camera.h"
#include "blts-v4l2-definitions.h"
#include "v4l2-controls.h"
#include "v4l2-priority.h"
#include "v4l2-stream-quality.h"

int init_done = 0;

io_method io = IO_METHOD_MMAP;

//#define	MAIN_CAM_DEVICE 	"/dev/video0"
//#define	FRONT_CAM_DEVICE	"/dev/video1"
// maximum resolution dictating future standards
// http://en.wikipedia.org/wiki/Red_Digital_Cinema_Camera_Company
#define MAX_WIDTH			28000
#define MAX_HEIGHT			9334
#define SCREEN_WIDTH		848
#define SCREEN_HEIGHT		480

static void v4l2_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-p] [-i]",
		"  -p: Enables profiling of ioctl calls\n"
		"  -i: Save camera snapshot images to disk in control settings tests, and\n"
		"      optionally verify them (see configuration file)\n");
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "pi";
static const struct option long_opts[] =
{
	{"profiling", no_argument, NULL, 'p'},
	{"imgsave", no_argument, NULL, 'i'},
	{0,0,0,0}
};

static void app_init_once(v4l2_data *data);
static int setup_img_save_params(v4l2_data* data);

/* Parse device name parameter from variable list */
static void *variant_args_dev(struct boxed_value *args, void *user_ptr)
{
	v4l2_data* data = (v4l2_data *) user_ptr;

	if (!user_ptr || !data->device) {
		BLTS_ERROR("Error: Invalid argument in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (!args) {
		BLTS_WARNING("Warning: No arguments to process in %s:%s\n",__FILE__,__func__);
		return user_ptr;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->device->dev_name = blts_config_boxed_value_get_string(args);
	return data;
}

static void *variant_args_dev_capfmt_io(struct boxed_value *args, void *user_ptr)
{
	v4l2_data* data = variant_args_dev(args, user_ptr);
	char *work = NULL;

	if (!data) {
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args) {
		BLTS_WARNING("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return user_ptr;
	}
	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	work = blts_config_boxed_value_get_string(args);

	data->device->requested_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	data->device->requested_format.fmt.pix.pixelformat = v4l2_fourcc(work[0], work[1],
		work[2], work[3]);

	data->device->fmt_a = work[0];
	data->device->fmt_b = work[1];
	data->device->fmt_c = work[2];
	data->device->fmt_d = work[3];

	free(work);

	args = args->next;
	if (!args) {
		BLTS_WARNING("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return user_ptr;
	}
	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	work = blts_config_boxed_value_get_string(args);

	if (!strcmp(work, "readwrite")) {
		data->device->io_method = IO_METHOD_READ;
	} else if (!strcmp(work, "MMAP")) {
		data->device->io_method = IO_METHOD_MMAP;

	} else if (!strcmp(work, "userptr")) {
		data->device->io_method = IO_METHOD_USERPTR;
	} else {
		BLTS_ERROR("Error: Invalid IO method \"%s\" in %s:%s\n",work,__FILE__,__func__);
		data = NULL;
	}

	return data;
}

static void *variant_args_dev_capfmt_io_res(struct boxed_value *args, void *user_ptr)
{
	v4l2_data* data = variant_args_dev_capfmt_io(args, user_ptr);
	char *work = NULL;

	if (!data) {
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	args = args->next; /* skip capfmt */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	args = args->next; /* skip io */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	work = blts_config_boxed_value_get_string(args);

	if (sscanf(work, "%dx%d", &data->device->requested_format.fmt.pix.width,
		&data->device->requested_format.fmt.pix.height) != 2) {
		BLTS_ERROR("Error: Invalid resolution \"%s\" %s:%s\n",work,__FILE__,__func__);
		data = NULL;
	}
	free(work);

	return data;
}

static void *variant_args_dev_capfmt_io_frate(struct boxed_value *args, void *user_ptr)
{
	v4l2_data* data = variant_args_dev_capfmt_io_res(args, user_ptr);

	if (!data) {
		return NULL;
	}

	args = args->next; /* skip dev */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	args = args->next; /* skip capfmt */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}
	args = args->next; /* skip io */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	args = args->next; /* skip resolution */
	if (!args) {
		BLTS_ERROR("Error: No arguments left to process in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	if (args->type != CONFIG_PARAM_FLOAT) {
		BLTS_ERROR("Error: Type mismatch in %s:%s\n",__FILE__,__func__);
		return NULL;
	}

	data->device->frame_rate = blts_config_boxed_value_get_float(args);

	return data;
}

/* Return NULL in case of an error */
static void* v4l2_argument_processor(int argc, char **argv)
{
	int c, ret;
	v4l2_data* my_data = malloc(sizeof (*my_data));
	memset(my_data, 0, sizeof(*my_data));

	app_init_once(my_data);

	while((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch(c)
		{
		case 'p':
			my_data->flags |= CLI_FLAG_PROFILING;
			break;
		case 'i':
			my_data->flags |= CLI_FLAG_IMGSAVE;
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	ret = blts_config_declare_variable_test("Core-Camera-Open device",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Read device",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Check device controls",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Check device capabilites",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Check device formats",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Measure device stepping down resolution",
		variant_args_dev_capfmt_io,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method", "readwrite",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Stream from device to screen",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "320x240",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Measure device FPS with defined resolution",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method", "readwrite",
		CONFIG_PARAM_STRING, "camera_resolution_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Take picture as JPEG with device",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "JPEG",
		CONFIG_PARAM_STRING, "io_method", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Device enumeration",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Stream from device and crop corners",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Read from device using poll",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method", "readwrite",
		CONFIG_PARAM_STRING, "camera_resolution_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Test standard control settings",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method", "readwrite",
		CONFIG_PARAM_STRING, "camera_resolution_controltest", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Test extended control settings",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method", "readwrite",
		CONFIG_PARAM_STRING, "camera_resolution_controltest", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Check device FD priority",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Stream from device and vary frame rate",
		variant_args_dev_capfmt_io_frate,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "320x240",
		CONFIG_PARAM_FLOAT, "frame_rate", 5.0,
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Get stream quality setting",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;
	ret = blts_config_declare_variable_test("Core-Camera-Set stream quality setting to high",
		variant_args_dev_capfmt_io_res,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
		CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
		CONFIG_PARAM_STRING, "camera_resolution_stream_all", "640x480",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;


	ret = blts_config_declare_variable_test("Core-Camera-Check debug capability",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Select input and stream",
			variant_args_dev_capfmt_io_res,
			CONFIG_PARAM_STRING, "video_device", "/dev/video0",
			CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
			CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
			CONFIG_PARAM_STRING, "camera_resolution_stream_all", "320x240",
			CONFIG_PARAM_NONE);
		if (ret)
			return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Select output and stream",
			variant_args_dev_capfmt_io_res,
			CONFIG_PARAM_STRING, "video_device", "/dev/video0",
			CONFIG_PARAM_STRING, "camera_pixel_format", "YUYV",
			CONFIG_PARAM_STRING, "io_method_stream", "MMAP",
			CONFIG_PARAM_STRING, "camera_resolution_stream_all", "320x240",
			CONFIG_PARAM_NONE);
		if (ret)
			return NULL;

	ret = blts_config_declare_variable_test("Core-Camera-Test input device standards",
		variant_args_dev,
		CONFIG_PARAM_STRING, "video_device", "/dev/video0",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;


	return my_data;
}

/* user_ptr is what you returned from my_example_argument_processor */
static void v4l2_teardown(void *user_ptr)
{

	if(user_ptr)
	{
		free(user_ptr);
	}
}

/* user_ptr is what you returned from my_example_argument_processor
 * test_num Test case being run from my_example_cases, starts from 1
 * return non-zero in case of an error */
static int v4l2_case_open(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);
	if( open_device (data->device) )
	{
		LOG("Device opened\n");
		close_device(data->device);
		return 0;
	}
	else
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		close_device(data->device);
		return -1;
	}

	return 0;
}

static int v4l2_case_read(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);
	int w = MAX_WIDTH;
	int h = MAX_HEIGHT;

	if( open_device (data->device) && init_device (data->device,w, h))
	{
		LOG("Device opened\n");
		close_device(data->device);
		return 0;
	}
	else
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		close_device(data->device);
		return -1;
	}
	return 0;
}

static int v4l2_case_measure(void* user_ptr, int test_num)
{
	FUNC_ENTER();
	int max_width;
	int max_height;
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);
	if(!open_device (data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		FUNC_LEAVE();
		return -1;
	}

	if(init_device (data->device, MAX_WIDTH, MAX_HEIGHT))
	{
		BLTS_DEBUG("Maximum resolution is: %ix%i\n", data->device->format.fmt.pix.width, data->device->format.fmt.pix.height);
		LOG("Measuring FPS on maximum resolution\n");

		max_width = data->device->format.fmt.pix.width;
		max_height = data->device->format.fmt.pix.height;
		LOG("-----------------------\n");
		LOG("%ix%i resolution\n", max_width, max_height);
		LOG("-----------------------\n");

		if(!start_capturing (data->device))
			goto err;
		mainloop (data->device, LOOPS);

		stop_capturing (data->device);
		uninit_device (data->device);
		close_device (data->device);
	}
	else
	{
		LOG("Can't initialize device\n");
		goto err;
	}

	LOG("Stepping down resolutions and calculating FPS\n");

	int step_size_x, step_size_y, i, x, y;


	step_size_x = max_width / 10;
	step_size_y = max_height / 10;

	for(i=1; i<8; i++)
	{
		x = max_width - step_size_x * i;
		y = max_height - step_size_y * i;
		LOG("-----------------------\n");
		LOG("%ix%i resolution\n", x, y);
		LOG("-----------------------\n");
		if(!open_device (data->device))
			goto err;

		if(init_device (data->device, x, y))
		{
			LOG("Resolution is: %ix%i\n", x, y);
			if(!start_capturing (data->device))
				goto err;

			if(!mainloop (data->device, LOOPS))
				goto err;

			stop_capturing (data->device);
			uninit_device (data->device);
		}
		else
		{
			LOG("Can't initialize device\n");
			goto err;
		}

		close_device (data->device);
	}
	FUNC_LEAVE();
	return 0;

err:
	stop_capturing(data->device);
	uninit_device(data->device);
	close_device(data->device);
	FUNC_LEAVE();
	return -1;
}

static int v4l2_case_check_controls(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);
	int w = MAX_WIDTH;
	int h = MAX_HEIGHT;


	if( open_device (data->device) && init_device (data->device, w, h))
	{
		LOG("Device opened\n");
		if(!enum_controls(data->device))
			return -1;
		return 0;
	}
	else
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		close_device(data->device);
		return -1;
	}
	close_device(data->device);
	return 0;
}

static int v4l2_case_check_capa(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);
	int w = MAX_WIDTH;
	int h = MAX_HEIGHT;


	if( open_device (data->device) && init_device (data->device, w, h))
	{
		LOG("Device opened\n");
		if(!query_capabilites(data->device))
			return -1;
		return 0;
	}
	else
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}
	close_device(data->device);
	return 0;
}

static int v4l2_case_check_formats(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);
	int w = MAX_WIDTH;
	int h = MAX_HEIGHT;


	if( open_device (data->device) && init_device (data->device, w, h))
	{
		LOG("Device opened\n");
		if(!enum_formats(data->device))
			return -1;
		return 0;
	}
	else
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}
	close_device(data->device);
	return 0;
}


static int v4l2_case_measure_resolution(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);

	if(!open_device (data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	if(init_device (data->device, data->device->requested_format.fmt.pix.width, data->device->requested_format.fmt.pix.height))
	{
		BLTS_DEBUG("Resolution gained: %ix%i\n", data->device->format.fmt.pix.width, data->device->format.fmt.pix.height);
		if(!start_capturing (data->device))
			goto err;
		if(!mainloop(data->device, LOOPS))
			goto err;

		stop_capturing (data->device);
		uninit_device (data->device);
	}
	else
	{
		LOG("Can't initialize device\n");
		goto err;
	}

	close_device (data->device);
	return 0;

err:
	stop_capturing(data->device);
	uninit_device(data->device);
	close_device(data->device);
	return -1;

}

static int v4l2_case_take_pic(void* user_ptr, int test_num)
{
	FUNC_ENTER()
	BLTS_DEBUG("Test number %i:\n", test_num);
	v4l2_data* data = (v4l2_data*)user_ptr;

	if(!open_device (data->device))
	{
		LOGERR("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	if(init_device (data->device, data->device->requested_format.fmt.pix.width, data->device->requested_format.fmt.pix.height))
	{
		if(data->snapshot_filename != NULL)
		{
			free(data->snapshot_filename);
			data->snapshot_filename = NULL;
		}
		data->snapshot_filename = create_picture_filename(data->device);
		BLTS_DEBUG("Filename to create: '%s'\n", data->snapshot_filename);
		BLTS_DEBUG("Maximum resolution is: %ix%i\n", data->device->format.fmt.pix.width, data->device->format.fmt.pix.height);

		if(!start_capturing (data->device))
			goto err;
		if(!mainloop (data->device, 0))
			goto err;
		LOG("Taking picture %s\n", data->snapshot_filename);
		if(!do_snapshot (data->device, data->snapshot_filename))
			goto err;
		// ensure that image is actual JPEG
		if(read_jpeg_image(data->snapshot_filename))
		{
			LOG("Error, image %s is not JPEG standard.\n", data->snapshot_filename);
			return -1;
		}


		stop_capturing (data->device);
		uninit_device (data->device);
	}
	else
	{
		LOGERR("Can't initialize device\n");
		goto err;
	}

	close_device (data->device);
	FUNC_LEAVE()
	return 0;

err:

	stop_capturing(data->device);
	uninit_device(data->device);
	close_device(data->device);
	FUNC_LEAVE()
	return -1;
}

static int v4l2_case_device_enumeration(void* user_ptr, int test_num)
{
	boolean cam_ret;

	v4l2_data* data = (v4l2_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);


	cam_ret = device_enumeration(data->device);

	if (cam_ret)
		return 0;
	else
		return -1;
}


static int v4l2_run_stream(v4l2_data *data, int do_crop)
{
	int loops = 50;

	int screen_height = SCREEN_WIDTH;
	int screen_width = SCREEN_HEIGHT;

	if (!open_device(data->device))	{
			LOG("Can't open device %s\n", data->device->dev_name);
			return -1;
	}

	data->device->use_streaming = 1;
	if (init_device(data->device, screen_width, screen_height)) {
		int w = data->device->format.fmt.pix.width;
		int h = data->device->format.fmt.pix.height;
		xvideo_init(w, h, 0);

 		if (!start_capturing(data->device))
				goto err;

		if (do_crop) {
			if (!crop_corners(data->device, loops))
				goto err;
		} else {
			if (!stream_image_xv(data->device, loops))
				goto err;
		}

		stop_capturing(data->device);
		uninit_device(data->device);
	} else {
		close_device(data->device);
		LOG("Could not open device\n");
		data->device->use_streaming = 0;
		return -1;
	}
	data->device->use_streaming = 0;

	close_device(data->device);
	xvideo_deinit();

	return 0;
err:

	data->device->use_streaming = 0;
	stop_capturing(data->device);
	uninit_device(data->device);
	close_device(data->device);
	xvideo_deinit();

	return -1;

}

static int v4l2_case_stream(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	return v4l2_run_stream(data, 0);
}

static int v4l2_case_stream_and_crop(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	return v4l2_run_stream(data, 1);
}

static int v4l2_case_read_with_poll(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	if (!open_device(data->device))	{
		BLTS_ERROR("Can't open device %s\n",
			data->device->dev_name);
		return -1;
	}

	if (!init_device(data->device,
		data->device->requested_format.fmt.pix.width,
		data->device->requested_format.fmt.pix.height))	{
		BLTS_ERROR("Can't initialize device\n");
		goto err;
	}

	LOG("Resolution used: %ix%i\n",
		data->device->format.fmt.pix.width,
		data->device->format.fmt.pix.height);

	if (!start_capturing(data->device))
		goto err;

	LOG("Starting polled reads\n");
	if (!mainloop(data->device, LOOPS))
		goto err;

	stop_capturing(data->device);
	uninit_device(data->device);
	if (!close_device(data->device))
		return -1;

	return 0;

err:
	stop_capturing(data->device);
	uninit_device(data->device);
	close_device(data->device);

	return -1;
}

static int setup_img_save_params(v4l2_data* data)
{
	if (data->flags & CLI_FLAG_IMGSAVE)
	{
		blts_config_get_value_string("image_save_path", &data->img_save_path);
		blts_config_get_value_string("image_verify_tool", &data->img_verify_tool);

		if (data->img_save_path)
		{
			struct stat pathstat;
			if (stat(data->img_save_path, &pathstat) == -1)
			{
				BLTS_ERROR("Cannot access image save path %s\n", data->img_save_path);
				return 0;
			}
			else if (!S_ISDIR(pathstat.st_mode))
			{
				BLTS_ERROR("Image save path %s is not a directory\n", data->img_save_path);
				return 0;
			}
			else
			{
				LOG("Saving images to %s\n", data->img_save_path);
			}
		}
		else
		{
			BLTS_ERROR("Image saving requested but \"image_save_path\" not defined in config\n");
			return 0;
		}

		if (data->img_verify_tool)
		{
			struct stat pathstat;
			if (stat(data->img_verify_tool, &pathstat) == -1)
			{
				BLTS_ERROR("Cannot access image verify tool %s\n", data->img_verify_tool);
				return 0;
			}
			else if (!S_ISREG(pathstat.st_mode) || (pathstat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0)
			{
				BLTS_ERROR("Image verify tool %s is not an executable file\n", data->img_verify_tool);
				return 0;
			}
			else
			{
				LOG("Using %s to verify images\n", data->img_verify_tool);
			}
		}
		else
		{
			LOG("\"image_verify_tool\" not defined in config, verification disabled\n");
		}
	}

	return 1;
}

static int v4l2_case_test_controls(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;

	BLTS_DEBUG("Test number %i:\n", test_num);

	if (!setup_img_save_params(data))
		return -1;

	if (!open_device(data->device))
	{
		BLTS_ERROR("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	if (!init_device(data->device,
		data->device->requested_format.fmt.pix.width,
		data->device->requested_format.fmt.pix.height))
	{
		BLTS_ERROR("Can't initialize device %s\n", data->device->dev_name);
		close_device(data->device);
		return -1;
	}

	int res = 0;
	if (test_num == CORE_TEST_STD_CONTROLS)
	{
		LOG("Starting standard control test\n");
		if (!test_std_controls(data))
			res = -1;
	}
	else if (test_num == CORE_TEST_EXT_CONTROLS)
	{
		LOG("Starting extended control test\n");
		if (!test_ext_controls(data))
			res = -1;
	}

	uninit_device(data->device);
	close_device(data->device);

	return res;
}

static int test_frame_rate(v4l2_dev_data *dev, float* fps, float* calculated_fps)
{
	int loops = 100;
	int screen_height = SCREEN_WIDTH;
	int screen_width = SCREEN_HEIGHT;

	dev->use_streaming = 1;

	if (init_device(dev, screen_width, screen_height))
	{
		xvideo_init(dev->format.fmt.pix.width, dev->format.fmt.pix.height, 0);

		if (!start_capturing(dev))
			goto err;

		if (!stream_image_xv(dev, loops))
			goto err;

		*calculated_fps = dev->calculated_frame_rate;

		stop_capturing(dev);
		uninit_device(dev);
		xvideo_deinit();
	}
	else
	{
		close_device(dev);
		LOG("Could not open device\n");
		dev->use_streaming = 0;
		return -1;
	}

	return 0;
err:

	dev->use_streaming = 0;
	stop_capturing(dev);
	uninit_device(dev);
	xvideo_deinit();

	return -1;

}

static int v4l2_case_vary_frame_rate(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	int ret = 0;

	float initial_fps = 0.0;
	float initial_cfps = 0.0; /* calculated fps for original value */

	float target_fps = data->device->frame_rate;
	float target_cfps = 0.0; /* calculated fps for new value */

	boolean reset = FALSE;

	if (!open_device(data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	/* find out initial frame rate */
	if(get_framerate_fps(data->device, &initial_fps) != 0)
	{
		LOG("Can't get initial fps\n");
		goto err;
	}

	/* test frame rate with initial values */
	if(test_frame_rate(data->device, &initial_fps, &initial_cfps) != 0)
	{
		LOG("Can't test initial frame rate\n");
		goto err;
	}

	/* try to set new frame rate values */
	if(set_framerate_fps(data->device, &target_fps) != 0)
	{
		LOG("Can't set new frame rate\n");
		reset = TRUE;
		goto err;
	}

	/* test frame rate with new values */
	if(test_frame_rate(data->device, &target_fps, &target_cfps) != 0)
	{
		LOG("Can't test new frame rate\n");
		reset = TRUE;
		goto err;
	}

	/* sanity check for test times */
	if(initial_fps < target_fps)
		ret = ((initial_cfps < target_cfps))?0:-1;
	else if (initial_fps > target_fps)
		ret = ((initial_cfps > target_cfps))?0:-1;
	else
		ret = 0;

	/* reset initial frame rate */
	set_framerate(data->device, 0, 0, &initial_fps);
	close_device(data->device);

	return ret;
err:
	if(reset)
		set_framerate(data->device, 0, 0, &initial_fps);
	close_device(data->device);
	return -1;
}


static int v4l2_case_debug_capability(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	int ret = 0;
	if (!open_device(data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}
	if(start_syslog_capture())
	{
		BLTS_ERROR("Can't initiate syslog capture\n");
		goto err;
	}

	ret = driver_debug(data->device);

	if(errno == EINVAL)
	{
		LOG("Driver returned EINVAL, and does not support driver status logging\n");
		ret = 0;
	}

	if(end_syslog_capture())
	{
		BLTS_ERROR("Common library can't stop syslog capture\n");
	}

	close_device(data->device);
	return ret;
err:
	close_device(data->device);
	return -1;
}

static int v4l2_select_output_stream(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	int i=0;

	if (!open_device(data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	while(1)
	{
		errno = 0;
		if(-1 == has_output(data->device, i) && !i)
			goto force_testing;
		else if(errno == EINVAL)
			break;
		else if (errno)
			goto err;

		if(-1 == try_output(data->device, i))
			if(errno != EINVAL)
				goto err;
		if(-1 == get_output(data->device, &i))
			if(errno != EINVAL)
				goto err;
		i++;
	}

	close_device(data->device);
	return 0;

force_testing:
	LOG("No output device available, force testing IOCTL\n");
	if(try_output(data->device, 0))
		if(errno != EINVAL)
			goto err;
	if(get_output(data->device, &i))
		if(errno != EINVAL)
			goto err;
	return 0;

err:
	close_device(data->device);
	return -1;
}

static int v4l2_select_input_stream(void* user_ptr, int test_num)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	int i=0;

	if (!open_device(data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}

	while(1)
	{
		errno = 0;
		if(has_input(data->device, i) && !i)
			goto force_testing;
		else if(errno == EINVAL)
			break;
		else if (errno)
			goto err;

		if(!try_input(data->device, i))
		{
			LOG("Trying to stream from found input %i\n", i);
			close_device(data->device);
			if(-1 == v4l2_run_stream(data, 0))
				goto err;
			if (!open_device(data->device))
			{
				LOG("Error opening device after streaming\n");
				return -1;
			}
		}
		if(get_input(data->device, &i))
		{
			BLTS_ERROR("Can't get current input device\n");
			goto err;
		}
		i++;
	}

	close_device(data->device);
	return 0;


force_testing:
	LOG("No input device available, force testing IOCTL\n");
	if(try_input(data->device, 0))
		if(errno != EINVAL)
			goto err;
	if(get_input(data->device, &i))
		if(errno != EINVAL)
			goto err;
	return 0;

err:
	close_device(data->device);
	return -1;
}

static int v4l2_case_test_standards(void* user_ptr, int test_num)
{
	boolean ret_get;
	boolean ret_set;

	v4l2_data* data = (v4l2_data*)user_ptr;
	BLTS_DEBUG("Test number %i:\n", test_num);

	if (!open_device(data->device))
	{
		LOG("Can't open device %s\n", data->device->dev_name);
		return -1;
	}
	LOG("**Get standards**\n");
	ret_get = loop_through_video_inputs(data->device, do_get_standards);

	LOG("**Set standards**\n");
	ret_set = loop_through_video_inputs(data->device, do_set_standards);


	BLTS_DEBUG("ret_get %i ret_set %i\n", ret_get, ret_set);
	close_device(data->device);

	if(!ret_get || !ret_set)
		return -1;

	return 0;

}

static void app_deinitialize(void * user_ptr)
{
	v4l2_data* data = (v4l2_data*)user_ptr;
	free(data->snapshot_filename);
	free(data->img_verify_tool);
	free(data->img_save_path);
	free(data->device);
}

static void app_init_once(v4l2_data *data)
{
	static int init_done = 0;

	if (init_done)
		return;

	init_done = 1;

	data->device = (v4l2_dev_data *)malloc(sizeof(v4l2_dev_data));
	memset(data->device, 0, sizeof(v4l2_dev_data));
	if (data->device==NULL)
	{
		LOG("Not enough memory to allocate camera data\n");
		exit(1);
	}
	data->snapshot_filename = NULL;
	data->img_verify_tool	= NULL;
	data->img_save_path	= NULL;
	data->device->dev_name = NULL;
	data->device->buffers  = NULL;
	data->device->dev_fd   = -1;


	//TODO: necessary initilization here
}

static int v4l2_run_case(void* user_ptr, int test_num)
{
	int ret = 0;
	v4l2_data* data = (v4l2_data*)user_ptr;

	if(data->flags&CLI_FLAG_PROFILING)
	{
		ioctl_start_profiling();
	}

	switch (test_num)
	{
	case CORE_OPEN_DEVICE: ret = v4l2_case_open(user_ptr, test_num); break;
	case CORE_READ_DEVICE: ret = v4l2_case_read(user_ptr, test_num); break;
	case CORE_CHECK_CONTROLS: ret = v4l2_case_check_controls(user_ptr, test_num); break;
	case CORE_CHECK_CAPABILITIES: ret = v4l2_case_check_capa(user_ptr, test_num); break;
	case CORE_CHECK_FORMATS: ret = v4l2_case_check_formats(user_ptr, test_num); break;
	case CORE_MEASURE_DEVICE: ret = v4l2_case_measure(user_ptr, test_num); break;
	case CORE_STREAM_TO_SCREEN: ret = v4l2_case_stream(user_ptr, test_num); break;
	case CORE_MEASURE_FPS: ret = v4l2_case_measure_resolution(user_ptr, test_num); break;
	case CORE_TAKE_JPEG: ret = v4l2_case_take_pic(user_ptr, test_num); break;
	case CORE_DEVICE_ENUMERATION: ret = v4l2_case_device_enumeration(user_ptr, test_num); break;
	case CORE_STREAM_AND_CROP: ret = v4l2_case_stream_and_crop(user_ptr, test_num); break;
	case CORE_READ_WITH_POLL: ret = v4l2_case_read_with_poll(user_ptr, test_num); break;
	case CORE_TEST_STD_CONTROLS: ret = v4l2_case_test_controls(user_ptr, test_num); break;
	case CORE_TEST_EXT_CONTROLS: ret = v4l2_case_test_controls(user_ptr, test_num); break;
	case CORE_CHECK_PRIORITY: ret = v4l2_case_priority (user_ptr, test_num); break;
	case CORE_VARY_FRAME_RATE: ret = v4l2_case_vary_frame_rate(user_ptr, test_num); break;
	case CORE_DEBUG_CAPABILITY: ret = v4l2_case_debug_capability(user_ptr, test_num); break;
    case CORE_GET_STREAM_QUALITY: ret = v4l2_case_get_stream_quality (user_ptr, test_num); break;
    case CORE_SET_STREAM_QUALITY: ret = v4l2_case_set_stream_quality (user_ptr, test_num); break;
	case CORE_SELECT_INPUT_STREAM: ret = v4l2_select_input_stream(user_ptr, test_num); break;
	case CORE_SELECT_OUTPUT_STREAM: ret = v4l2_select_output_stream(user_ptr, test_num); break;
	case CORE_TEST_STANDARDS: ret = v4l2_case_test_standards(user_ptr, test_num); break;
	default: LOG("Not supported case number%d\n", test_num);
	}

	if(data->flags&CLI_FLAG_PROFILING)
	{
		ioctl_print_profiling_data();
		ioctl_stop_profiling();
	}
	// free allocated memory
	app_deinitialize(data);

	return ret;
}


static blts_cli_testcase v4l2_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity */
	{ "Core-Camera-Open device", v4l2_run_case, 5000 },
	{ "Core-Camera-Read device", v4l2_run_case, 5000 },
	{ "Core-Camera-Check device controls", v4l2_run_case, 5000 },
	{ "Core-Camera-Check device capabilites", v4l2_run_case, 5000 },
	{ "Core-Camera-Check device formats", v4l2_run_case, 5000 },
	{ "Core-Camera-Measure device stepping down resolution", v4l2_run_case, 300000 },
	{ "Core-Camera-Stream from device to screen", v4l2_run_case, 60000 },
	{ "Core-Camera-Measure device FPS with defined resolution", v4l2_run_case, 60000 },
	{ "Core-Camera-Take picture as JPEG with device", v4l2_run_case, 60000 },
	{ "Core-Camera-Device enumeration", v4l2_run_case, 60000 },
	{ "Core-Camera-Stream from device and crop corners", v4l2_run_case, 60000 },
	{ "Core-Camera-Read from device using poll", v4l2_run_case, 60000 },
	{ "Core-Camera-Test standard control settings", v4l2_run_case, 1800000 },
	{ "Core-Camera-Test extended control settings", v4l2_run_case, 1800000 },
	{ "Core-Camera-Check device FD priority", v4l2_run_case, 60000 },
	{ "Core-Camera-Stream from device and vary frame rate", v4l2_run_case, 60000 },
	{ "Core-Camera-Get stream quality setting", v4l2_run_case, 60000 },
	{ "Core-Camera-Set stream quality setting to high", v4l2_run_case, 60000 },
	{ "Core-Camera-Check debug capability", v4l2_run_case, 60000 },
	{ "Core-Camera-Select input and stream", v4l2_run_case, 60000 },
	{ "Core-Camera-Select output and stream", v4l2_run_case, 60000 },
	{ "Core-Camera-Test input device standards", v4l2_run_case, 60000 },

	BLTS_CLI_END_OF_LIST
};

static blts_cli v4l2_cli =
{
	.test_cases = v4l2_cases,
	.log_file = "blts_v4l2_log.txt",
	.blts_cli_help = v4l2_help,
	.blts_cli_process_arguments = v4l2_argument_processor,
	.blts_cli_teardown = v4l2_teardown
};

int main(int argc, char **argv)
{
	/* You can do something here if you wish */
	return blts_cli_main(&v4l2_cli, argc, argv);
}

