/* blts_params_local.h -- Local definitions for param handling/config mechanism

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
#ifndef BLTS_PARAMS_LOCAL_H
#define BLTS_PARAMS_LOCAL_H

#include "blts_params.h"

#define SYMBOL_TABLE_MAX 1024


struct symbol_table_entry; /* Forward declaration, used in a few structs */


/* The following structures here form the syntax tree built during parsing. */

/* Variation generated from list of discrete constant values */
struct param_constlist_args {
	struct boxed_value *values;    /* Linked list of values */
};

/* Variation generated from continuous range [low..high] */
struct param_range_args {
	struct boxed_value *low_bound;
	struct boxed_value *high_bound;
};

/* Variation generated from registered function */
struct param_generated_args {
	char *generator_fn_name;                              /* Declared after parsing by user */
	unsigned n_params;                                    /* Size of parameter array */
	struct symbol_table_entry **generator_fn_params_syms; /* Array of symbol table entries */
};

/* Possible parameter variation modes */
enum param_variation_mode {
	PARAM_MODE_CONSTLIST,
	PARAM_MODE_RANGE,
	PARAM_MODE_GENERATED
};

/* Parameter definition top-level struct.
   (type == CONFIG_PARAM_*, mode == PARAM_MODE_*) */
struct paramdef {
	int type;
	int mode;
	union {
		struct param_constlist_args *constlist_args;
		struct param_range_args *range_args;
		struct param_generated_args *generated_args;
	};
};

/* Parameter group top-level struct */
struct pgroupdef {
	unsigned n_params;                                /* Size of parameter array */
	struct symbol_table_entry **pgroup_params_syms;   /* Array of symbol table entries */
};

/* Possible test setting types */
enum test_setting_types {
	TEST_SETTING_PARAMLIST,
	TEST_SETTING_PGROUP,
	TEST_SETTING_FIX
};

/* Temporary override for parameter ("fix" keyword) */
struct fixed_value {
	struct symbol_table_entry *param;     /* Entry in symbol table we're overriding */
	struct boxed_value *value;            /* Overriding value */
};

/* A test setting (setting_type == TEST_SETTING_*) */
struct test_setting {
	int setting_type;
	union {
		struct pgroupdef *param_pgroup;          /* Paramlist stored in hidden pgroup */
		struct symbol_table_entry *pgroup;       /* User-defined pgroup */
		struct fixed_value *fix_value;           /* Value override */
	};
};

/* Test top-level struct */
struct testdef {
	unsigned n_settings;                    /* Size of settings array */
	struct test_setting **settings;
	struct boxed_value *param_defaults;     /* List of default parameters for test, declared
						   in test module before the test run. The types
						   here also record function parameter types. */
	struct boxed_value *param_tags;         /* Tag strings for args (supplied as above) */
	arg_handler_fn arg_handler;
	tagged_arg_handler_fn arg_handler2;
};

/* All the ways a testset can be executed */
enum testset_exec_type {
	TESTSET_EXEC_SERIAL,
	TESTSET_EXEC_PARALLEL,
	TESTSET_EXEC_THREADS
};

/* Testset top-level struct */
struct testsetdef {
	int exec_type;
	unsigned n_settings;
	struct test_setting **settings;
	unsigned n_tests;
	struct symbol_table_entry **tests;
};

/* User-registrable generator function, with generated arguments */
struct generator_fn {
	var_generator_func gen_func;                 /* user function */
};


/* Possible symbol types */
enum symbol_type {
	SYM_TYPE_NONE,
	SYM_TYPE_BOXED_VALUE,
	SYM_TYPE_PARAM,
	SYM_TYPE_PGROUP,
	SYM_TYPE_TEST,
	SYM_TYPE_TESTSET,
	SYM_TYPE_GENERATOR_FN
};


/* Symbol table entry.

   key -> unique name for symbol
   type -> value type (for the union)
   reserved -> just that (not used atm)
   list_next -> list pointer (when used during argument list parsing, =0 in table proper)

   The union is keyed on 'type', and contains data associated with the symbol.
 */
struct symbol_table_entry {
	char *key;
	int type;
	long reserved;
	struct symbol_table_entry *list_next;
	union {
		struct boxed_value *value;
		struct paramdef *param_definition;
		struct pgroupdef *pgroup_definition;
		struct testdef *test_definition;
		struct testsetdef *testset_definition;
		struct generator_fn *gen_fn_definition;
	};
};

/* Global (but not part of API in exported headers) functions used in the
   scanner and parser */

int _blts_conf_symbol_table_insert(struct symbol_table_entry *e);
struct symbol_table_entry *_blts_conf_symbol_table_entry_new();
struct symbol_table_entry *_blts_conf_symbol_table_lookup(char *key);
void _blts_conf_symbol_table_entry_free(struct symbol_table_entry *e);
char *_blts_conf_unquote_string_lit(char *str);

/* Internal parameter variation support */
void* _blts_config_mutate_user_params(char *testname, struct boxed_value *values, void* user_ptr);


/* Internal memory pool for the parser */
void *_blts_params_mempool_alloc(unsigned size);
void _blts_params_mempool_release(void);

#endif /* BLTS_PARAMS_LOCAL_H */
