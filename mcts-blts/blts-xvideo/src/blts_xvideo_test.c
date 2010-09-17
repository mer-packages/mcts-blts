/* blts_xvideo_test.c -- Command line interface for XVideo tests

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
#include <getopt.h>
#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>
#include "xvideo_util.h"
#include "xvideo_test_cases.h"

static void blts_xvideo_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base, "", "");
}

static int parse_port_attr_list(xvideo_data *data, char *list)
{
	char *tmp, *ptr;

	tmp = strdup(list);
	if(!tmp)
		return -ENOMEM;

	ptr = strtok(tmp, " ");
	data->num_attrs = 0;

	while (ptr) {
		strcpy(data->attrs[data->num_attrs].name, ptr);
		ptr = strtok(NULL, " ");
		if (!ptr) {
			free(tmp);
			return -EINVAL;
		}

		data->attrs[data->num_attrs].value = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " ");

		BLTS_TRACE("Got port attribute: %s = %d\n",
			data->attrs[data->num_attrs].name,
			data->attrs[data->num_attrs].value);

		data->num_attrs++;
	}

	free(tmp);
	return 0;
}

void *benchmark_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	xvideo_data *data = ((xvideo_data *) user_ptr);
	if (!data)
		return NULL;

	if (sscanf(blts_config_boxed_value_get_string(args), "%dx%d",
		&data->src_width, &data->src_height) != 2) {
		BLTS_ERROR("Invalid value '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	if (sscanf(blts_config_boxed_value_get_string(args), "%dx%d",
		&data->dst_width, &data->dst_height) != 2) {
		BLTS_ERROR("Invalid value '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	data->duration = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->screen = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->adaptor = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->test_images = string_to_test_images(
		blts_config_boxed_value_get_string(args));
	if (data->test_images == XV_TI_UNKNOWN) {
		BLTS_ERROR("Invalid test image in configuration file (%s)\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	if (strlen(blts_config_boxed_value_get_string(args)) != 4) {
		BLTS_ERROR("Invalid pixel format '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	strcpy(data->fourcc, blts_config_boxed_value_get_string(args));

	args = args->next;
	if (parse_port_attr_list(data,
		blts_config_boxed_value_get_string(args))) {
		BLTS_ERROR("Failed to parse port attribures '%s'\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}

	return data;
}

void *querybestsize_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	xvideo_data *data = ((xvideo_data *) user_ptr);
	if (!data)
		return NULL;

	if (sscanf(blts_config_boxed_value_get_string(args), "%dx%d",
		&data->src_width, &data->src_height) != 2) {
		BLTS_ERROR("Invalid value '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	data->screen = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->adaptor = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->test_images = string_to_test_images(
		blts_config_boxed_value_get_string(args));
	if (data->test_images == XV_TI_UNKNOWN) {
		BLTS_ERROR("Invalid test image in configuration file (%s)\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	if (strlen(blts_config_boxed_value_get_string(args)) != 4) {
		BLTS_ERROR("Invalid pixel format '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	strcpy(data->fourcc, blts_config_boxed_value_get_string(args));

	args = args->next;
	if (parse_port_attr_list(data,
		blts_config_boxed_value_get_string(args))) {
		BLTS_ERROR("Failed to parse port attribures '%s'\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}

	return data;
}

void *notifyevents_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	xvideo_data *data = ((xvideo_data *) user_ptr);
	if (!data)
		return NULL;

	data->screen = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->adaptor = blts_config_boxed_value_get_int(args);
	args = args->next;

	if (strlen(blts_config_boxed_value_get_string(args)) != 4) {
		BLTS_ERROR("Invalid pixel format '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	strcpy(data->fourcc, blts_config_boxed_value_get_string(args));

	args = args->next;
	if (parse_port_attr_list(data,
		blts_config_boxed_value_get_string(args))) {
		BLTS_ERROR("Failed to parse port attribures '%s'\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}

	return data;
}

static void* blts_xvideo_argument_processor(int argc, char **argv)
{
	BLTS_UNUSED_PARAM(argc)
	BLTS_UNUSED_PARAM(argv)
	int ret;
	xvideo_data* xv = malloc(sizeof(xvideo_data));
	memset(xv, 0, sizeof(xvideo_data));

	ret = blts_config_declare_variable_test(
		"XVideo - XvPutImage benchmark",
		benchmark_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "input_resolution", "800x600",
		CONFIG_PARAM_STRING, "output_resolution", "0x0",
		CONFIG_PARAM_INT, "duration", 5,
		CONFIG_PARAM_INT, "screen", 0,
		CONFIG_PARAM_INT, "adaptor", 0,
		CONFIG_PARAM_STRING, "test_images", "checkerboard",
		CONFIG_PARAM_STRING, "format", "YUY2",
		CONFIG_PARAM_STRING, "port_attributes", "",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test(
		"XVideo - XvShmPutImage benchmark",
		benchmark_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "input_resolution", "800x600",
		CONFIG_PARAM_STRING, "output_resolution", "0x0",
		CONFIG_PARAM_INT, "duration", 5,
		CONFIG_PARAM_INT, "screen", 0,
		CONFIG_PARAM_INT, "adaptor", 0,
		CONFIG_PARAM_STRING, "test_images", "checkerboard",
		CONFIG_PARAM_STRING, "format", "YUY2",
		CONFIG_PARAM_STRING, "port_attributes", "",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test(
		"XVideo - PutImage with QueryBestSize",
		querybestsize_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "input_resolution", "800x600",
		CONFIG_PARAM_INT, "screen", 0,
		CONFIG_PARAM_INT, "adaptor", 0,
		CONFIG_PARAM_STRING, "test_images", "checkerboard",
		CONFIG_PARAM_STRING, "fute_format", "YUY2",
		CONFIG_PARAM_STRING, "fute_port_attributes", "XV_BRIGHTNESS 0",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test("XVideo - Notify events",
		notifyevents_variant_set_arg_processor,
		CONFIG_PARAM_INT, "screen", 0,
		CONFIG_PARAM_INT, "adaptor", 0,
		CONFIG_PARAM_STRING, "fute_format", "YUY2",
		CONFIG_PARAM_STRING, "fute_port_attributes", "XV_BRIGHTNESS 0",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	return xv;

error_exit:
	if (xv)
		free(xv);

	return NULL;
}

static void blts_xvideo_teardown(void *user_ptr)
{
	if(user_ptr)
		free(user_ptr);
}

static blts_cli_testcase blts_xvideo_cases[] =
{
	{ "XVideo - Presence check", xvideo_presence_check, 10000 },
	{ "XVideo - Enumerate adapters", xvideo_enumerate_adapters, 10000 },
	{ "XVideo - XvPutImage benchmark", xvideo_putimage, 120000 },
	{ "XVideo - XvShmPutImage benchmark", xvideo_shmputimage, 120000 },
	{ "XVideo - PutImage with QueryBestSize", xvideo_querybestsize, 120000 },
	{ "XVideo - Notify events", xvideo_notify_events_test, 20000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli xvideo_cli =
{
	.test_cases = blts_xvideo_cases,
	.log_file = "blts_xvideo.txt",
	.blts_cli_help = blts_xvideo_help,
	.blts_cli_process_arguments = blts_xvideo_argument_processor,
	.blts_cli_teardown = blts_xvideo_teardown
};

int main(int argc, char **argv)
{
	log_set_level(LEVEL_DEBUG);
	return blts_cli_main(&xvideo_cli, argc, argv);
}

