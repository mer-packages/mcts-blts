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

#include "blts_watchdog_util.h"

#define EXECUTE_TEST(a, b)\
{\
	int ret;\
	LOG("\nStarting test '%s'...\n", a);\
	ret = b;\
	if(ret)\
	{\
		LOGERR("Test failed (%d).\n", ret);\
	}\
	else\
	{\
		LOG("Test passed.\n");\
	}\
}\

static void show_help()
{
	fprintf(stdout, "USAGE: blts-watchdog-tests [-a] [-l logfile] [-t execution_time_in_seconds]\n");
	fprintf(stdout, "-a: Run all tests (default: run only smoke test set)\n");
	fprintf(stdout, "-l: Used logfile (default: ./blts_watchdog.txt)\n");
	fprintf(stdout, "-t: Maximum execution time of each test in seconds (default: 10s for smoke test, 30 s for full test)\n");
}

static void invalid_params()
{
	fprintf(stderr, "Invalid parameters\n");
	show_help();
}

static void smoke_test(double execution_time)
{
	EXECUTE_TEST("Core-Watchdog presence check", wdt_dep_check());
	EXECUTE_TEST("Core-Open and close watchdog", wdt_open_close());
	EXECUTE_TEST("Core-Send keepalive messages", wdt_send_keepalive(execution_time));
}

static void full_test(double execution_time)
{
	// TODO: Add tests here
}

int main(int argc, char *argv[])
{
	int t;
	double execution_time = -1;
	int all_tests = 0;
	char* logfile = "blts_watchdog.txt";

	for(t = 1; t < argc; t++)
	{
		if(strcmp(argv[t], "-a") == 0)
		{
			all_tests = 1;
		}
		else if(strcmp(argv[t], "-l") == 0)
		{
			if(++t >= argc)
			{
				invalid_params();
				return 1;
			}
			logfile = argv[t];
		}
		else if(strcmp(argv[t], "-t") == 0)
		{
			if(++t >= argc)
			{
				invalid_params();
				return 1;
			}
			execution_time = (double)atoi(argv[t]);
		}
		else if(strcmp(argv[t], "-h") == 0)
		{
			show_help();
			return 0;
		}
		else
		{
			invalid_params();
			return 1;
		}
	}

	if(execution_time == -1)
	{
		if(all_tests)
		{
			execution_time = 30.0;
		}
		else
		{
			execution_time = 10.0;
		}
	}

	log_open(logfile, 1);

	if(all_tests)
	{
		smoke_test(execution_time);
		full_test(execution_time);
	}
	else
	{
		smoke_test(execution_time);
	}

	log_close(logfile);

	return 0;
}

