/* blts_cli_frontend.h -- CLI handling

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

#ifndef BLTS_CLI_FRONTEND_H
#define BLTS_CLI_FRONTEND_H

#define BLTS_UNUSED_PARAM(a) (void)(a);
#define BLTS_CLI_END_OF_LIST { NULL, NULL, 0 }

typedef struct
{
	/* NULL terminated test case name. */
	char* case_name;

	/* Function that executes the test case. */ 
	int (*case_func)(void* user_ptr, int test_num);

	/* Timeout for the test case in ms. */
	unsigned int timeout;
} blts_cli_testcase;

typedef struct
{
	/* Array of all test cases. Mandatory. */
	blts_cli_testcase* test_cases;

	/* Prints out help msg. Optional, can be NULL.
	 * help_msg_base: use with;
	 * fprintf(stdout, help_msg_base, "[-j]", "-j: something"); */
	void (*blts_cli_help)(const char* help_msg_base);

	/* Process given arguments. Optional, can be NULL. */
	void* (*blts_cli_process_arguments)(int argc, char **argv);

	/* Called before exiting the program. Optional, can be NULL. */	
	void (*blts_cli_teardown)(void* user_ptr);
	
	/* Name of the log file. Optional, can be NULL. */
	char* log_file;
} blts_cli;

/* Call from your main() func */
int blts_cli_main(blts_cli* cli, int argc, char **argv);

/* Override timeout for all following test cases */
void blts_cli_set_timeout(unsigned int timeout);

#endif /* BLTS_CLI_FRONTEND_H */

