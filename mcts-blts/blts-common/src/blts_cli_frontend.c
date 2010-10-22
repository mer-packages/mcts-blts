/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* blts_cli_frontend.c -- CLI handling

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <memory.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>

#include <blts_log.h>
#include "blts_cli_frontend.h"
#include "blts_params.h"
#include "blts_params_local.h"

#define MAX_ARGS 256

#define sigsegv_outp(x, ...) BLTS_ERROR(x "\n", ##__VA_ARGS__)

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

#define BLTS_CONFIG_SUFFIX ".cnf"

static struct sigaction *saved_sigchld_action = NULL;
static unsigned int timout_override = 0;

static void show_help(char* bin_name, blts_cli* cli)
{
	const char* help_msg_base =
		"USAGE: %s [-l mylog.txt] [-e test1,test2...] [-en \"my test\"] [-s] "
		"[-?] [-v] [-C variation_config.cnf] %%s\n"
		"  -l: Used log file, default %s\n"
		"  -e: Execute single or multiple selected tests, for example -e 1,4,5.\n"
		"  -en: Execute test by name, for example -en \"My test X\"\n"
		"  -s: Show list of all tests\n"
		"  -C: Used parameter configuration file\n"
		"  -?: This message\n"
		"  -v: Verbose logging\n"
		"  -vv: Even more verbose logging\n%%s";
	int log_file_len = cli->log_file?strlen(cli->log_file):0;
	char *help_msg = malloc(strlen(help_msg_base) + strlen(bin_name) +
		log_file_len);

	sprintf(help_msg, help_msg_base, bin_name, cli->log_file);

	if(cli->blts_cli_help)
	{
		cli->blts_cli_help(help_msg);
	}

	free(help_msg);
}

static void invalid_arguments(char* bin_name, blts_cli* cli)
{
	fprintf(stderr, "Invalid arguments.\n");
	show_help(bin_name, cli);
}

static void save_and_clear_sigchld()
{
	struct sigaction default_sa =
	{
		.sa_handler = SIG_DFL
	};

	if(saved_sigchld_action) return; /* Already set */
	saved_sigchld_action = malloc(sizeof(struct sigaction));
	if(sigaction(SIGCHLD, &default_sa, saved_sigchld_action) == -1)
	{
		BLTS_ERROR("Error setting SIGCHLD handler (results may be inaccurate)\n");
	}
}

static void restore_sigchld()
{
	if(sigaction(SIGCHLD, saved_sigchld_action, NULL) == -1)
	{
		BLTS_ERROR("Error restoring SIGCHLD handler.\n");
	}
	else
	{
		free(saved_sigchld_action);
		saved_sigchld_action = NULL;
	}
}

static void signal_segv(int signum, siginfo_t* info, void *ptr)
{
	static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
	int i, f = 0;
	void **bp = 0;
	void *ip = 0;
	Dl_info dlinfo;
	ucontext_t *ucontext = (ucontext_t*)ptr;

	sigsegv_outp("Segmentation Fault!");
	sigsegv_outp("info.si_signo = %d", signum);
	sigsegv_outp("info.si_errno = %d", info->si_errno);
	sigsegv_outp("info.si_code  = %d (%s)", info->si_code,
		si_codes[info->si_code]);
	sigsegv_outp("info.si_addr  = %p", info->si_addr);

	/* TODO: Add support for ARM */
#ifndef SIGSEGV_NOSTACK
#if defined(SIGSEGV_STACK_IA64) || defined(SIGSEGV_STACK_X86)
	for(i = 0; i < NGREG; i++)
		sigsegv_outp("reg[%02d]	      = 0x" REGFORMAT, i,
			ucontext->uc_mcontext.gregs[i]);
#if defined(SIGSEGV_STACK_IA64)
	ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(SIGSEGV_STACK_X86)
	ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
#endif

	sigsegv_outp("Stack trace:");
	while(bp && ip)
	{
		if(!dladdr(ip, &dlinfo))
			break;

		const char *symname = dlinfo.dli_sname;

		sigsegv_outp("% 2d: %p <%s+%lu> (%s)", ++f, ip, symname,
			(unsigned long)ip - (unsigned long)dlinfo.dli_saddr, dlinfo.dli_fname);

		if(dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "main"))
			break;

		ip = bp[1];
		bp = (void**)bp[0];
	}
#else
	int sz;
	void *bt[20];
	char **strings;

	sigsegv_outp("Stack trace (non-dedicated):");
	sz = backtrace(bt, 20);
	strings = backtrace_symbols(bt, sz);
	for(i = 0; i < sz; ++i)
		sigsegv_outp("%s", strings[i]);
#endif
	sigsegv_outp("End of stack trace.");
#else
	sigsegv_outp("Not printing stack strace.");
#endif
	_exit (-1);
}

static void setup_sigsegv()
{
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = signal_segv;
	action.sa_flags = SA_SIGINFO;
	if(sigaction(SIGSEGV, &action, NULL) < 0)
		BLTS_LOGGED_PERROR("sigaction");
}

static int run_test(int testnum, blts_cli_testcase* testcase, void* user_ptr,
	int var_style, int stack_trace)
{
	int ret = 0;
	unsigned int time_elapsed = 0;
	unsigned int real_timeout = timout_override?timout_override:testcase->timeout;
	struct variant_list *test_variants = 0, *failed_variants = 0, *temp_var;
	struct boxed_value *variant_param_names = NULL;
	unsigned n_variations = 0;

	if (blts_config_test_is_variable(testcase->case_name)) {
		test_variants = blts_config_generate_test_variations(testcase->case_name, var_style);
		variant_param_names = blts_config_get_test_param_names(testcase->case_name);
	}

	pid_t pid;

	/* TODO: real implementation, for example;
	 * blts-ofono(1)-001:     Voice call..........................................[PASSED]
	 * blts-ofono(1)-017:     Emergency call......................................[PASSED]
	 * blts-ofono(1)-023-001: Multiparty call.....................................[FAILED]
	 * blts-ofono(1)-023:     Multiparty call.....................................[FAILED]
	 * blts-ofono(1)-011:     Send SMS............................................[PASSED]
	 * blts-ofono(1):         Final result........................................[FAILED]
	 * ...and [FAILED] with red color
	 **/
	fprintf(stdout, "\nStarting test '%d: %s'...\n", testnum, testcase->case_name);

	do {
		n_variations++;	  /* <- nb. this needs to be outside the forks */
		save_and_clear_sigchld();

		if((pid = fork()) < 0)
		{
			BLTS_ERROR("Failed to fork\n");
			return -1;
		}
		if(!pid)
		{
			if(stack_trace)
				setup_sigsegv();
			if(test_variants) {
				LOG("\n----------------------------------------------------------------\n");
				LOG("Variant %d, parameters: ", n_variations);
				blts_config_dump_labeled_value_list_on_log(variant_param_names,
									   test_variants->values);
				LOG("\n");
				/* Process test variation parameters */
				user_ptr = _blts_config_mutate_user_params(testcase->case_name, test_variants->values, user_ptr);
			}

			exit(testcase->case_func(user_ptr, testnum));
		}

		/* TODO: Implement better timeout */
		time_elapsed = 0;
		while(waitpid(pid, &ret, WNOHANG) != -1)
		{
			sleep(1);
			time_elapsed += 1000;
			if(real_timeout && time_elapsed > real_timeout)
			{
				kill(pid, SIGKILL);
				BLTS_ERROR("Test execution timed out.\n");
				ret = -1;
				break;
			}
		}
		if(ret)
		{
			if(WIFSIGNALED(ret))
			{
				BLTS_ERROR("Killed with signal %d (%s)\n", WTERMSIG(ret),
					strsignal(WTERMSIG(ret)));
			}
			fprintf(stdout, "Test failed.\n");
			ret = 1;
		}
		else
		{
			fprintf(stdout, "Test passed.\n");
			ret = 0;
		}

		restore_sigchld();
		/* Move to next variation: record errors, free current variant data */
		if (test_variants) {
			if (ret) {
				LOGERR("Test failed for values: ");
				blts_config_dump_labeled_value_list_on_log(variant_param_names,
									   test_variants->values);
				LOGERR("\n");
				temp_var = malloc(sizeof *temp_var);
				temp_var->values = blts_config_boxed_value_list_dup(test_variants->values);
				temp_var->next = failed_variants;
				failed_variants = temp_var;
			}
			temp_var = test_variants;
			test_variants = test_variants->next;
			while ((temp_var->values = blts_config_boxed_value_free(temp_var->values)));
			free(temp_var);
		}
	} while (test_variants);

	if (failed_variants) {
		BLTS_ERROR("Test failed for variations: \n");

		while (failed_variants) {
			LOGERR("\t");
			blts_config_dump_labeled_value_list_on_log(variant_param_names,
								   failed_variants->values);
			LOGERR("\n");
			temp_var = failed_variants;
			failed_variants = failed_variants->next;
			while ((temp_var->values = blts_config_boxed_value_free(temp_var->values)));
			free(temp_var);
		}
	}

	while(variant_param_names)
		variant_param_names = blts_config_boxed_value_free(variant_param_names);

	return ret;
}

static int find_test_by_name(blts_cli* cli, const char* name)
{
	int t = 0;

	while(cli->test_cases[t].case_name)
	{
		if(!strcmp(cli->test_cases[t].case_name, name))
			return t;
		t++;
	}
	return -1;
}

static int parse_test_list(const char* list, int* out)
{
	char tmp[8];
	unsigned int t, p, i;

	i = 0; p = 0;
	for(t = 0; t < strlen(list); t++)
	{
		if(list[t] == ',')
		{
			out[p++] = atoi(tmp);
			i = 0;

			if(p >= MAX_ARGS - 1)
			{
				break;
			}
		}
		else
		{
			if(i >= 8) return 0;
			tmp[i++] = list[t];
			tmp[i] = 0;
		}
	}

	if(i) out[p++] = atoi(tmp);
	out[p] = 0;
	return p;
}

static void list_all_tests(blts_cli* cli)
{
	int t = 0;

	while(cli->test_cases[t].case_name)
	{
		fprintf(stdout, "%d: %s\n", t + 1, cli->test_cases[t].case_name);
		t++;
	}
}

static int count_tests(blts_cli* cli)
{
	int t = 0;
	while(cli->test_cases[t].case_name) t++;
	return t;
}

void blts_cli_set_timeout(unsigned int timeout)
{
	timout_override = timeout;
}

/* Returns number of failed test cases, or < 0 if invalid arguments given
 * or something other fatal happens */
int blts_cli_main(blts_cli* cli, int argc, char **argv)
{
	int result = 0, t;
	int num_tests = 0;
	int* test_list = NULL;
	unsigned int log_flags = BLTS_LOG_FLAG_FILE;
	void* user_ptr = NULL;
	char* used_log_file = NULL;
	char* used_config_file = NULL;
	char** processed_argv = NULL;
	int processed_argc = 0;
	int log_want_trace = 0;
	int want_test_list = 0;
	int variant_run_mode = 0;
	int log_stack_trace = 0;
	int config_file_given = 0;

	timout_override = 0;

	/* Root required by logging alone */
	if(getuid())
	{
		fprintf(stderr, "You need to run this program as root.\n");
		result = -1;
		goto cleanup;
	}

	test_list = malloc(sizeof(int) * MAX_ARGS);
	if(!test_list)
	{
		result = -ENOMEM;
		goto cleanup;
	}

	processed_argv = malloc(sizeof(char*) * MAX_ARGS);
	if(!processed_argv)
	{
		result = -ENOMEM;
		goto cleanup;
	}

	processed_argv[processed_argc++] = argv[0];

	for(t = 1; t < argc; t++)
	{
		if(strcmp(argv[t], "-e") == 0)
		{
			if(++t >= argc)
			{
				invalid_arguments(argv[0], cli);
				result = -EINVAL;
				goto cleanup;
			}
			if(!(num_tests = parse_test_list(argv[t], test_list)))
			{
				invalid_arguments(argv[0], cli);
				result = -EINVAL;
				goto cleanup;
			}
		}
		else if(strcmp(argv[t], "-en") == 0)
		{
			if(++t >= argc)
			{
				invalid_arguments(argv[0], cli);
				result = -EINVAL;
				goto cleanup;
			}
			test_list[num_tests] = find_test_by_name(cli, argv[t]) + 1;
			if(!test_list[num_tests])
			{
				fprintf(stdout, "Invalid name, available tests are:\n");
				list_all_tests(cli);
				result = -EINVAL;
				goto cleanup;
			}
			num_tests++;
			test_list[num_tests] = 0;
		}
		else if(strcmp(argv[t], "-s") == 0)
		{
			want_test_list = 1;
		}
		else if(strcmp(argv[t], "-?") == 0)
		{
			show_help(argv[0], cli);
			result = 0;
			goto cleanup;
		}
		else if(strcmp(argv[t], "-l") == 0)
		{
			if(++t >= argc)
			{
				invalid_arguments(argv[0], cli);
				result = -EINVAL;
				goto cleanup;
			}
			used_log_file = argv[t];
		}
		else if(strcmp(argv[t], "-v") == 0)
		{
			log_flags |= BLTS_LOG_FLAG_STDOUT;
		}
		else if(strcmp(argv[t], "-vv") == 0)
		{
			log_flags |= BLTS_LOG_FLAG_STDOUT;
			log_want_trace = 1;
		}
		else if(strcmp(argv[t], "-strace") == 0)
		{
			log_stack_trace = 1;
		}
		else if(strcmp(argv[t], "-C") == 0)
		{
			if(++t >= argc)
			{
				invalid_arguments(argv[0], cli);
				result = -EINVAL;
				goto cleanup;
			}
			if(used_config_file)
			{
				fprintf(stderr, "Only one configuration file supported.\n");
				result = -EINVAL;
				goto cleanup;
			}
			used_config_file = strdup (argv[t]);
			config_file_given = 1;
		}
		else
		{
			if(processed_argc >= MAX_ARGS)
			{
				result = -EINVAL;
				goto cleanup;
			}
			processed_argv[processed_argc++] = argv[t];
		}
	}

	/* Open a log file specified by given argument or by the test module */
	if(used_log_file)
	{
		blts_log_open(used_log_file, log_flags);
	}
	else
	{
		if(cli->log_file)
		{
			blts_log_open(cli->log_file, log_flags);
		}
	}

	if(log_want_trace)
	{
		blts_log_set_level(LEVEL_TRACE);
	}

	/* If we didn't specify a config file, try to use the default */
	if (used_config_file == NULL)
	{
		char *program = argv[0];
		char *slash   = NULL;
		int   len     = 0;

		/* Invalid args */
		if (program == NULL)
		{
			result = -EINVAL;
			goto cleanup;
		}

		slash = strrchr (program, '/');

		if (slash != NULL)
		{
			slash++;
		}
		else
		{
			slash = program;
		}

		len = strlen (BLTS_CONFIG_DIR);
		len += strlen (slash);
		len += strlen (BLTS_CONFIG_SUFFIX);
		len += 2; /* Adding 2 to add a slash and ending char */

		used_config_file = malloc (sizeof (char) * len);
		if (used_config_file == NULL)
		{
			result = -ENOMEM;
			goto cleanup;
		}

		memset (used_config_file, 0, sizeof (char) * len);

		strcat (used_config_file, BLTS_CONFIG_DIR);
		strcat (used_config_file, "/");
		strcat (used_config_file, slash);
		strcat (used_config_file, BLTS_CONFIG_SUFFIX);

		BLTS_DEBUG ("No config file given, trying default: %s\n",
			    used_config_file);
	}

	result = blts_config_load_from_file (used_config_file);

	if (result == -ENOENT)
	{
		if (config_file_given == 0)
		{
			BLTS_WARNING ("No default configuration found. "
				      "Running with inbuilt parameters.\n");
		}
		else
		{
			BLTS_ERROR ("Did not find configuration file %s!\n",
				    used_config_file);
			goto cleanup;
		}
	}
	else
	{
		if (result < 0)
		{
			fprintf(stderr, "Could not load configuration file.\n");
			goto cleanup;
		}

		if(blts_config_add_testcases(&cli) < 0)
		{
			fprintf(stderr, "Could not add test cases from configuration file.\n");
			result = -EINVAL;
			goto cleanup;
		}
	}

	/* Include additions from config file parsing */
	if(want_test_list) {
		list_all_tests(cli);
		result = 0;
		goto cleanup;
	}

	/* Pass the araguments to test module */
	if(cli->blts_cli_process_arguments)
	{
		user_ptr = cli->blts_cli_process_arguments(processed_argc,
			processed_argv);
		if(!user_ptr)
		{
			invalid_arguments(argv[0], cli);
			result = -EINVAL;
			goto cleanup;
		}
	}


	/* Run the tests given with '-e' argument or all the cases
	 * specified by test module */
	if(num_tests)
	{
		t = 0;
		while(test_list[t])
		{
			if(test_list[t] > count_tests(cli) || test_list[t] < 1)
			{
				BLTS_ERROR("Test case '%d' does not exist. Skipping.\n",
					test_list[t]);
				result++;
			}
			else
			{
				result += run_test(test_list[t],
					&cli->test_cases[test_list[t] - 1], user_ptr, variant_run_mode,
					log_stack_trace);
			}
			t++;
		}
	}
	else
	{
		t = 0;
		while(cli->test_cases[t].case_name)
		{
			result += run_test(t + 1, &cli->test_cases[t], user_ptr,
				variant_run_mode, log_stack_trace);
			t++;
		}
	}

cleanup:

	/* All done, cleanup */
	if(cli->blts_cli_teardown)
	{
		cli->blts_cli_teardown(user_ptr);
	}

	if(used_config_file)
	{
		blts_config_free();
		free (used_config_file);
	}

	blts_log_close();

	if(processed_argv)
	{
		free(processed_argv);
	}

	if(test_list)
	{
		free(test_list);
	}

	return result;
}
