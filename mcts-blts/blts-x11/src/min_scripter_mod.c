/* min_scripter_mod.c -- Min scripter plugin interface for X11 tests

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

#include <test_module_api.h>
#include <min_parser.h>
#include <min_logger.h>
#include <min_test_event_if.h>
#include <min_ipc_mechanism.h>

#include <limits.h>

#include "blts_x11_util.h"
#include "xcomposite_tests.h"
#include "xdamage_tests.h"
#include "xinput_tests.h"
#include "xrandr_tests.h"
#include "xrecord_tests.h"
#include "xrender_tests.h"
#include "xtest_tests.h"
#include "xvideo_tests.h"


/* Min calls getters for these variables (see below): */
char *module_date = __DATE__;
char *module_time = __TIME__;
TTestModuleType module_type     = ETestClass;
unsigned int    module_version  = 200924;


/* Required definitions, for Min scripter interface. From Min template. */
/* snip >>>-------------------------------------------------------------*/

#ifdef ENTRYTC
#undef ENTRYTC
#endif
#define ENTRYTC(_l_, _n_,_f_)                                           \
        do {                                                            \
        TestCaseInfoTC* tc = (TestCaseInfoTC*)malloc(sizeof(TestCaseInfoTC));\
        if( tc == NULL ) break;                                         \
        STRCPY(tc->name_,_n_,MaxTestCaseName);                          \
        tc->test_ = _f_;                                                \
        tc->id_   = dl_list_size(_l_)+1;                                \
        dl_list_add( _l_, (void*)tc );                                  \
        } while(0)

typedef struct _TestCaseInfoTC TestCaseInfoTC;
typedef struct _ScriptVariable ScriptVariable;

typedef int (*ptr2testtc)( MinItemParser * tcr );

struct _TestCaseInfoTC
{
        char name_[MaxTestCaseName];
        ptr2testtc test_;
        unsigned int id_;
};

struct _ScriptVariable
{
        char *var_name_;
        TSBool is_initialized_;
        char *var_value_;
};

/* <<< snip ------------------------------------------------------------*/

/* Forward declarations */

int test_x11_presence(MinItemParser* args);
int test_create_simple_window(MinItemParser *args);
int test_enumerate_x_extensions(MinItemParser *args);
int test_xvideo_init(MinItemParser *args);
int test_xtest_move_cursor(MinItemParser *args);
int test_xcomposite_get_widow_contents(MinItemParser *args);
int test_xrecord_capture_mouse_movement(MinItemParser *args);
int test_xrender_draw_rectangle(MinItemParser *args);
int test_xrandr_rotate_screen(MinItemParser *args);
int test_xdamage_monitor_region(MinItemParser *args);
int test_xinput_list_devices(MinItemParser *args);
int test_xrenderbench(MinItemParser *args);

/* Required functions for a Min scripter plugin. --->>> */

/* Min calls this when it needs a list of test cases implemented by us. */
int ts_get_test_cases(DLList ** list)
{
	ENTRYTC(*list, "test_x11_presence", test_x11_presence);
	ENTRYTC(*list, "test_create_simple_window", test_create_simple_window);
	ENTRYTC(*list, "test_enumerate_x_extensions", test_enumerate_x_extensions);
	ENTRYTC(*list, "test_xvideo_init", test_xvideo_init);
	ENTRYTC(*list, "test_xtest_move_cursor", test_xtest_move_cursor);
	ENTRYTC(*list, "test_xcomposite_get_widow_contents", test_xcomposite_get_widow_contents);
	ENTRYTC(*list, "test_xrecord_capture_mouse_movement", test_xrecord_capture_mouse_movement);
	ENTRYTC(*list, "test_xrender_draw_rectangle", test_xrender_draw_rectangle);
	ENTRYTC(*list, "test_xrandr_rotate_screen", test_xrandr_rotate_screen);
	ENTRYTC(*list, "test_xdamage_monitor_region", test_xdamage_monitor_region);
	ENTRYTC(*list, "test_xinput_list_devices", test_xinput_list_devices);
	ENTRYTC(*list, "xrenderbench", test_xrenderbench);
	return 0;
}

/* Getters for module id data */
unsigned int get_module_type() { return module_type; }
unsigned int get_module_version() { return module_version; }
char* get_module_date() { return module_date; }
char* get_module_time() { return module_time; }

/* Comparison helper from Min template. */
int _look4callname (const void *a, const void *b)
{
        TestCaseInfoTC *tci = (TestCaseInfoTC *) a;
        return strcmp (tci->name_, (char *)b);
}

/* Min TC runner (from template, shm var passing removed) */
int ts_run_method (MinItemParser * item)
{
        DLList         *l;
        DLListIterator  it;
        int             retval;
        char           *callname;

        l = dl_list_create ();
        callname = INITPTR;

        retval = mip_get_next_string (item, &callname);
        if (retval != ENOERR) {
                retval = -1;
                goto EXIT;
        }

        ts_get_test_cases (&l);
        it = dl_list_find (dl_list_head (l)
                           , dl_list_tail (l)
                           , _look4callname, callname);

        if (it == DLListNULLIterator) {
                retval = -1;
                DELETE (callname);
                it = dl_list_head (l);
                while (it != DLListNULLIterator) {
                        free (dl_list_data (it));
                        dl_list_remove_it (it);
                        it = dl_list_head (l);
                }
                dl_list_free (&l);
                goto EXIT;
        }

        retval = ((TestCaseInfoTC *) dl_list_data (it))->test_ (item);

        DELETE (callname);
        it = dl_list_head (l);
        while (it != DLListNULLIterator) {
                free (dl_list_data (it));
                dl_list_remove_it (it);
                it = dl_list_head (l);
        }
        dl_list_free (&l);
EXIT:
        return retval;
}

static struct sigaction *saved_sigchld_action = NULL;

/**
 * Save old SIGCHLD handler so we can eg. call g_spawn_*
 */
void save_and_clear_sigchld()
{
	struct sigaction default_sa =
		{
			.sa_handler = SIG_DFL
		};

	if(saved_sigchld_action)
	{
		log_print(" *** SIGCHLD handler already set ***");
		return;
	}

	saved_sigchld_action = malloc(sizeof(struct sigaction));

	if(sigaction(SIGCHLD, &default_sa, saved_sigchld_action) == -1)
	{
		log_print(" !! Error setting SIGCHLD handler (test results may be inaccurate)");
	}
}


/**
 * Restore previously saved SIGCHLD handler
 */
void restore_sigchld()
{
	if(!saved_sigchld_action)
	{
		log_print(" *** SIGCHLD handler has not been saved (this is a bug) ***\n");
		return;
	}
	if(sigaction(SIGCHLD, saved_sigchld_action, NULL) == -1)
	{
		log_print(" !! Error restoring SIGCHLD handler.\n");
	}
	else
	{
		free(saved_sigchld_action);
		saved_sigchld_action = NULL;
	}
}

/* These are scripter-callable functions for individual components. >>> */

int test_x11_presence(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = x11_dep_check();
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_create_simple_window(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = create_simple_window(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_enumerate_x_extensions(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = enumerate_x_extensions();
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xvideo_init(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xvideo_init_test(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xtest_move_cursor(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xtest_move_cursor(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xcomposite_get_widow_contents(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xcomposite_get_widow_contents(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xrecord_capture_mouse_movement(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xrecord_capture_mouse_movement(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xrender_draw_rectangle(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xrender_draw_rectangle(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xrandr_rotate_screen(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xrandr_rotate_screen(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xdamage_monitor_region(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xdamage_monitor_region(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xinput_list_devices(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;

	log_open(logfile, 0);

	test_result = xinput_list_devices(execution_time);
	if(test_result)
	{
		log_print(" *** Test execution failed ***\n");
	}
	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

int test_xrenderbench(MinItemParser *args)
{
	char* logfile = NULL;
	int execution_time = 30;
	int test_result = 0;
	int exit_stat = 0;
	int test_num = 1;
	char cmd[PATH_MAX];

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_int(args, &execution_time) < 0) goto parse_error;
	if(mip_get_next_int(args, &test_num) < 0) goto parse_error;

	log_open(logfile, 0);

	sprintf(cmd, "blts-xrender-bench -c -l %s -e %d",  logfile, test_num);

	save_and_clear_sigchld();
	exit_stat = system(cmd);
	if(WIFEXITED(exit_stat))
	{
		int child_exit_val = WEXITSTATUS(exit_stat);
		if (child_exit_val != 0)
		{
			log_print("Child returned %d (test fails)\n", child_exit_val);
			test_result = -1;
		}
		else
		{
			log_print("Child finished successfully.\n");
		}
	}
	else
	{
		log_print("Child terminated unexpectedly.\n");
		test_result = -1;
		if(WIFSIGNALED(exit_stat))
		{
			log_print("Terminated on signal %d\n", WTERMSIG(exit_stat));
		}
		else
		{
			log_print(" !! Unknown termination reason !!\n");
		}
	}
	restore_sigchld();

	log_close();
	free(logfile);

	return test_result;

parse_error:
	tm_printf(1, "", "Parse error in %s",__FUNCTION__);
	if(logfile) free(logfile);
	return EINVAL;
}

