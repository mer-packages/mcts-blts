/* v4l2-controls.c -- V4L2 control tests

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

#define _GNU_SOURCE

#include "camera.h"
#include "v4l2-ioctl.h"
#include "blts-v4l2-definitions.h"
#include "v4l2-controls.h"

#include <string.h>

#include <blts_log.h>
#include <blts_params.h>

static int test_control(v4l2_data* data, int ctlid);
static int validate_control_values(v4l2_data* data, const unsigned char* ctlname, __s32 min, __s32 max);
static int test_configured_values(v4l2_data* data, const unsigned char* ctlname, int ctlid);
static int test_control_value(v4l2_data* data, const unsigned char* ctlname, int ctlid, __s32 value);
static int test_control_menu_values(v4l2_data* data, const unsigned char* ctlname, int ctlid, __s32 minvalue, __s32 maxvalue);
static int set_control_value(v4l2_dev_data* dev, int ctlid, __s32 value);
static int get_control_value(v4l2_dev_data* dev, int ctlid, __s32* value);

static void create_picture_filename_for_ctlvalue(char* filename, const v4l2_data* data, const unsigned char* ctlname, __s32 ctlvalue);
static int take_snapshot(v4l2_dev_data* dev, char* filename);
static int verify_image(v4l2_data* data, const char* filename, const unsigned char* ctlname, int ctlid, __s32 value);
static void escape_control_name(const unsigned char* name, unsigned char* escaped_name);

/**
 * Test standard camera controls
 */
int test_std_controls(v4l2_data* data)
{
	int test_result = 1;

	int ctlid;
	for (ctlid = V4L2_CID_BASE; ctlid < V4L2_CID_LASTP1; ctlid++)
		test_result &= test_control(data, ctlid);

	return test_result;
}

/**
 * Test extended camera controls
 */
int test_ext_controls(v4l2_data* data)
{
	int test_result = 1;

	struct v4l2_queryctrl queryctrl;

	memset (&queryctrl, 0, sizeof (queryctrl));
	// Start control enumeration from V4L2_CID_LASTP1 to skip standard controls
	queryctrl.id = V4L2_CID_LASTP1 | V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl_VIDIOC_QUERYCTRL(data->device->dev_fd, &queryctrl, 0))
	{
		test_result &= test_control(data, queryctrl.id);
		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	return test_result;
}

static int test_control(v4l2_data* data, int ctlid)
{
	struct v4l2_queryctrl queryctrl;
	int test_result = 1;
	int restore_value = 0;

	// Query the control
	memset (&queryctrl, 0, sizeof (queryctrl));
	queryctrl.id = ctlid;

	if (-1 == ioctl_VIDIOC_QUERYCTRL(data->device->dev_fd, &queryctrl, 0))
	{
		if (errno == EINVAL)
		{
			BLTS_DEBUG("Control \"%s\" (0x%08X) is not supported\n", queryctrl.name, ctlid);
			return 1;
		}
		else
		{
			BLTS_ERROR("VIDIOC_QUERYCTRL error %d, %s\n", errno, strerror(errno));
			return 0;
		}
	}
	else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
	{
		BLTS_DEBUG("Control \"%s\" (0x%08X) is disabled\n", queryctrl.name, ctlid);
		return 1;
	}
	else if (queryctrl.flags & (V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_WRITE_ONLY))
	{
		BLTS_DEBUG("Control \"%s\" (0x%08X) is read only or write only\n", queryctrl.name, ctlid);
		return 1;
	}
	else if (queryctrl.flags & V4L2_CTRL_FLAG_INACTIVE)
	{
		BLTS_DEBUG("Control \"%s\" (0x%08X) is inactive\n", queryctrl.name, ctlid);
		return 1;
	}

	LOG("Testing control \"%s\"\n", queryctrl.name);
	BLTS_DEBUG("Control \"%s\" (0x%08X) value range %d - %d type %d flags 0x%X\n",
			queryctrl.name, ctlid, queryctrl.minimum, queryctrl.maximum, queryctrl.type, queryctrl.flags);

	// Store current value of control
	__s32 orig_value;
	if (!get_control_value(data->device, ctlid, &orig_value))
		return 0;

	switch (queryctrl.type)
	{
	case V4L2_CTRL_TYPE_INTEGER:
		test_result = validate_control_values(data, queryctrl.name, queryctrl.minimum, queryctrl.maximum) && test_result;

		test_result = test_control_value(data, queryctrl.name, ctlid, queryctrl.minimum) && test_result;
		test_result = test_control_value(data, queryctrl.name, ctlid, queryctrl.maximum) && test_result;

		test_result = test_configured_values(data, queryctrl.name, ctlid) && test_result;

		restore_value = 1;
		break;

	case V4L2_CTRL_TYPE_BOOLEAN:
		test_result = test_control_value(data, queryctrl.name, ctlid, queryctrl.minimum) && test_result;
		test_result = test_control_value(data, queryctrl.name, ctlid, queryctrl.maximum) && test_result;

		restore_value = 1;
		break;

	case V4L2_CTRL_TYPE_INTEGER64:
		/* No min/max values available, test values must be provided */
		/* in configuration file if testing is required for int64 controls */
		break;

	case V4L2_CTRL_TYPE_MENU:
		test_result = test_control_menu_values(data, queryctrl.name, ctlid, queryctrl.minimum, queryctrl.maximum) && test_result;

		restore_value = 1;
		break;

	case V4L2_CTRL_TYPE_STRING:
	case V4L2_CTRL_TYPE_BUTTON:
	case V4L2_CTRL_TYPE_CTRL_CLASS:
		/* Cannot test */
		break;

	default:
		break;
	}

	if (restore_value)
	{
		BLTS_DEBUG("\tRestoring original value %d\n", orig_value);
		set_control_value(data->device, ctlid, orig_value);
	}

	return test_result;
}

static int validate_control_values(v4l2_data* data, const unsigned char* ctlname, __s32 min, __s32 max)
{
	char conf_name[128];
	char escaped_name[128];
	__s32 conf_min, conf_max;
	int devnamelen = strlen(data->device->dev_name);

	escape_control_name(ctlname, (unsigned char*)escaped_name);

	sprintf(conf_name, "dev%c_%s_min", data->device->dev_name[devnamelen - 1], escaped_name);
	if (blts_config_get_value_int(conf_name, &conf_min) != 0)
	{
		BLTS_DEBUG("\tNo minimum value configured\n");
		return 1;
	}

	sprintf(conf_name, "dev%c_%s_max", data->device->dev_name[devnamelen - 1], escaped_name);
	if (blts_config_get_value_int(conf_name, &conf_max) != 0)
	{
		BLTS_DEBUG("\tNo maximum value configured\n");
		return 1;
	}

	if (min != conf_min)
	{
		BLTS_ERROR("\tReported minimum %d is different from configured minimum %d\n", min, conf_min);
		return 0;
	}

	if (max != conf_max)
	{
		BLTS_ERROR("\tReported maximum %d is different from configured maximum %d\n", max, conf_max);
		return 0;
	}

	return 1;
}

static int test_configured_values(v4l2_data* data, const unsigned char* ctlname, int ctlid)
{
	char conf_name[128];
	char escaped_name[128];
	char* conf_values;
	int devnamelen = strlen(data->device->dev_name);
	int test_result = 1;

	escape_control_name(ctlname, (unsigned char*)escaped_name);

	sprintf(conf_name, "dev%c_%s_testvalues", data->device->dev_name[devnamelen - 1], escaped_name);
	if (blts_config_get_value_string(conf_name, &conf_values) != 0)
	{
		BLTS_DEBUG("\tNo values configured\n", conf_name);
		return 1;
	}

	char* saveptr = NULL;
	char* p;
	p = strtok_r(conf_values, " ", &saveptr);
	while (p != NULL)
	{
		char* endptr;
		int conf_value = strtol(p, &endptr, 10);
		if (*endptr != 0)
		{
			BLTS_ERROR("\tSyntax error: invalid value %s\n", p);
			test_result = 0;
			break;
		}

		test_result = test_control_value(data, ctlname, ctlid, conf_value) && test_result;
		p = strtok_r(NULL, " ", &saveptr);
	}

	if (conf_values)
		free(conf_values);

	return test_result;
}

static int test_control_value(v4l2_data* data, const unsigned char* ctlname, int ctlid, __s32 value)
{
	int res;
	if (set_control_value(data->device, ctlid, value))
	{
		LOG("\tValue set to %d\n", value);
		res = 1;

		if (data->flags & CLI_FLAG_IMGSAVE)
		{
			LOG("\tTaking snapshot\n");
			char filename[128];
			create_picture_filename_for_ctlvalue(filename, data, ctlname, value);
			res = take_snapshot(data->device, filename);

			if (res && data->img_verify_tool != NULL)
			{
				res = verify_image(data, filename, ctlname, ctlid, value);
			}
		}
	}
	else
	{
		BLTS_ERROR("\tSetting %s to value %d failed\n", ctlname, value);
		res = 0;
	}

	return res;
}

static int test_control_menu_values(v4l2_data* data, const unsigned char* ctlname, int ctlid, __s32 minvalue, __s32 maxvalue)
{
	struct v4l2_querymenu querymenu;
	int test_result = 1;

	memset (&querymenu, 0, sizeof (querymenu));
	querymenu.id = ctlid;

	for (querymenu.index = minvalue;
		querymenu.index <= maxvalue;
		querymenu.index++)
	{
		if (0 == ioctl_VIDIOC_QUERYMENU(data->device->dev_fd, &querymenu, 0))
		{
			test_result = test_control_value(data, ctlname, ctlid, querymenu.index) && test_result;
		}
		else if (errno == EINVAL)
		{
			LOG("\tMenu index %d not supported\n", querymenu.index);
		}
		else
		{
			test_result = 0;
			break;
		}
	}

	return test_result;
}

static int set_control_value(v4l2_dev_data* dev, int ctlid, __s32 value)
{
	if (V4L2_CTRL_ID2CLASS(ctlid) == V4L2_CTRL_CLASS_USER)
	{
		// User-class i.e. standard control, use old ioctl
		struct v4l2_control control;
		memset(&control, 0, sizeof(control));
		control.id = ctlid;
		control.value = value;

		if (-1 == ioctl_VIDIOC_S_CTRL(dev->dev_fd, &control, 0))
			return 0;
	}
	else
	{
		struct v4l2_ext_control control;
		struct v4l2_ext_controls controls;

		memset(&control, 0, sizeof(control));
		control.id = ctlid;
		control.value = value;

		memset(&controls, 0, sizeof(controls));
		controls.count = 1;
		controls.ctrl_class = V4L2_CTRL_ID2CLASS(ctlid);
		controls.controls = &control;

		if (-1 == ioctl_VIDIOC_TRY_EXT_CTRLS(dev->dev_fd, &controls, 0))
		{
			// VIDIOC_TRY_EXT_CTRLS may fail because the ioctl is not implemented.
			// In that case, EINVAL is returned, which is also returned when
			// the kernel driver rejects the input values. These cases cannot
			// be distinguished, so don't fail the test here.
		}

		if (-1 == ioctl_VIDIOC_S_EXT_CTRLS(dev->dev_fd, &controls, 0))
			return 0;
	}

	return 1;
}

static int get_control_value(v4l2_dev_data* dev, int ctlid, __s32* value)
{
	if (V4L2_CTRL_ID2CLASS(ctlid) == V4L2_CTRL_CLASS_USER)
	{
		// User-class i.e. standard control, use old ioctl
		struct v4l2_control control;
		memset(&control, 0, sizeof(control));
		control.id = ctlid;

		if (-1 == ioctl_VIDIOC_G_CTRL(dev->dev_fd, &control, 0))
			return 0;

		*value = control.value;
	}
	else
	{
		struct v4l2_ext_control control;
		struct v4l2_ext_controls controls;

		memset(&control, 0, sizeof(control));
		control.id = ctlid;

		memset(&controls, 0, sizeof(controls));
		controls.count = 1;
		controls.ctrl_class = V4L2_CTRL_ID2CLASS(ctlid);
		controls.controls = &control;

		if (-1 == ioctl_VIDIOC_G_EXT_CTRLS(dev->dev_fd, &controls, 0))
			return 0;

		*value = control.value;
	}

	return 1;
}

static void create_picture_filename_for_ctlvalue(char* filename, const v4l2_data* data, const unsigned char* ctlname, __s32 ctlvalue)
{
	v4l2_dev_data* dev = data->device;
	unsigned char escaped_name[128];

	escape_control_name(ctlname, escaped_name);

	sprintf(filename, "%s/ctl_image_%ix%i_%c%c%c%c_dev%c_%s_%d_",
		data->img_save_path,
		dev->format.fmt.pix.width, dev->format.fmt.pix.height,
		dev->fmt_a, dev->fmt_b, dev->fmt_c, dev->fmt_d,	dev->dev_name[strlen(dev->dev_name)-1],
		escaped_name, ctlvalue);
	switch (dev->io_method)
	{
	case IO_METHOD_MMAP:
		strcat(filename, "mmap.jpg");
		break;
	case IO_METHOD_USERPTR:
		strcat(filename, "userp.jpg");
		break;
	case IO_METHOD_READ:
		strcat(filename, "read.jpg");
		break;
	case IO_METHOD_NONE:
		strcat(filename, ".jpg");
		break;
	}
}

static int take_snapshot(v4l2_dev_data* dev, char* filename)
{
	if (!start_capturing(dev))
		return 0;

	if (!mainloop(dev, 0))
	{
		stop_capturing(dev);
		return 0;
	}

	if (!do_snapshot(dev, filename))
	{
		stop_capturing(dev);
		return 0;
	}

	stop_capturing(dev);
	return 1;
}

static int verify_image(v4l2_data* data, const char* filename, const unsigned char* ctlname, int ctlid, __s32 value)
{
	char verifycmd[256];
	FILE* fpipe;
	int verifyres;
	int exitstatus;
	int res;
	char line[256];

	LOG("\tVerifying image\n");

	sprintf(verifycmd, "%s '%s' '%s' %d %d",
			data->img_verify_tool, filename, ctlname, ctlid, value);
	fpipe = popen(verifycmd, "r");
	if (fpipe == NULL)
	{
		BLTS_ERROR("\tFailed to execute verify command: %s\n", strerror(errno));
		BLTS_ERROR("\tCommand line was:\n")
		BLTS_ERROR("\t%s\n", verifycmd);
		res = 0;
	}
	else
	{
		// Read the output of the verify tool from the pipe and copy it to the log
		while (fgets(line, sizeof(line), fpipe))
		{
			int n = strlen(line);
			LOG("\t%s: %s%s", data->img_verify_tool, line, (line[n-1] == '\n') ? "" : "\n");
		}

		verifyres = pclose(fpipe);
		if (WIFEXITED(verifyres))
		{
			exitstatus = WEXITSTATUS(verifyres);

			if (exitstatus == -1 || exitstatus == 127)
			{
				BLTS_ERROR("\tFailed to execute verify command, command line was:\n");
				BLTS_ERROR("\t\"%s\"\n", verifycmd);
				res = 0;
			}
			else if (exitstatus != 0)
			{
				BLTS_ERROR("\tImage verification failed with result %d\n", exitstatus);
				res = 0;
			}
			else
			{
				LOG("\tImage verified successfully\n");
				res = 1;
			}
		}
		else
		{
			BLTS_ERROR("\tVerify command terminated with signal %d\n", WTERMSIG(verifyres));
			res = 0;
		}
	}

	return res;
}

static void escape_control_name(const unsigned char* name, unsigned char* escaped_name)
{
	int i, j;
	int n = strlen((const char*)name);
	char* escapedchars = " ";
	char* skippedchars = "/()[]";

	for (i = 0, j = 0; i < n; i++)
	{
		if (strchr(escapedchars, name[i]) != NULL)
			escaped_name[j++] = '_';
		else if (strchr(skippedchars, name[i]) == NULL)
			escaped_name[j++] = name[i];
	}
	escaped_name[j] = 0;
}
