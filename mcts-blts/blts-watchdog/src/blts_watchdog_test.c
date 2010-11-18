/* blts_watchdog_test.c -- Command line interface for watchdog tests

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

#include "blts_watchdog_util.h"

static void watchdog_cli_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"",
		"");
}
static void* watchdog_argument_processor(int argc, char **argv)
{
	BLTS_UNUSED_PARAM(argc)
	BLTS_UNUSED_PARAM(argv)
	return (void*)1;
}

static void watchdog_teardown(void *user_ptr)
{
	BLTS_UNUSED_PARAM(user_ptr)
}

static int run_tests(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(user_ptr)

	switch(test_num) {
	case 1:
		return wdt_dep_check();
	case 2:
		return wdt_open_close();
	case 3:
		return wdt_send_keepalive(20);
	default:
		 BLTS_ERROR("Unknown test case %d!\n", test_num);
		 return -EINVAL;
	}
}

static blts_cli_testcase watchdog_cases[] =
{
	{ "Core-Watchdog presence check", run_tests, 60000 },
	{ "Core-Open and close watchdog", run_tests, 60000 },
	{ "Core-Send keepalive messages", run_tests, 60000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli watchdog_cli =
{
	.test_cases = watchdog_cases,
	.log_file = "blts_watchdog.txt",
	.blts_cli_help = watchdog_cli_help,
	.blts_cli_process_arguments = watchdog_argument_processor,
	.blts_cli_teardown = watchdog_teardown
};

int main(int argc, char **argv)
{
	return blts_cli_main(&watchdog_cli, argc, argv);
}

