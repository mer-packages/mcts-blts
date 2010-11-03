/* blts_params.h -- Configuration file based parameter variation handling

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

#ifndef BLTS_PARAMS_H
#define BLTS_PARAMS_H

#include <blts_cli_frontend.h>

/* Options for variation generation */
enum config_variation_mode {
	CONFIG_VARIATION_COMBINATIONS,  /* Generate all combinations (likely slow) */
	CONFIG_VARIATION_ALLPAIRS,      /* Generate combinations with allpairs */
	CONFIG_VARIATION_FIRST,         /* Pick first value from each variable
					   (start of range, first mentioned in lists) */
	CONFIG_VARIATION_LAST           /* Pick last value from each variable */
};

/* Type codes for boxed values */
enum config_param_type {
	CONFIG_PARAM_INT,    /* C: int */
	CONFIG_PARAM_LONG,   /* C: long */
	CONFIG_PARAM_FLOAT,  /* C: float */
	CONFIG_PARAM_DOUBLE, /* C: double */
	CONFIG_PARAM_STRING, /* C: char * */
	CONFIG_PARAM_BOOL,   /* C: int */
	CONFIG_PARAM_NONE
};

/* Generic value (type == CONFIG_PARAM_*) */
struct boxed_value {
	int type;
	struct boxed_value *next; /* optional, for use in lists */
	union {
		long int_val;       /* INT, LONG, BOOL */
		char *str_val;      /* STRING */
		double float_val;   /* FLOAT, DOUBLE */
	};
};

/* Linked list for storing generated parameter variations */
struct variant_list {
	struct variant_list *next;
	struct boxed_value *values; /* Linked list of parameter values for this variation */
};

/* Type for argument handler function called once for each parameter variation
   when running variable case. Returned pointer is passed to test case as
   user_ptr argument, with null return as fail. The function is assumed to
   set up the passed pointer according to the passed parameter list.
*/
typedef void *(*arg_handler_fn)(struct boxed_value *args, void *user_ptr);

typedef void *(*tagged_arg_handler_fn)(struct boxed_value *arg_tags, struct boxed_value *args, void *user_ptr);

/* Type for user-supplied function that can be used to generate values for a parameter,
   given other varying parameter values in the configuration. Arguments are a linked
   list of boxed values, in same order as the definition in the configuration file.
 */
typedef struct boxed_value *(* var_generator_func)(struct boxed_value *arg_list);


/* Init/deinit (usually called from common CLI code) */
int blts_config_load_from_file(char *filename);
int blts_config_free();
int blts_config_add_testcases(blts_cli **cli);


/* Variation support */
int blts_config_declare_variable_test(char *name, arg_handler_fn arg_handler, ...);
int blts_config_declare_variable_test_dynamic(char* name, tagged_arg_handler_fn arg_handler);
int blts_config_register_generating_func(char *name, var_generator_func func);
int blts_config_test_is_variable(char* name);
int blts_config_debug_dump_variants(char * test, int style);
struct variant_list *blts_config_generate_test_variations(char *testname, int variation_style);

struct boxed_value *blts_config_get_test_param_names(char *variant_test_name);

/* Configuration reading ([global] variables) */
int blts_config_get_value_int(char *key, int *value);
int blts_config_get_value_long(char *key, long *value);
int blts_config_get_value_float(char *key, float *value);
int blts_config_get_value_double(char *key, double *value);
int blts_config_get_value_string(char *key, char **value);
int blts_config_get_value_bool(char *key, int *value);

/* Boxing/unboxing */
struct boxed_value *blts_config_boxed_value_new_int(long v);
struct boxed_value *blts_config_boxed_value_new_long(long v);
struct boxed_value *blts_config_boxed_value_new_bool(long v);
struct boxed_value *blts_config_boxed_value_new_float(double v);
struct boxed_value *blts_config_boxed_value_new_double(double v);
struct boxed_value *blts_config_boxed_value_new_string(char* v);
struct boxed_value *blts_config_boxed_value_new_string_take(char* v);

int blts_config_boxed_value_get_int(struct boxed_value *val);
long blts_config_boxed_value_get_long(struct boxed_value *val);
int blts_config_boxed_value_get_bool(struct boxed_value *val);
float blts_config_boxed_value_get_float(struct boxed_value *val);
double blts_config_boxed_value_get_double(struct boxed_value *val);
char *blts_config_boxed_value_get_string(struct boxed_value *val);

/* Boxed value / valuelist handling */
struct boxed_value *blts_config_boxed_value_free(struct boxed_value *p);
struct boxed_value *blts_config_boxed_value_dup(struct boxed_value *v);
struct boxed_value *blts_config_boxed_value_list_reverse(struct boxed_value *list);
struct boxed_value *blts_config_boxed_value_list_dup(struct boxed_value *v);

void blts_config_dump_boxed_value_list_on_loglevel(struct boxed_value *v, int loglevel);
void blts_config_dump_boxed_value_list_on_log(struct boxed_value *v);
void blts_config_dump_labeled_value_list_on_log(struct boxed_value *labels, struct boxed_value *v);

#endif
