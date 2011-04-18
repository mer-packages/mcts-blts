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
#include <blts_params.h>
#include <blts_reporting.h>
#include <blts_sync.h>

typedef struct
{
	int q_set;
	int w_set;
	int p_set;
	int var_arg1;
	int var_arg2;
} my_example_data;

static void my_example_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		/* What is displayed on the first 'USAGE' line */
		"[-q] [-w] [-p]",
		/* Description of the arguments */
		"  -q: Example argument q\n"
		"  -w: Example argument w\n"
		"  -p: Example argument p\n");
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

static void *my_example_variable_parser(struct boxed_value *args, void *user_ptr)
{
	my_example_data *data = (my_example_data *)user_ptr;
	if (!data)
		return NULL;

	data->var_arg1 = blts_config_boxed_value_get_int(args);
	args = args->next;
	data->var_arg2 = blts_config_boxed_value_get_int(args);
	args = args->next;

	return data;
}

/* Return NULL in case of an error */
static void* my_example_argument_processor(int argc, char **argv)
{
	int c, ret;
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
			BLTS_TRACE("Argument 'q' given\n");
			my_data->q_set = 1;
			break;
		case 'w':
			BLTS_TRACE("Argument 'w' given\n");
			my_data->w_set = 1;
			break;
		case 'p':
			BLTS_TRACE("Argument 'p' given\n");
			my_data->p_set = 1;
			break;
		default:
			free(my_data);
			return NULL;
		}
	}

	ret = blts_config_declare_variable_test(
		"My example variant test A",
		my_example_variable_parser,
		CONFIG_PARAM_INT, "variable1", 0,
		CONFIG_PARAM_INT, "variable2", 0,
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	ret = blts_config_declare_variable_test(
		"My example variant test B",
		my_example_variable_parser,
		CONFIG_PARAM_INT, "variable1", 0,
		CONFIG_PARAM_INT, "variable2", 0,
		CONFIG_PARAM_NONE);
	if (ret)
		goto error_exit;

	return my_data;

error_exit:
	free(my_data);
	return NULL;
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

	BLTS_DEBUG("in my_example_case_1\n");
	if(data->q_set) BLTS_DEBUG("q is set\n");
	BLTS_DEBUG("This is test #%d\n", test_num);

	return 0;
}

static int my_example_case_2(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;

	BLTS_DEBUG("in my_example_case_2\n");
	if(data->w_set) BLTS_DEBUG("w is set\n");
	BLTS_DEBUG("This is test #%d\n", test_num);
	BLTS_DEBUG("This should not timeout...\n", test_num);
	sleep(5);

	return 0;
}

static int my_example_case_3(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	int *ptr = NULL;
	BLTS_DEBUG("in my_example_case_3\n");

	BLTS_DEBUG("this will crash\n");
	*ptr = data->p_set;
	BLTS_ERROR("you should not see this\n");

	return 0;
}

static int my_example_case_4(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	BLTS_DEBUG("in my_example_case_4\n");

	if(data->p_set) BLTS_DEBUG("p is set\n");
	BLTS_DEBUG("this case should fail cleanly\n");

	return -1;
}

static int my_example_case_5(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	BLTS_DEBUG("in my_example_case_5\n");

	if(data->p_set) BLTS_DEBUG("p is set\n");
	BLTS_DEBUG("this case should should timeout\n");
	sleep(5);
	BLTS_ERROR("you should not see me\n");

	return -1;
}

static int my_example_case_6(void* user_ptr, int test_num)
{
	BLTS_DEBUG("in my_example_case_6\n");
	BLTS_DEBUG("all the variants of this test case should pass\n");
	return 0;
}

static int my_example_case_7(void* user_ptr, int test_num)
{
	my_example_data* data = (my_example_data*)user_ptr;
	BLTS_DEBUG("in my_example_case_7\n");
	if (data->var_arg2 == 1) {
		BLTS_DEBUG("This should fail\n");
		return -1;
	} else {
		BLTS_DEBUG("This should pass\n");
		return 0;
	}
}


static int my_example_case_with_measurement(void* user_ptr, int test_num)
{
	int ret;
	BLTS_DEBUG("in my_example_case_with_measurement\n");
	BLTS_DEBUG("This case will produce a result entry in a CSV file (if available)\n");
	ret = blts_report_extended_result("demo_measurement", 42.0, "fish/s", 0);
	return ret;
}


/* Try:
 *   blts_cli_example -syncprep 2
 *   (blts_cli_example -e <this test> -sync &); blts_cli_example -e <next test> -sync
 */
static int synctest_a(void* user_ptr, int test_num)
{
	int ret;
	/* BLTS_DEBUG("in test_a\n"); */

	blts_sync_add_tag("test_sync_tag");
	blts_sync_add_tag("test_sync_tag_1only");

	BLTS_DEBUG("-> A: sync 1 (plain):\n");
	blts_sync_anon();
	BLTS_DEBUG("-> A: sync 1 done\n");

	sleep(1);

	BLTS_DEBUG("-> A: sync tagged:\n");
	blts_sync_tagged("test_sync_tag");
	BLTS_DEBUG("-> A: done\n");

	sleep(1);

	BLTS_DEBUG("-> A: sync tagged (1 only, this should not block):\n");
	blts_sync_tagged("test_sync_tag_1only");
	BLTS_DEBUG("-> A: done\n");

	sleep(1);

	BLTS_DEBUG("-> A: sync 2 (plain):\n");
	blts_sync_anon();
	BLTS_DEBUG("-> A: sync 2 done\n");

	BLTS_DEBUG("-> A: sync 3 (short timeout):\n");
	ret = blts_sync_anon_to(100);
	BLTS_DEBUG("-> A: sync 3 done\n");

	BLTS_DEBUG("-> A: sync 4 (expect timeout with B):\n");
	ret = blts_sync_anon_to(100);
	BLTS_DEBUG("-> A: sync 4 done, ret = %d\n",ret);

	BLTS_DEBUG("-> A: final (plain) sync:\n");
	blts_sync_anon();
	BLTS_DEBUG("-> A: final sync done\n");

	return 0;
}

static int synctest_b(void* user_ptr, int test_num)
{
	/* BLTS_DEBUG("in test_b\n"); */

	blts_sync_add_tag("test_sync_tag");
	sleep(2);

	BLTS_DEBUG("-> B: sync 1 (plain):\n");
	blts_sync_anon();
	BLTS_DEBUG("-> B: sync 1 done\n");
	sleep(2);

	BLTS_DEBUG("-> B: sync tagged:\n");
	blts_sync_tagged("test_sync_tag");
	BLTS_DEBUG("-> B: done\n");
	sleep(2);

	BLTS_DEBUG("-> B: sync 2 (plain):\n");
	blts_sync_anon();
	BLTS_DEBUG("-> B: sync 2 done\n");

	BLTS_DEBUG("-> B: sync 3 (plain):\n");
	blts_sync_anon();
	BLTS_DEBUG("-> B: sync 3 done\n");

	BLTS_DEBUG("-> B: sleeping a bit...\n");
	sleep(3);
	BLTS_DEBUG("-> B: done.\n");

	BLTS_DEBUG("-> B: final (plain) sync:\n");
	blts_sync_anon();
	BLTS_DEBUG("-> B: final sync done\n");

	return 0;
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
	{ "My example variant test A", my_example_case_6, 2000 },
	{ "My example variant test B", my_example_case_7, 2000 },
	{ "My example test with measurement", my_example_case_with_measurement, 2000 },
	{ "My example test with sync points A", synctest_a, 20000 },
	{ "My example test with sync points B", synctest_b, 20000 },
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

