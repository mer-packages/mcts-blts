/* min_scripter_mod.c -- Min scripter plugin interface for GLES2 tests

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

#include "ogles2_helper.h"
#include "test_blitter.h"
#include "test_common.h"
#include "ogles2_conf_file.h"

/* Min calls getters for these variables (see below): */
char *module_date = __DATE__;
char *module_time = __TIME__;
TTestModuleType module_type     = ETestClass;
unsigned int    module_version  = 200924;

const char* config_filename = "/usr/lib/tests/blts-opengles2-tests/blts-opengles2-perf.cnf";

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
int simple_triangle_test(MinItemParser *args);
int enum_gl_extensions(MinItemParser *args);
int enum_egl_extensions(MinItemParser *args);
int enum_egl_configs(MinItemParser *args);
int polygon_count_test(MinItemParser *args);
int frag_shader_test(MinItemParser *args);
int vert_shader_test(MinItemParser *args);
int texels_per_second_test(MinItemParser *args);
int fillrate_test(MinItemParser *args);
int test_blit(MinItemParser *args);


/* Required functions for a Min scripter plugin. --->>> */

/* Min calls this when it needs a list of test cases implemented by us. */
int ts_get_test_cases(DLList ** list)
{
	ENTRYTC(*list, "test_simple_triangle_test", simple_triangle_test);
	ENTRYTC(*list, "test_enum_gl_extensions", enum_gl_extensions);
	ENTRYTC(*list, "test_enum_egl_extensions", enum_egl_extensions);
	ENTRYTC(*list, "test_enum_egl_configs", enum_egl_configs);
	ENTRYTC(*list, "test_polygons", polygon_count_test);
	ENTRYTC(*list, "test_frag_shader", frag_shader_test);
	ENTRYTC(*list, "test_vert_shader", vert_shader_test);
	ENTRYTC(*list, "test_texels", texels_per_second_test);
	ENTRYTC(*list, "test_fillrate", fillrate_test);
	ENTRYTC(*list, "test_blit", test_blit);
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


int read_execution_params(MinItemParser *args, test_execution_params* params)
{
	memset(params, 0, sizeof(test_execution_params));
	if(mip_get_next_int(args, &params->execution_time) < 0) return -1;
	if(mip_get_next_int(args, &params->w) < 0) return -1;
	if(mip_get_next_int(args, &params->h) < 0) return -1;
	if(read_config(config_filename, &params->config)) return -1;

	return 0;
}

/* These are scripter-callable functions for individual components. >>> */

/* Polygons per second -test
 * args: logfile execution_time width height
 */
int polygon_count_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_polygons(&params);
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

/* Fragment shader performance test
 * args: logfile execution_time width height
 */
int frag_shader_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_frag_shader(&params);
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

/* Vertex shader performance test
 * args: logfile execution_time width height
 */
int vert_shader_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_vert_shader(&params);
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

/* Texels per second test
 * args: logfile execution_time width height
 */
int texels_per_second_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_texels(&params);
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

/* Fillrate test
 * args: logfile execution_time width height
 */
int fillrate_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_fillrate(&params);
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

/* Draw and rotate simple triangle
 * args: logfile execution_time width height
 */
int simple_triangle_test(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_simple_tri(&params);
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

/* Enumrate and check GL extensions
 * args: logfile execution_time width height
 */
int enum_gl_extensions(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_enum_glextensions(&params);
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

/* Enumerate and check EGL extensions
 * args: logfile execution_time width height
 */
int enum_egl_extensions(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_enum_eglextensions(&params);
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

/* Enumerate and check EGL display configurations
 * args: logfile execution_time width height
 */
int enum_egl_configs(MinItemParser *args)
{
	char* logfile = NULL;
	test_execution_params params;
	int test_result;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;

	log_open(logfile, 0);

	test_result = test_enum_eglconfigs(&params);
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

/* blitter -tests
 * args: logfile execution_time width height test_num
 */
int test_blit(MinItemParser *args)
{
	char* logfile = NULL;
	int test_result;
	int test_num;
	test_execution_params params;
	const int flags[] =
	{
		0,
		T_FLAG_BLEND,
		T_FLAG_BLEND|T_FLAG_WIDGETS,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|T_FLAG_PARTICLES,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|T_FLAG_ROTATE|T_FLAG_PARTICLES,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|T_FLAG_ZOOM|T_FLAG_ROTATE|T_FLAG_PARTICLES,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|T_FLAG_ZOOM|T_FLAG_ROTATE|T_FLAG_BLUR|T_FLAG_PARTICLES,
		T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS|T_FLAG_VIDEO_WIDGETS,
		T_FLAG_CONVOLUTION|T_FLAG_BLEND|T_FLAG_WIDGETS|T_FLAG_WIDGET_SHADOWS
	};

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(read_execution_params(args, &params)) goto parse_error;
	if(mip_get_next_int(args, &test_num) < 0) goto parse_error;

	log_open(logfile, 0);

	params.flag = flags[test_num];
	test_result = test_blitter(&params);
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


