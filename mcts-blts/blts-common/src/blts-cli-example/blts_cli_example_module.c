/* blts_cli_example_module.c -- Example using common CLI functions

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

typedef struct
{
	int q_set;
	int w_set;
	int p_set;
} my_example_data;

static void my_example_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		/* What is displayed on the first 'USAGE' line */
		"[-q] [-w] [-p]",
		/* Description of the arguments */
		"-q: Some argument\n"
		"-w: Some argument\n"
		"-p: Some argument\n");
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "qwp";
static const struct option long_opts[] =
{
	{"setq", no_argument, NULL, 'q'},
	{"setw", no_argument, NULL, 'w'},
	{"setp", no_argument, NULL, 'p'},
	{0,0,0,0}
};

/* Return NULL in case of an error */
static void* my_example_argument_processor(int argc, char **argv)
{
	int c;
	my_example_data* my_data = malloc(sizeof(my_example_data));
	memset(my_data, 0, sizeof(my_example_data));

	while((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch(c)
		{
		case 0:
			/* getopt_long() flag */
			break;
		case 'q':
			LOG("Argument 'q' given\n");
			my_data->q_set = 1;
			break;
		case 'w':
			LOG("Argument 'w' given\n");
			my_data->w_set = 1;
			break;
		case 'p':
			LOG("Argument 'p' given\n");
			my_data->p_set = 1;
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	return my_data;
}

/* user_ptr is what you returned from my_example_argument_processor */
static void my_example_teardown(void *user_ptr)
{
	if(user_ptr)
	{
		free(user_ptr);
	}
}

/* user_ptr is what you returned from my_example_argument_processor
 * test_num Test case being run from my_example_cases, starts from 1
 * return non-zero in case of an error */
static int my_example_case_1(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;

	LOG("in my_example_case_1\n");
	if(data->q_set) LOG("q is set\n");
	LOG("This is test #%d\n", test_num);

	return 0;
}

static int my_example_case_2(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;

	LOG("in my_example_case_2\n");
	if(data->w_set) LOG("w is set\n");
	LOG("This is test #%d\n", test_num);
	LOG("This should not timeout...\n", test_num);
	sleep(5);

	return 0;
}

static int my_example_case_3(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	int *ptr = NULL;
	LOG("in my_example_case_3\n");

	LOG("this will crash\n");
	*ptr = data->p_set;
	LOG("you should not see this\n");

	return 0;
}

static int my_example_case_4(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	LOG("in my_example_case_4\n");

	if(data->p_set) LOG("p is set\n");
	LOG("this case should fail cleanly\n");

	return -1;
}

static int my_example_case_5(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	LOG("in my_example_case_5\n");

	if(data->p_set) LOG("p is set\n");
	LOG("this case should should timeout\n");
	sleep(5);
	LOG("you should not see me\n");

	return -1;
}

/* Your test definitions */

static blts_cli_testcase my_example_cases[] =
{
	/* Test case name, test case function, timeout in ms
	 * It is possible to use same function for multiple cases.
	 * Zero timeout = infinity */
	{ "My example A", my_example_case_1, 5000 },
	{ "My example B", my_example_case_2, 0 },
	{ "My example C", my_example_case_3, 5000 },
	{ "My example D", my_example_case_4, 5000 },
	{ "My example E", my_example_case_5, 2000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli my_example_cli =
{
	.test_cases = my_example_cases,
	.log_file = "my_example_log.txt",
	.blts_cli_help = my_example_help,
	.blts_cli_process_arguments = my_example_argument_processor,
	.blts_cli_teardown = my_example_teardown
};

int main(int argc, char **argv)
{
	/* You can do something here if you wish */
	return blts_cli_main(&my_example_cli, argc, argv);
}

