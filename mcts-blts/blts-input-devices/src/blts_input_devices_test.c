/* blts_input_devices_test.c -- Command line interface for input device -tests

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
#include "input_util.h"
#include "input_test_cases.h"

static int parse_input_dev_list(struct input_data *data, char *list)
{
	char *tmp, *ptr;

	tmp = strdup(list);
	if(!tmp)
		return -ENOMEM;

	data->num_devices = 0;
	memset(data->devices, 0, sizeof(data->devices));
	ptr = strtok(tmp, " ");

	do {
		if (ptr[0] != '/')
			strcpy(data->devices[data->num_devices], "/dev/input/");

		strcat(data->devices[data->num_devices++], ptr);
		if (data->num_devices >= MAX_DEVICES)
			break;
	} while ((ptr = strtok(NULL, " ")));

	free(tmp);
	return 0;
}

static int parse_key_mapping(struct input_data *data, char *list)
{
	char *tmp, *ptr;
	int ret = -1;

	tmp = strdup(list);
	if(!tmp)
		return -ENOMEM;

	data->num_devices = 0;
	memset(&data->key, 0, sizeof(data->key));

	ptr = strtok(tmp, " ");
	if (!ptr)
		goto cleanup;
	if (ptr[0] != '/')
		strcpy(data->key.device, "/dev/input/");
	strcat(data->key.device, ptr);

	ptr = strtok(NULL, " ");
	if (!ptr)
		goto cleanup;
	data->key.code = strtol(ptr, NULL, 10);

	ptr = strtok(NULL, "");
	if (!ptr)
		goto cleanup;
	strcpy(data->key.description, ptr);

	ret = 0;
cleanup:
	free(tmp);
	return ret;
}

static void *enumerate_test_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	struct input_data *data = ((struct input_data *) user_ptr);
	if (!data)
		return NULL;

	if (parse_input_dev_list(data,
		blts_config_boxed_value_get_string(args))) {
		BLTS_ERROR("Failed to parse device list '%s'\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}

	return data;
}

static void *key_test_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	struct input_data *data = ((struct input_data *) user_ptr);
	if (!data)
		return NULL;

	if (parse_key_mapping(data,
		blts_config_boxed_value_get_string(args))) {
		BLTS_ERROR("Failed to parse device list '%s'\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}

	return data;
}

static void *pointer_test_variant_set_arg_processor(struct boxed_value *args,
	void *user_ptr)
{
	struct input_data *data = ((struct input_data *) user_ptr);
	if (!data)
		return NULL;

	if (blts_config_boxed_value_get_string(args)[0] != '/')
		strcpy(data->pointer_device, "/dev/input/");
	strcat(data->pointer_device, blts_config_boxed_value_get_string(args));
	args = args->next;
	data->swap_xy = blts_config_boxed_value_get_int(args);

	args = args->next;
	if (sscanf(blts_config_boxed_value_get_string(args), "%dx%d",
		&data->scr_width, &data->scr_height) != 2) {
		BLTS_ERROR("Invalid value '%s' in configuration file\n",
			blts_config_boxed_value_get_string(args));
		return NULL;
	}
	args = args->next;

	return data;
}

static void blts_input_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-g] [-d <device_name>]",
		"  -g: Do not grab the devices exclusively\n"
		"  -d: Debug specific device. This is for development purposes.\n");
}

static const char short_opts[] = "d:g";
static const struct option long_opts[] =
{
	{"device", 0, NULL, 'd'},
	{"nograb", 0, NULL, 'g'},
	{0,0,0,0}
};

static void* blts_input_argument_processor(int argc, char **argv)
{
	BLTS_UNUSED_PARAM(argc)
	BLTS_UNUSED_PARAM(argv)
	int ret, c;
	struct input_data *data = malloc(sizeof(struct input_data));
	memset(data, 0, sizeof(*data));

	while((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
		switch(c) {
		case 0:
			/* getopt_long() flag */
			break;
		case 'g':
			data->no_grab = 1;
			break;
		case 'd':
			test_device(optarg);
			/* Not a test case. Fall through and fail. */
		default:
			free(data);
			return NULL;
		}
	}

	ret = blts_config_declare_variable_test(
		"Input - Enumerate devices",
		enumerate_test_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "input_devices", "event1",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;
/*
	ret = blts_config_declare_variable_test(
		"Input - Key events",
		key_test_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "keys", "event1 1 Escape",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test(
		"Input - Pointer events",
		pointer_test_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "pointer_device", "event4",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;
*/
	ret = blts_config_declare_variable_test(
		"Input - Single touch",
		pointer_test_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "pointer_device", "event4",
		CONFIG_PARAM_INT, "scr_orientation", 0,
		CONFIG_PARAM_STRING, "scr_size", "864x480",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test(
		"Input - Multi-touch",
		pointer_test_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "pointer_device", "event4",
		CONFIG_PARAM_INT, "scr_orientation", 0,
		CONFIG_PARAM_STRING, "scr_size", "864x480",
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	return data;

error_exit:
	free(data);
	return NULL;
}

static void blts_input_teardown(void *user_ptr)
{
	if(user_ptr)
		free(user_ptr);
}

static blts_cli_testcase blts_input_cases[] =
{
	{ "Input - Enumerate devices", input_enumerate_test, 120000 },
	{ "Input - Single touch", input_single_touch_test, 3600000 },
	{ "Input - Multi-touch", input_multi_touch_test, 3600000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli input_cli =
{
	.test_cases = blts_input_cases,
	.log_file = "blts_input.txt",
	.blts_cli_help = blts_input_help,
	.blts_cli_process_arguments = blts_input_argument_processor,
	.blts_cli_teardown = blts_input_teardown
};

int main(int argc, char **argv)
{
	log_set_level(LEVEL_DEBUG);
	return blts_cli_main(&input_cli, argc, argv);
}

