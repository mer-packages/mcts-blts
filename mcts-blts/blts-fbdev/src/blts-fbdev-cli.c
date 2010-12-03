/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-cli.c -- Frame buffer functional tests

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

#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

/* BLTS common */
#include <blts_cli_frontend.h>
#include <blts_params.h>
#include <blts_log.h>

/* Own includes */
#include "blts-fbdev-defs.h"

/* Test cases */
#include "blts-fbdev-fute.h"
#include "blts-fbdev-blanking.h"
#include "blts-fbdev-backlight.h"
#include "blts-fbdev-perf.h"

/* Usage */
static void
blts_fbdev_cli_usage (const char *message)
{
        fprintf (stdout, message,
                 "",
                 "");
}

/* Parse device name parameter from variable list */
static void *
blts_fbdev_cli_args_dev (struct boxed_value *args, void *user_data)
{
	blts_fbdev_data *data = user_data;

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Invalid argument in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
                return NULL;
        }

	if (!args) {
		BLTS_WARNING ("Warning: No arguments to process in %s:%s\n",
                              __FILE__, __PRETTY_FUNCTION__);
		return user_data;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR ("Error: Type mismatch in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
		return NULL;
	}

        char *device_name = blts_config_boxed_value_get_string (args);

        if (!device_name) {
                BLTS_ERROR ("Error: Could not read device name in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
                return NULL;
        }
        data->device->name = strdup (device_name);

	return data;
}

/* Parse backlight framework subsystem and limit values from config */
static void *
blts_fbdev_cli_args_backlight_level (struct boxed_value *args, void *user_data)
{
	blts_fbdev_data *data = user_data;

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Invalid argument in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
                return NULL;
        }

	if (!args) {
		BLTS_WARNING ("Warning: No arguments to process in %s:%s\n",
                              __FILE__, __PRETTY_FUNCTION__);
		return user_data;
	}

	if (args->type != CONFIG_PARAM_STRING) {
		BLTS_ERROR ("Error: Type mismatch in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
		return NULL;
	}

        char *backlight_subsystem = blts_config_boxed_value_get_string (args);

        if (!backlight_subsystem) {
                BLTS_ERROR ("Error: Could not read backlight subsystem in "
                            "%s:%s\n", __FILE__, __PRETTY_FUNCTION__);
                return NULL;
        }
        data->backlight_subsystem = strdup (backlight_subsystem);

        args = args->next;

        if (!args) {
                BLTS_WARNING ("Warning: No minimum backlight level given\n");
                return user_data;
        }

        if (args->type != CONFIG_PARAM_INT &&
            args->type != CONFIG_PARAM_LONG) {
		BLTS_ERROR ("Error: Type mismatch in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
		return user_data;
        }

        data->minimum_light = blts_config_boxed_value_get_int (args);

        args = args->next;

        if (!args) {
                BLTS_WARNING ("Warning: No maximum backlight level given\n");
                return user_data;
        }

        if (args->type != CONFIG_PARAM_INT &&
            args->type != CONFIG_PARAM_LONG) {
		BLTS_ERROR ("Error: Type mismatch in %s:%s\n",
                            __FILE__, __PRETTY_FUNCTION__);
		return user_data;
        }

        data->maximum_light = blts_config_boxed_value_get_int (args);

        return data;
}

/* Process command line arguments (and init). Should return user data
 * to be used in the test cases on success, NULL on failure
 */
static void *
blts_fbdev_cli_parse_args (int argc, char *argv[])
{
        int ret;
        blts_fbdev_data *data = NULL;

        data = malloc (sizeof (blts_fbdev_data));
        memset (data, 0, sizeof (blts_fbdev_data));
        data->device = malloc (sizeof (blts_fbdev_device));
        memset (data->device, 0, sizeof (blts_fbdev_device));

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer read frame buffer information with ioctl",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer set blanking levels",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer 1-pixel uncached read",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer 1-pixel uncached write",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer 1-pixel uncached modify",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer fullscreen buffer read",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer fullscreen buffer write",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Framebuffer fullscreen buffer modify",
                blts_fbdev_cli_args_dev,
                CONFIG_PARAM_STRING, "framebuffer_device", "/dev/fb0",
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Backlight verify backlight levels",
                blts_fbdev_cli_args_backlight_level,
                CONFIG_PARAM_STRING, "backlight_subsystem",
                "/sys/class/backlight/dell_backlight",
                CONFIG_PARAM_INT,    "minimum_level",       0,
                CONFIG_PARAM_INT,    "maximum_level",       7,
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Backlight linear backlight level changes",
                blts_fbdev_cli_args_backlight_level,
                CONFIG_PARAM_STRING, "backlight_subsystem",
                "/sys/class/backlight/dell_backlight",
                CONFIG_PARAM_INT,    "minimum_level",       0,
                CONFIG_PARAM_INT,    "maximum_level",       7,
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        ret = blts_config_declare_variable_test (
                "Core-Backlight logarithmic backlight level changes",
                blts_fbdev_cli_args_backlight_level,
                CONFIG_PARAM_STRING, "backlight_subsystem",
                "/sys/class/backlight/dell_backlight",
                CONFIG_PARAM_INT,    "logarithmic_min",     1,
                CONFIG_PARAM_INT,    "logarithmic_max",     7,
                CONFIG_PARAM_NONE);

        if (ret) goto ERROR;

        return data;

ERROR:
        if (data->device) {
                if (data->device->name) {
                        free (data->device->name);
                        data->device->name = NULL;
                }
                free (data->device);
                data->device = NULL;
        }
        free (data);
        return NULL;
}

/* Closing the test case */
static void
blts_fbdev_cli_teardown (void *user_data)
{
        blts_fbdev_data *data = user_data;

        if (!data) return;

        if (data->device) {
                if (data->device->name) {
                        free (data->device->name);
                        data->device->name = NULL;
                }
                free (data->device);
                data->device = NULL;
        }
        if (data->shuffle_lut) {
                free (data->shuffle_lut);
                data->shuffle_lut = NULL;
        }
        if (data->testframe[0]) {
                free (data->testframe[0]);
                data->testframe[0] = NULL;
        }
        if (data->testframe[1]) {
                free (data->testframe[1]);
                data->testframe[1] = NULL;
        }

        free (data);
}

/* Test case definitions */
static blts_cli_testcase blts_fbdev_cases[] = {
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity
         */
        { "Core-Framebuffer read frame buffer information with ioctl",
          blts_fbdev_case_fetch_info, 30000 },
        { "Core-Framebuffer set blanking levels",
          blts_fbdev_case_blanking, 60000 },
        { "Core-Framebuffer 1-pixel uncached read",
          blts_fbdev_case_one_pixel_read, 60000 },
        { "Core-Framebuffer 1-pixel uncached write",
          blts_fbdev_case_one_pixel_write, 60000 },
        { "Core-Framebuffer 1-pixel uncached modify",
          blts_fbdev_case_one_pixel_readwrite, 60000 },
        { "Core-Framebuffer fullscreen buffer read",
          blts_fbdev_case_buffer_read, 60000 },
        { "Core-Framebuffer fullscreen buffer write",
          blts_fbdev_case_buffer_write, 60000 },
        { "Core-Framebuffer fullscreen buffer modify",
          blts_fbdev_case_buffer_modify, 60000 },
        { "Core-Backlight verify backlight levels",
          blts_fbdev_case_backlight_verify, 60000 },
        { "Core-Backlight linear backlight level changes",
          blts_fbdev_case_backlight_linear, 0 },
        { "Core-Backlight logarithmic backlight level changes",
          blts_fbdev_case_backlight_logarithmic, 0 },
	BLTS_CLI_END_OF_LIST
};

/* Common client definition */
static blts_cli blts_fbdev_cli = {
        .test_cases                 = blts_fbdev_cases,
        .log_file                   = "blts-fbdev-log.txt",
        .blts_cli_help              = blts_fbdev_cli_usage,
        .blts_cli_process_arguments = blts_fbdev_cli_parse_args,
        .blts_cli_teardown          = blts_fbdev_cli_teardown
};

int
main (int argc, char *argv[])
{
        return blts_cli_main (&blts_fbdev_cli, argc, argv);
}
