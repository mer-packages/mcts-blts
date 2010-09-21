/* blts-usb-testrunner.c -- USB testing tool

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
#include <blts_log.h>
#include <getopt.h>
#include <blts_cli_frontend.h>
#include <blts_params.h>

#include "usb-util.h"
#include "transfer-cases.h"
#include "peripheral-control.h"

static void my_usb_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		/* What is displayed on the first 'USAGE' line */
		"[-h host driver] [-p peripheral driver] [-t timeout]\n",
		/* Description of the arguments */
		"  -h: Host driver name (default: blts_usb_host)\n"
		"  -p: Peripheral driver name (default: blts_gadget)\n"
		"  -t: Data transfer timeout\n"
		);
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "h:p:t:";
static const struct option long_opts[] =
{
	{0,0,0,0}
};

/* Return NULL in case of an error */
static void* my_usb_argument_processor(int argc, char **argv)
{
	int c, ret;
	my_usb_data* my_data = malloc(sizeof(my_usb_data));
	memset(my_data, 0, sizeof(my_usb_data));

	while((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch(c)
		{
		case 'h':
			my_data->host_driver = strdup(optarg);
			break;
		case 'p':
			my_data->peripheral_driver = strdup(optarg);
			break;
		case 't':
			my_data->data_transfer_timeout = atoi(optarg);
			break;
		case 0:
			/* getopt_long() flag */
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	ret = blts_config_declare_variable_test("USB - Setup host driver",
		data_transfer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "host_driver", "blts_usb_host",
		CONFIG_PARAM_STRING, "host_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("USB - Setup peripheral driver",
		peripheral_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "peripheral_driver", "blts_gadget",
		CONFIG_PARAM_STRING, "peripheral_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_INT, "usb_transfer_size", 4096,
		CONFIG_PARAM_STRING, "ep_configuration_file", "/usr/share/blts-usb-tests/ep-configuration-default.cfg",
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("USB - Read test",
		data_transfer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "host_driver", "blts_usb_host",
		CONFIG_PARAM_STRING, "host_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_STRING, "peripheral_driver", "gadgetdrv",
		CONFIG_PARAM_STRING, "peripheral_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_INT, "usb_transfer_size", 4096,
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	ret = blts_config_declare_variable_test("USB - Write test",
		data_transfer_variant_set_arg_processor,
		CONFIG_PARAM_STRING, "host_driver", "blts_usb_host",
		CONFIG_PARAM_STRING, "host_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_STRING, "peripheral_driver", "gadgetdrv",
		CONFIG_PARAM_STRING, "peripheral_driver_path", "/usr/lib/tests/blts-usb-tests/",
		CONFIG_PARAM_INT, "usb_transfer_size", 4096,
		CONFIG_PARAM_NONE);
	if (ret)
		return NULL;

	return my_data;
}

/* user_ptr is what you returned from my_usb_argument_processor */
static void my_usb_teardown(void *user_ptr)
{
	my_usb_data* data = (my_usb_data*)user_ptr;

	if(user_ptr)
	{
		if (data->host_driver)
			free(data->host_driver);

		if (data->host_driver_path)
			free(data->host_driver_path);

		if (data->peripheral_driver)
			free(data->peripheral_driver);

		if (data->peripheral_driver_path)
			free(data->peripheral_driver_path);

		free(user_ptr);
	}
}


/* Add required application init here; run only once. */
static void app_init_once()
{
	static int init_done = 0;

	if (init_done)
		return;

	init_done = 1;
}

/* user_ptr is what you returned from my_usb_argument_processor
 * test_num Test case being run from my_usb_cases, starts from 1
 * return non-zero in case of an error */

/* Your test definitions */

static blts_cli_testcase my_usb_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity */
	{ "USB - Setup host driver", blts_setup_host_driver, 0 },
	{ "USB - Setup peripheral driver", blts_setup_peripheral_driver, 0},
	{ "USB - Read test", blts_data_read, 0 },
	{ "USB - Write test", blts_data_write, 0 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli my_usb_cli =
{
	.test_cases = my_usb_cases,
	.log_file = "blts_usb_log.txt",
	.blts_cli_help = my_usb_help,
	.blts_cli_process_arguments = my_usb_argument_processor,
	.blts_cli_teardown = my_usb_teardown
};

int main(int argc, char **argv)
{
	app_init_once();

	return blts_cli_main(&my_usb_cli, argc, argv);
}

