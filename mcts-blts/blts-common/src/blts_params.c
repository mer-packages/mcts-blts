/* blts_params.c -- Configuration file based parameter variation handling

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

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "blts_params.h"
#include "blts_params_local.h"
#include "blts_log.h"
#include "blts_cli_frontend.h"

static struct symbol_table_entry **symbol_table;
static int symbol_table_top;

/* When this is zero, test runs are done with defaults and test declarations
   create new symbols */
static int config_load_complete;

extern int _blts_conf_parser_start(FILE *f);

/* --------------------------------------------------------------------- */
/* Boxed values */

/* Type codes for simple types */
#define BOXED_VALUE_TYPEOF(_type) __boxed_value_typeof_ ## _type ()
static int __boxed_value_typeof_int() { return CONFIG_PARAM_INT; };
static int __boxed_value_typeof_long() { return CONFIG_PARAM_LONG; };
static int __boxed_value_typeof_bool() { return CONFIG_PARAM_BOOL; };
static int __boxed_value_typeof_float() { return CONFIG_PARAM_FLOAT; };
static int __boxed_value_typeof_double() { return CONFIG_PARAM_DOUBLE; };

/* Typecase compatibility for simple types */
static int __boxed_value_can_cast(int type, int new_type)
{
	switch(new_type) {
	case CONFIG_PARAM_INT:
	case CONFIG_PARAM_LONG:
	case CONFIG_PARAM_BOOL:
		if (type == CONFIG_PARAM_INT
			|| type == CONFIG_PARAM_LONG
			|| type == CONFIG_PARAM_BOOL)
			return 1;
	case CONFIG_PARAM_FLOAT:
	case CONFIG_PARAM_DOUBLE:
		if (type == CONFIG_PARAM_FLOAT
			|| type == CONFIG_PARAM_DOUBLE)
			return 1;
	}
	return 0;
}

/* Constructor for simple types */
#define BOXED_VALUE_CONSTRUCT(_name, _arg_type, _type, _storage)	\
	struct boxed_value *blts_config_boxed_value_new_ ## _name ( _arg_type v) \
	{								\
		struct boxed_value *val;				\
		val = malloc(sizeof *val);				\
		if (val) {						\
			val->type = _type;				\
			val->_storage = v;				\
			val->next = 0;					\
		}							\
		return val;						\
	}

BOXED_VALUE_CONSTRUCT(int, long, CONFIG_PARAM_INT, int_val)
BOXED_VALUE_CONSTRUCT(long, long, CONFIG_PARAM_LONG, int_val)
BOXED_VALUE_CONSTRUCT(bool, long, CONFIG_PARAM_BOOL, int_val)
BOXED_VALUE_CONSTRUCT(float, double, CONFIG_PARAM_FLOAT, float_val)
BOXED_VALUE_CONSTRUCT(double, double, CONFIG_PARAM_FLOAT, float_val)

/* Unboxing for simple types. Checked typecast. */
#define BOXED_VALUE_GETTER(_rettype, _name, _type, _storage)		\
	_rettype blts_config_boxed_value_get_ ## _name (struct boxed_value *val) \
	{								\
		_rettype ret;						\
		assert(val);						\
		assert(__boxed_value_can_cast(val->type, BOXED_VALUE_TYPEOF(_name))); \
		ret = (_rettype) val->_storage;				\
		return ret;						\
	}

BOXED_VALUE_GETTER(int, int, CONFIG_PARAM_INT, int_val)
BOXED_VALUE_GETTER(long, long, CONFIG_PARAM_LONG, int_val)
BOXED_VALUE_GETTER(int, bool, CONFIG_PARAM_BOOL, int_val)
BOXED_VALUE_GETTER(float, float, CONFIG_PARAM_FLOAT, float_val)
BOXED_VALUE_GETTER(double, double, CONFIG_PARAM_DOUBLE, float_val)

/* String constructor, takes ownership of string (so don't free elsewhere) */
struct boxed_value *blts_config_boxed_value_new_string_take(char* v)
{
	struct boxed_value *val;
	val = malloc(sizeof *val);
	if (val) {
		val->type = CONFIG_PARAM_STRING;
		val->str_val = v;
		val->next = 0;
	}
	return val;
}

/* Return contained string (need to modify->make a copy) */
char* blts_config_boxed_value_get_string(struct boxed_value *val)
{
	assert(val);
	assert(val->type == CONFIG_PARAM_STRING);
	return val->str_val;
}

/* String constructor, copies the string given */
struct boxed_value *blts_config_boxed_value_new_string(char* v)
{
	return blts_config_boxed_value_new_string_take(strdup(v));
}



/* Copy given value (deep). Clears list links. */
struct boxed_value *blts_config_boxed_value_dup(struct boxed_value *v)
{
	struct boxed_value *val;
	if (!v)
		return 0;
	val = malloc(sizeof *val);
	memcpy(val, v ,sizeof *val);
	val->next = 0;
	if (v->type == CONFIG_PARAM_STRING)
		val->str_val = strdup(v->str_val);
	return val;
}

/* Copy given value, but also following list if any. */
struct boxed_value *blts_config_boxed_value_list_dup(struct boxed_value *v)
{
	struct boxed_value *val, *head;
	val = blts_config_boxed_value_dup(v);
	head = val;
	v = v->next;
	while (v) {
		val->next = blts_config_boxed_value_dup(v);
		val = val->next;
		v = v->next;
	}
	return head;
}

/* Free car, return cdr */
struct boxed_value *blts_config_boxed_value_free(struct boxed_value *p)
{
	struct boxed_value *next = 0;
	if (!p)
		return 0;
	if (p->next)
		next = p->next;
	if (p->type == CONFIG_PARAM_STRING && p->str_val)
		free(p->str_val);
	free(p);
	return next;
}

/* Reverse list, returning head of reversed list */
struct boxed_value *blts_config_boxed_value_list_reverse(struct boxed_value *list)
{
	struct boxed_value *prev = 0, *next;

	while (list) {
		next = list->next;
		list->next = prev;
		prev = list;
		list = next;
	}
	return prev;
}


/* --------------------------------------------------------------------- */
/* Symbol table sub-datatype handling: free helpers */

static void param_constlist_args_free(struct param_constlist_args *v)
{
	struct boxed_value *bv;
	if (!v)
		return;
	bv = v->values;
	while ((bv = blts_config_boxed_value_free(bv)));
	free(v);
}

static void param_range_args_free(struct param_range_args *v)
{
	if (!v)
		return;
	blts_config_boxed_value_free(v->low_bound);
	blts_config_boxed_value_free(v->high_bound);
	free(v);
}

static void param_generated_args_free(struct param_generated_args *v)
{
	if (!v)
		return;

	if (v->generator_fn_name)
		free(v->generator_fn_name);

	/* The values in the array are just entries in the symbol table */
	if (v->generator_fn_params_syms)
		free(v->generator_fn_params_syms);
	free(v);
}

static void paramdef_free(struct paramdef *v)
{
	if (!v)
		return;
	switch (v->mode) {
	case PARAM_MODE_CONSTLIST:
		param_constlist_args_free(v->constlist_args);
		break;
	case PARAM_MODE_RANGE:
		param_range_args_free(v->range_args);
		break;
	case PARAM_MODE_GENERATED:
		param_generated_args_free(v->generated_args);
		break;
	}
	free(v);
}

static void pgroupdef_free(struct pgroupdef *v)
{
	if (!v)
		return;

	/* The values in the array are just entries in the symbol table */
	if (v->pgroup_params_syms)
		free(v->pgroup_params_syms);
	free(v);
}

static void fixed_value_free(struct fixed_value *v)
{
	if (!v)
		return;
	if (v->value)
		blts_config_boxed_value_free(v->value);

	free(v);
}

static void test_setting_free(struct test_setting *v)
{
	if (!v)
		return;
	switch (v->setting_type) {
	case TEST_SETTING_PARAMLIST:
		pgroupdef_free(v->param_pgroup);
		break;
	case TEST_SETTING_FIX:
		fixed_value_free(v->fix_value);
		break;
	}
	free(v);
}

static void testdef_free(struct testdef *v)
{
	unsigned i;
	if (!v)
		return;
	if (v->settings) {
		if (v->n_settings)
			for(i = 0; i < v->n_settings; ++i)
				test_setting_free(v->settings[i]);
		free(v->settings);
	}
	if (v->param_defaults) {
		while((v->param_defaults =
			blts_config_boxed_value_free(v->param_defaults)));
	}
	if (v->param_tags) {
		while((v->param_tags =
			blts_config_boxed_value_free(v->param_tags)));
	}
	free(v);
}

static void testsetdef_free(struct testsetdef *v)
{
	unsigned i;
	if (!v)
		return;
	if (v->settings) {
		if (v->n_settings)
			for(i = 0; i < v->n_settings; ++i)
				test_setting_free(v->settings[i]);
		free(v->settings);
	}
	if (v->tests)
		free(v->tests);
	free(v);
}

static void generator_fn_free(struct generator_fn *v)
{
	if (!v)
		return;
	free(v);
}

/* --------------------------------------------------------------------- */
/* Tracing helpers (symbol table dump) */

static void dump_type(int t)
{
	switch(t) {
	case CONFIG_PARAM_STRING:
		BLTS_TRACE("string");
		break;
	case CONFIG_PARAM_INT:
		BLTS_TRACE("int");
		break;
	case CONFIG_PARAM_LONG:
		BLTS_TRACE("long");
		break;
	case CONFIG_PARAM_BOOL:
		BLTS_TRACE("boolean");
		break;
	case CONFIG_PARAM_FLOAT:
		BLTS_TRACE("float");
		break;
	case CONFIG_PARAM_DOUBLE:
		BLTS_TRACE("double");
		break;
	default:
		BLTS_TRACE("(UNKNOWN TYPE)");
	}
}

static void dump_boxed_value_level(struct boxed_value *v, int loglevel)
{
	unsigned len;
	if (!v) {
		blts_log_print_level(loglevel, "(nil)");
		return;
	}
	switch(v->type) {
	case CONFIG_PARAM_STRING:
		len = strlen(v->str_val);
		if (len < 80) {
			blts_log_print_level(loglevel, "\"%s\"",v->str_val);
		} else {
			blts_log_print_level(loglevel, "\"%.40s\"... (total %u characters)", v->str_val, len);
		}
		break;
	case CONFIG_PARAM_INT:
	case CONFIG_PARAM_LONG:
		blts_log_print_level(loglevel, "%ld",v->int_val);
		break;
	case CONFIG_PARAM_BOOL:
		blts_log_print_level(loglevel, "%s",(v->int_val)?"true":"false");
		break;
	case CONFIG_PARAM_FLOAT:
	case CONFIG_PARAM_DOUBLE:
		blts_log_print_level(loglevel, "%lf",v->float_val);
		break;
	default:
		blts_log_print_level(loglevel, "(UNKNOWN TYPE)");
	}
}

static void dump_boxed_value(struct boxed_value *v)
{
	dump_boxed_value_level(v, LEVEL_TRACE);
}

static void dump_boxed_value_log(struct boxed_value *v)
{
	unsigned len;
	if (!v) {
		BLTS_DEBUG("(nil)");
		return;
	}
	switch(v->type) {
	case CONFIG_PARAM_STRING:
		len = strlen(v->str_val);
		if (len < 80) {
			BLTS_DEBUG("\"%s\"",v->str_val);
		} else {
			BLTS_DEBUG("\"%.40s\"... (total %u characters)", v->str_val, len);
		}
		break;
	case CONFIG_PARAM_INT:
	case CONFIG_PARAM_LONG:
		BLTS_DEBUG("%ld",v->int_val);
		break;
	case CONFIG_PARAM_BOOL:
		BLTS_DEBUG("%s",(v->int_val)?"true":"false");
		break;
	case CONFIG_PARAM_FLOAT:
	case CONFIG_PARAM_DOUBLE:
		BLTS_DEBUG("%lf",v->float_val);
		break;
	default:
		BLTS_DEBUG("(UNKNOWN TYPE)");
	}
}

void blts_config_dump_boxed_value_list_on_loglevel(struct boxed_value *v, int loglevel)
{
	if (!v) {
		blts_log_print_level(loglevel, "[]");
		return;
	}
	blts_log_print_level(loglevel, "[");
	while (v) {
		dump_boxed_value_level(v, loglevel);
		if ((v = v->next))
			blts_log_print_level(loglevel, ", ");
	}
	blts_log_print_level(loglevel, "]");
}

static void dump_boxed_value_list(struct boxed_value *v)
{
	blts_config_dump_boxed_value_list_on_loglevel(v, LEVEL_TRACE);
}

void blts_config_dump_boxed_value_list_on_log(struct boxed_value *v)
{
	if (!v) {
		BLTS_DEBUG("[]");
		return;
	}
	BLTS_DEBUG("[");
	while (v) {
		dump_boxed_value_log(v);
		if ((v = v->next))
			BLTS_DEBUG(", ");
	}
	BLTS_DEBUG("]");
}

void blts_config_dump_labeled_value_list_on_log(struct boxed_value *labels, struct boxed_value *v)
{
	if (!v) {
		BLTS_DEBUG("[]");
		return;
	}
	BLTS_DEBUG("[");
	while (v) {
		if(labels) {
			BLTS_DEBUG("%s", blts_config_boxed_value_get_string(labels));
			labels = labels->next;
		}
		BLTS_DEBUG(":");
		dump_boxed_value_log(v);
		if ((v = v->next))
			BLTS_DEBUG(", ");
	}
	BLTS_DEBUG("]");
}

static void dump_param_generated_args(struct param_generated_args *g)
{
	unsigned i;
	if (!g) {
		BLTS_TRACE("(nil)");
		return;
	}
	if (g->generator_fn_name) {
		BLTS_TRACE("%s", g->generator_fn_name);
	}
	else {
		BLTS_TRACE("(nil)");
	}
	for (i = 0; i < g->n_params; ++i) {
		if (g->generator_fn_params_syms[i]) {
			BLTS_TRACE(" %s", g->generator_fn_params_syms[i]->key);
		}
		else {
			BLTS_TRACE(" (nil)");
		}
	}
}

static void dump_paramdef(struct paramdef *p)
{
	if (!p) {
		BLTS_TRACE("(nil)");
		return;
	}
	switch(p->mode) {
	case PARAM_MODE_CONSTLIST:
		BLTS_TRACE("const ");
		dump_boxed_value_list(p->constlist_args->values);
		break;
	case PARAM_MODE_RANGE:
		BLTS_TRACE("range (");
		dump_type(p->type);
		BLTS_TRACE(") ");
		dump_boxed_value(p->range_args->low_bound);
		BLTS_TRACE("..");
		dump_boxed_value(p->range_args->high_bound);
		break;
	case PARAM_MODE_GENERATED:
		BLTS_TRACE("generated ");
		dump_param_generated_args(p->generated_args);
		break;
	default:
		BLTS_TRACE("(UNKNOWN TYPE)");
	}
}

static void dump_pgroupdef(struct pgroupdef *p)
{
	unsigned i;
	if (!p) {
		BLTS_TRACE("(nil)");
		return;
	}
	for (i = 0; i < p->n_params; ++i) {
		if (p->pgroup_params_syms[i]) {
			BLTS_TRACE(" %s", p->pgroup_params_syms[i]->key);
		}
		else {
			BLTS_TRACE(" (nil)");
		}
	}
}

static void dump_fixed_value(struct fixed_value *v)
{
	if (!v) {
		BLTS_TRACE("(nil)");
		return;
	}
	if (v->param) {
		BLTS_TRACE("%s ", v->param->key);
	}
	else {
		BLTS_TRACE("(nil) ");
	}
	dump_boxed_value(v->value);
}

static void dump_test_setting(struct test_setting *s)
{
	if (!s) {
		BLTS_TRACE("(nil)");
		return;
	}
	switch (s->setting_type) {
	case TEST_SETTING_PARAMLIST:
		BLTS_TRACE("param ");
		dump_pgroupdef(s->param_pgroup);
		break;
	case TEST_SETTING_PGROUP:
		BLTS_TRACE("pgroup ");
		if (s->pgroup) {
			BLTS_TRACE("%s", s->pgroup->key);
		}
		else {
			BLTS_TRACE("(nil)");
		}
		break;
	case TEST_SETTING_FIX:
		BLTS_TRACE("fix ");
		dump_fixed_value(s->fix_value);
		break;
	default:
		BLTS_TRACE("(UNKNOWN TYPE)");
	}
}

static void dump_testdef(struct testdef *t)
{
	unsigned i;
	struct boxed_value *bv;
	if (!t) {
		BLTS_TRACE("(nil)");
		return;
	}
	if (!(t->n_settings))
		BLTS_TRACE("()");
	for (i = 0; i < t->n_settings; ++i) {
		BLTS_TRACE("\n\t");
		if (t->settings[i])
			dump_test_setting(t->settings[i]);
		else
			BLTS_TRACE("(nil)");
	}
	/* uncomment if needed */
	BLTS_TRACE("\n  Test declaration:");
	BLTS_TRACE("\n\tParam tags: ");
	dump_boxed_value_list(t->param_tags);
	BLTS_TRACE("\n\tParam defaults and types: [");
	bv = t->param_defaults;
	while(bv) {
		dump_boxed_value(bv);
		BLTS_TRACE(" :: ");
		dump_type(bv->type);
		if ((bv = bv->next))
			BLTS_TRACE(", ");
	}
	BLTS_TRACE("]\n\tArg handler: %p\n", t->arg_handler);
}

static void dump_testsetdef(struct testsetdef *t)
{
        unsigned i;
	if (!t) {
		BLTS_TRACE("(nil)");
		return;
	}
	if (t->n_settings)
		BLTS_TRACE("\n  Settings:");
	for (i = 0; i < t->n_settings; ++i) {
		BLTS_TRACE("\n\t");
		if (t->settings[i])
			dump_test_setting(t->settings[i]);
		else
			BLTS_TRACE("(nil)");
	}
	switch (t->exec_type) {
	case TESTSET_EXEC_SERIAL:
		BLTS_TRACE("\n  serial");
		break;
	case TESTSET_EXEC_PARALLEL:
		BLTS_TRACE("\n  parallel");
		break;
	case TESTSET_EXEC_THREADS:
		BLTS_TRACE("\n  threads");
		break;
	default:
		BLTS_TRACE("\n  (UNKNOWN TYPE)");
	}
	for (i = 0; i < t->n_tests; ++i) {
		if (t->tests[i]) {
			BLTS_TRACE(" \"%s\"", t->tests[i]->key);
		}
		else {
			BLTS_TRACE(" (nil)");
		}
	}
}

static void dump_generator_fn(struct generator_fn *g)
{
	if (!g) {
		BLTS_TRACE("(nil)");
		return;
	}
	BLTS_TRACE("(func)");
}

static void dump_symbol_table()
{
	int i;
	struct symbol_table_entry *e;
	BLTS_TRACE("********* Symbol table contents:\n");

	for (i=0; i < SYMBOL_TABLE_MAX; ++i) {
		if (!symbol_table[i])
			continue;
		e = symbol_table[i];
		BLTS_TRACE("(%d) %s :: ", i, e->key);
		switch(e->type) {
		case SYM_TYPE_NONE:
			BLTS_TRACE("(NO TYPE)\n");
			break;
		case SYM_TYPE_BOXED_VALUE:
			dump_type(e->value->type);
			BLTS_TRACE(" = ");
			dump_boxed_value(e->value);
			BLTS_TRACE("\n");
			break;
		case SYM_TYPE_PARAM:
			BLTS_TRACE("param = ");
			dump_paramdef(e->param_definition);
			BLTS_TRACE("\n");
			break;
		case SYM_TYPE_PGROUP:
			BLTS_TRACE("pgroup =");
			dump_pgroupdef(e->pgroup_definition);
			BLTS_TRACE("\n");
			break;
		case SYM_TYPE_TEST:
			BLTS_TRACE("test = ");
			dump_testdef(e->test_definition);
			BLTS_TRACE("\n");
			break;
		case SYM_TYPE_TESTSET:
			BLTS_TRACE("testset = ");
			dump_testsetdef(e->testset_definition);
			BLTS_TRACE("\n");
			break;
		case SYM_TYPE_GENERATOR_FN:
			BLTS_TRACE("generator_fn = ");
			dump_generator_fn(e->gen_fn_definition);
			BLTS_TRACE("\n");
			break;
		default:
			BLTS_TRACE("(UNKNOWN TYPE)\n");
		}
	}
	BLTS_TRACE("*********\n");
}

/* ---------------------------------------------------------------------- */
/* Utilities, used by parser and otherwise */

/* Remove quotes from string ends; return new string. */
/* Warning: this uses the internal memory pool */
/* TODO: proper unescaping */
char *_blts_conf_unquote_string_lit(char *str)
{
	char *p, *tail;
	unsigned len;
	if (!str )
		return 0;
	if (str[0] != '"')
		return 0;
	tail = strrchr(str,'"');
	len = tail - str;
	if (!len)
		return 0;   /* "" -> len = 1 */

	p = _blts_params_mempool_alloc(len);
	strncpy(p, &str[1], len);
	p[len-1] = '\0';
	return p;
}

struct symbol_table_entry *_blts_conf_symbol_table_entry_new()
{
	struct symbol_table_entry *e;

	e = malloc(sizeof *e);
	memset(e, 0, sizeof *e);
	return e;
}

void _blts_conf_symbol_table_entry_free(struct symbol_table_entry *e)
{
	if (!e)
		return;
	switch (e->type) {
	case SYM_TYPE_BOXED_VALUE:
		blts_config_boxed_value_free(e->value);
		break;
	case SYM_TYPE_PARAM:
		paramdef_free(e->param_definition);
		break;
	case SYM_TYPE_PGROUP:
		pgroupdef_free(e->pgroup_definition);
		break;
	case SYM_TYPE_TEST:
		testdef_free(e->test_definition);
		break;
	case SYM_TYPE_TESTSET:
		testsetdef_free(e->testset_definition);
		break;
	case SYM_TYPE_GENERATOR_FN:
		generator_fn_free(e->gen_fn_definition);
		break;
	default:
		break;
	}
	free(e->key);
	free(e);
}

/* Allocate symbol table; generally called only from ..config_load_from_file() */
static int symbol_table_init()
{
	if (symbol_table)
		return 0;

	symbol_table = malloc(SYMBOL_TABLE_MAX * sizeof *symbol_table);
	memset(symbol_table, 0, SYMBOL_TABLE_MAX * sizeof *symbol_table);
	symbol_table_top = 0;
	return 0;
}

/* Destroy symbol table and all linked entries. */
static void symbol_table_free()
{
	int i;
	if (!symbol_table)
		return;
	for (i = 0; i < SYMBOL_TABLE_MAX; ++i)
		_blts_conf_symbol_table_entry_free(symbol_table[i]);
	free(symbol_table);
	symbol_table = 0;
}

/* Return symbol table entry with given key, or null. */
struct symbol_table_entry *_blts_conf_symbol_table_lookup(char *key)
{
	assert(symbol_table);
	int i;

	for (i = 0; i < SYMBOL_TABLE_MAX; ++i) {
		if (!symbol_table[i])
			continue;
		if (!strcmp(key, symbol_table[i]->key))
			return symbol_table[i];
	}
	return 0;
}

/* Insert fully formed symbol to top of symbol table; <0 == error. */
int _blts_conf_symbol_table_insert(struct symbol_table_entry *e)
{
	if (symbol_table_top >= SYMBOL_TABLE_MAX)
		return -ENOMEM;
	if (!e)
		return -EINVAL;
	if (_blts_conf_symbol_table_lookup(e->key))
		return -EEXIST;

	symbol_table[symbol_table_top++] = e;
	return 0;
}

/* Sets up the symbol table in the no config case if necessary. The
   given test is inserted for "variation" generation (even if it only
   produces a single permutation. This is used when running variable
   tests on default values. */
static void setup_symbol_table_no_config(char *testname)
{
	struct symbol_table_entry *sym;
	struct testdef *td;
	if (!symbol_table)
		symbol_table_init();

	sym = _blts_conf_symbol_table_entry_new();
	sym->key = strdup(testname);
	sym->type = SYM_TYPE_TEST;
	td = malloc(sizeof *td);
	memset(td, 0, sizeof *td);
	sym->test_definition = td;

	_blts_conf_symbol_table_insert(sym);
}

/* Parameter insertion with default value. No change to already defined values.
   Parameter link is added to test. Used in no-config default-only case.
 */
static void symbol_table_add_param_for_test(char *paramname, char *testname)
{
	struct test_setting *set;
	struct test_setting **newsettings;
	struct symbol_table_entry *test, *param;
	assert(paramname && testname);
	param = _blts_conf_symbol_table_lookup(paramname);
	test = _blts_conf_symbol_table_lookup(testname);
	assert(param && test);
	assert(param->type == SYM_TYPE_PARAM);
	assert(test->type == SYM_TYPE_TEST);

	set = malloc(sizeof *set);
	memset(set, 0, sizeof *set);
	set->setting_type = TEST_SETTING_PARAMLIST;

	set->param_pgroup = malloc(sizeof (struct pgroupdef));
	set->param_pgroup->n_params = 1;
	set->param_pgroup->pgroup_params_syms = malloc(1 * sizeof (struct symbol_table_entry));
	set->param_pgroup->pgroup_params_syms[0] = param;

	test->test_definition->n_settings++;

	newsettings = malloc(test->test_definition->n_settings * sizeof (struct test_setting));
	memcpy(newsettings, test->test_definition->settings,
		(test->test_definition->n_settings - 1) * sizeof (struct test_setting));
	newsettings[test->test_definition->n_settings - 1] = set;

	free(test->test_definition->settings);
	test->test_definition->settings = newsettings;
}

/* Parameter insertion with default value. No change to already defined values.
   Parameter link is added to test. Used in no-config default-only case.
 */
static void symbol_table_insert_param_with_default(char *paramname, char *testname, struct boxed_value *value)
{
	struct symbol_table_entry *sym;

	assert(symbol_table);
	assert(paramname);
	assert(value);
	assert(testname);

	sym = _blts_conf_symbol_table_lookup(paramname);
	if (!sym) {
		sym = _blts_conf_symbol_table_entry_new();
		sym->key = strdup(paramname);
		sym->type = SYM_TYPE_PARAM;
		sym->param_definition = malloc(sizeof (struct paramdef));
		memset(sym->param_definition, 0, sizeof (struct paramdef));
		sym->param_definition->type = value->type;
		sym->param_definition->mode = PARAM_MODE_CONSTLIST;
		sym->param_definition->constlist_args = malloc(sizeof (struct param_constlist_args));
		sym->param_definition->constlist_args->values = blts_config_boxed_value_dup(value);
		_blts_conf_symbol_table_insert(sym);
	}

	symbol_table_add_param_for_test(paramname, testname);
}


/* Top level function for config parsing; parses configuration
   starting from given file. Usually called from CLI code in
   common. 0 == ok. */
int blts_config_load_from_file(char *filename)
{
	FILE *f;
	int ret = 0;

	f = fopen(filename, "r");
	if (!f)
		return -errno;

	if (!symbol_table)
		ret = symbol_table_init();
	if (ret) {
		fclose(f);
		return ret;
	}

	ret = _blts_conf_parser_start(f);

	// dump_symbol_table(); /* output symbols to trace if enabled */

	fclose(f);
	if (!ret)
		config_load_complete = 1;
	return ret;
}

/* Top level finalize; currently, drops and frees symbol table. */
int blts_config_free()
{
	symbol_table_free();
	config_load_complete = 0;
	symbol_table = 0;
	symbol_table_top = 0;
	return 0;
}

/* Create test cases from test sets defined in config. Return count, <0 on error */
/* FIXME: make this actually do something */
static int config_generate_testset_tests(blts_cli_testcase **cases)
{
	if (!cases)
		return -EINVAL;

	BLTS_TRACE("** %s not implemented yet\n",__func__);
	return 0;
}

/* Adds testsets to listing in the cli structure */
int blts_config_add_testcases(blts_cli **cli)
{
	blts_cli_testcase *cases;
	blts_cli_testcase *new_cases = 0;
	unsigned n_cases;
	int ret;

	if (!cli || !(*cli) || !((*cli)->test_cases))
		return -EINVAL;

	cases = (*cli)->test_cases;
	n_cases = 0;
	while(cases[n_cases].case_name)
		n_cases++;

	BLTS_TRACE("%u cases defined in module\n",n_cases);

	ret = config_generate_testset_tests(&new_cases);
	if (ret < 0) {
		BLTS_ERROR("Failure during test case generation.\n");
		return ret;
	}
	BLTS_TRACE("%d cases defined as testsets in configuration file.\n", ret);

	/* FIXME: this is never freed ATM; fix. */
	cases = malloc((n_cases + ret + 1) * sizeof *cases);
	memcpy(cases, (*cli)->test_cases, n_cases * sizeof *cases);
	memcpy(&(cases[n_cases]), new_cases, ret * sizeof *cases);
	memset(&(cases[n_cases + ret]), 0, sizeof *cases);
	(*cli)->test_cases = cases;

	return 0;
}


/* ---------------------------------------------------------------------- */
/* Return named values from symbol table, eg. globals. (Note that these can
   be used as poor man's config file handling, in absence of defined tests.)
   Each returns 0 on success, <0 on fail; pointed value is only touched on
   success.
*/

int blts_config_get_value_int(char *key, int *value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_INT
		&& e->value->type != CONFIG_PARAM_LONG
		&& e->value->type != CONFIG_PARAM_BOOL)
		return -ENOENT;

	*value = (int) e->value->int_val;
	return 0;
}

int blts_config_get_value_bool(char *key, int *value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_INT
		&& e->value->type != CONFIG_PARAM_LONG
		&& e->value->type != CONFIG_PARAM_BOOL)
		return -ENOENT;
	*value = !! (int) e->value->int_val;
	return 0;
}

int blts_config_get_value_long(char *key, long *value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_INT
		&& e->value->type != CONFIG_PARAM_LONG
		&& e->value->type != CONFIG_PARAM_BOOL)
		return -ENOENT;
	*value = e->value->int_val;
	return 0;
}

int blts_config_get_value_float(char *key, float *value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_FLOAT
		&& e->value->type != CONFIG_PARAM_DOUBLE)
		return -ENOENT;

	*value = (float) e->value->float_val;
	return 0;
}

int blts_config_get_value_double(char *key, double *value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_FLOAT
		&& e->value->type != CONFIG_PARAM_DOUBLE)
		return -ENOENT;

	*value = e->value->float_val;
	return 0;
}

int blts_config_get_value_string(char *key, char **value)
{
	struct symbol_table_entry *e;

	if (!key || !value)
		return -EINVAL;

	e = _blts_conf_symbol_table_lookup(key);
	if (!e)
		return -ENOENT;
	if (e->type != SYM_TYPE_BOXED_VALUE)
		return -ENOENT;
	if (e->value->type != CONFIG_PARAM_STRING)
		return -ENOENT;
	*value = strdup(e->value->str_val);
	return 0;
}

/* ---------------------------------------------------------------------- */
/* Test parameter variation support */

/* List of parameters a test accepts, used below */
struct test_param_list {
	struct test_param_list *next;
	struct symbol_table_entry *param;
	struct boxed_value *values;
};

/* Table for generated permutations */
struct var_permute_table {
	unsigned n_variables;
	unsigned n_permutations;      /* Total number of generated permutations */
	unsigned *var_max_indices;    /* -> 0..n_variables - 1 sizes for variable arrays */
	unsigned *permutations;       /* -> generated permutations, as indices to arrays
					    below. Stride size == n_variables. */
};

/* Lists of generated values converted to arrays */
struct var_parameter_arrays {
	unsigned n_params;
	unsigned *n_values;          /* column sizes for values */
	struct boxed_value ***values; /* v[param][variation] */
};

static void var_permute_table_free(struct var_permute_table *table)
{
	if (!table)
		return;
	if (table->var_max_indices)
		free(table->var_max_indices);
	if (table->permutations)
		free(table->permutations);
	free(table);
}

static void var_parameter_arrays_free(struct var_parameter_arrays *arrs)
{
	unsigned i, j;
	if (!arrs)
		return;
	if (arrs->n_values && arrs->values) {
		for (i = 0; i < arrs->n_params; ++i) {
			for (j = 0; j < arrs->n_values[i]; ++j) {
				blts_config_boxed_value_free(arrs->values[i][j]);
			}
			free(arrs->values[i]);
		}
		free(arrs->values);
		free(arrs->n_values);
	}
	free(arrs);
}

/* static void debug_dump_param_arrs(struct var_parameter_arrays *arrs) */
/* { */
/* 	unsigned i, j; */
/* 	BLTS_TRACE("++ Param array %p:\n",arrs); */
/* 	for (i = 0; i < arrs->n_params; ++i) { */
/* 		BLTS_TRACE("%.3d: ",i); */
/* 		for(j = 0; j < arrs->n_values[i]; ++j) { */
/* 			dump_boxed_value(arrs->values[i][j]); */
/* 			BLTS_TRACE(", "); */
/* 		} */
/* 		BLTS_TRACE("\n"); */
/* 	} */
/* } */

static struct test_param_list *test_param_list_reverse(struct test_param_list *list)
{
	struct test_param_list *prev = 0, *next;

	while (list) {
		next = list->next;
		list->next = prev;
		prev = list;
		list = next;
	}
	return prev;
}

static void test_param_list_free(struct test_param_list *list)
{
	struct test_param_list *next;
	while (list) {
		next = list->next;
		while((list->values = blts_config_boxed_value_free(list->values)));
		free(list);
		list = next;
	}
}

static struct var_parameter_arrays *test_param_list_flatten(struct test_param_list *params)
{
	unsigned i, j;
	struct var_parameter_arrays *arrs;
	struct test_param_list *temp;
	struct boxed_value *bv;

	arrs = malloc(sizeof *arrs);
	temp = params;

	arrs->n_params = 0;
	while (temp) {
		arrs->n_params++;
		temp = temp->next;
	}

	arrs->n_values = malloc(arrs->n_params * sizeof (unsigned *));
	memset(arrs->n_values, 0, arrs->n_params * sizeof (unsigned *));

	arrs->values = malloc(arrs->n_params * sizeof (struct boxed_value **));

	temp = params;
	for (i = 0; i < arrs->n_params; ++i) {
		bv = temp->values;
		while (bv) {
			arrs->n_values[i]++;
			bv = bv->next;
		}
		arrs->values[i] = malloc(arrs->n_values[i] * sizeof (struct boxed_value *));

		bv = temp->values;
		for (j = 0; j < arrs->n_values[i]; ++j) {
			arrs->values[i][j] = blts_config_boxed_value_dup(bv);
			bv = bv->next;
		}
		temp = temp->next;
	}
	return arrs;
}

/* Generate lexicographically next permutation from given permutation. Note that
   this just wraps around if called too many times. */
static void generate_next_permutation(unsigned n_vars, unsigned *permutation, unsigned *limits)
{
	unsigned i;
	for (i = 0; i < n_vars; ++i) {
		if (permutation[i] + 1 < limits[i]) {
			permutation[i]++;
			return;
		}
		permutation[i] = 0;
	}
}

/* Create all permutations of available variables.
   FIXME: This could be done better, even fitting large permutation counts in memory...
*/
static struct var_permute_table *generate_permutation_table(struct var_parameter_arrays *arrs)
{
	struct var_permute_table *perms;
	unsigned i, perm_table_sz, perm_sz;
	unsigned *current_permutation;
	unsigned *permutations;

	/* setup arrays */
	perms = malloc(sizeof *perms);
	perms->n_variables = arrs->n_params;
	perm_sz = perms->n_variables * sizeof(unsigned);

	perms->var_max_indices = malloc(perm_sz);
	memcpy(perms->var_max_indices, arrs->n_values, perm_sz);

	perms->n_permutations = 1;
	for (i = 0; i < perms->n_variables; ++i)
		perms->n_permutations *= perms->var_max_indices[i] ? perms->var_max_indices[i] : 1;

	perm_table_sz = perms->n_permutations * perm_sz;
	permutations = malloc(perm_table_sz);

	current_permutation = malloc(perm_sz);
	memset(current_permutation, 0, perm_sz);

	for (i = 0; i < perms->n_permutations; ++i) {
		memcpy(&permutations[i * perms->n_variables], current_permutation, perm_sz);
		generate_next_permutation(perms->n_variables, current_permutation, perms->var_max_indices);
	}
	free(current_permutation);
	perms->permutations = permutations;

	return perms;
}

/* Create parameter set for each index permutation from variables in param_arrs. */
static struct variant_list *generate_permutation_variants(struct var_permute_table *perms,
	struct var_parameter_arrays *param_arrs)
{
	struct variant_list *varlist = 0, *work;
	struct boxed_value *bv;
	unsigned i, j;

	for (i = 0; i < perms->n_permutations; ++i) {
		work = malloc(sizeof *work);
		work->next = varlist;
		varlist = work;
		work->values = 0;

		for (j = 0; j < perms->n_variables; ++j) {
			bv = blts_config_boxed_value_dup(param_arrs->values[j][perms->permutations[i * perms->n_variables + j]]);
			bv->next = work->values;
			work->values = bv;
		}
		work->values = blts_config_boxed_value_list_reverse(work->values);
	}
	return varlist;
}

/* Walk the test settings and find out the supported parameter list.
   Return 0 on success, with results returned in lists *test_params,
   *fixed_params, *fixed_param_values (set to 0 if not interested).
 */
static int collect_test_params(char *testname, struct test_param_list **test_params,
	struct test_param_list **fixed_params, struct boxed_value **fixed_param_values)
{
	struct symbol_table_entry *sym;
	struct test_param_list *params_list = 0, *fix_params_list = 0, *li_work;
	struct boxed_value *fix_values_list = 0, *bv_work;
	struct testdef *test;
	struct pgroupdef *pgroup;

	unsigned i, j;

	sym = _blts_conf_symbol_table_lookup(testname);
	if (!sym || sym->type != SYM_TYPE_TEST) {
		BLTS_ERROR("No test \"%s\" in configuration.\n", testname);
		return -ENOENT;
	}
	test = sym->test_definition;

	if (!test->n_settings) {
		BLTS_WARNING("No parameters defined for test \"%s\".\n", testname);
		return -ENOENT;
	}
	for (i = 0; i < test->n_settings; ++i) {
		li_work = 0;
		pgroup = 0;
		if (!test->settings[i]) {
			BLTS_ERROR("BUG: Invalid parameter (%s param #%u).\n", testname, i);
			return -EINVAL;
		}
		switch (test->settings[i]->setting_type) {
		case TEST_SETTING_PARAMLIST:
			pgroup = test->settings[i]->param_pgroup;
			break;

		case TEST_SETTING_PGROUP:
			pgroup = test->settings[i]->pgroup->pgroup_definition;
			break;

		case TEST_SETTING_FIX:
			li_work = malloc(sizeof *li_work);
			li_work->param = test->settings[i]->fix_value->param;
			li_work->next = fix_params_list;
			li_work->values = 0;
			fix_params_list = li_work;

			bv_work = blts_config_boxed_value_dup(test->settings[i]->fix_value->value);
			bv_work->next = fix_values_list;
			fix_values_list = bv_work;

			continue;

		default:
			BLTS_ERROR("BUG: Invalid setting type in config for \"%s\".\n", testname);
			return -EINVAL;
		}
		/* handle the pgroup */
		if (!pgroup) {
			BLTS_ERROR("BUG: Invalid parameter or parameter group in \"%s\".\n", testname);
			return -EINVAL;
		}
		for (j = 0; j < pgroup->n_params; ++j) {
			li_work = malloc(sizeof *li_work);
			li_work->param = pgroup->pgroup_params_syms[j];
			li_work->next = params_list;
			li_work->values = 0;
			params_list = li_work;
		}
	}

	params_list = test_param_list_reverse(params_list);
	fix_params_list = test_param_list_reverse(fix_params_list);
	fix_values_list = blts_config_boxed_value_list_reverse(fix_values_list);

/* 	BLTS_TRACE(":: Test \"%s\" params:\n\t[", testname); */
/* 	li_work = params_list; */
/* 	while (li_work) { */
/* 		BLTS_TRACE("%s", li_work->param->key); */
/* 		if ((li_work = li_work->next)) */
/* 			BLTS_TRACE(", "); */
/* 	} */
/* 	BLTS_TRACE("]\n::   Fixed:\n\t["); */
/* 	li_work = fix_params_list; */
/* 	while (li_work) { */
/* 		BLTS_TRACE("%s", li_work->param->key); */
/* 		if ((li_work = li_work->next)) */
/* 			BLTS_TRACE(", "); */
/* 	} */
/* 	BLTS_TRACE("]\n"); */
	if (test_params)
		*test_params = params_list;
	if (fixed_params)
		*fixed_params = fix_params_list;
	if (fixed_param_values)
		*fixed_param_values = fix_values_list;
	return 0;
}

static struct boxed_value *generate_constlist_variations(struct param_constlist_args *source, int variation_style)
{
	struct boxed_value *list = 0, *values, *temp;

	switch (variation_style) {
	case CONFIG_VARIATION_COMBINATIONS:
	case CONFIG_VARIATION_ALLPAIRS:
		values = source->values;
		while (values) {
			temp = blts_config_boxed_value_dup(values);
			temp->next = list;
			list = temp;
			values = values->next;
		}
		break;
	case CONFIG_VARIATION_FIRST:
		list = blts_config_boxed_value_dup(source->values);
		break;
	case CONFIG_VARIATION_LAST:
		values = source->values;
		while (values && values->next) {
			values = values->next;
		}
		list = blts_config_boxed_value_dup(values);
		break;
	default:
		BLTS_ERROR("BUG: Unknown variation style (%d)\n", variation_style);
		return 0;
	}
	return blts_config_boxed_value_list_reverse(list);
}

static struct boxed_value *generate_range_variations(struct param_range_args *source, int variation_style)
{
	struct boxed_value *list = 0;

	switch (variation_style) {
	case CONFIG_VARIATION_COMBINATIONS:
	case CONFIG_VARIATION_ALLPAIRS:
		list = blts_config_boxed_value_dup(source->low_bound);
		list->next = blts_config_boxed_value_dup(source->high_bound);
		break;
	case CONFIG_VARIATION_FIRST:
		list = blts_config_boxed_value_dup(source->low_bound);
		break;
	case CONFIG_VARIATION_LAST:
		list = blts_config_boxed_value_dup(source->high_bound);
		break;
	default:
		BLTS_ERROR("BUG: Unknown variation style (%d)\n", variation_style);
		return 0;
	}
	return list;
}

static struct variant_list *variant_list_reverse(struct variant_list *list)
{
	struct variant_list *prev = 0, *next;

	while (list) {
		next = list->next;
		list->next = prev;
		prev = list;
		list = next;
	}
	return prev;
}

/* Forward declaration for recursive calls */
static struct boxed_value *generate_variations(struct symbol_table_entry *sym, int variation_style);

static struct boxed_value *generate_generated_variations(struct param_generated_args *source, int variation_style)
{
	struct boxed_value *list = 0, *temp_bv;
	struct symbol_table_entry *sym;
	var_generator_func gen_func;
	struct test_param_list *params = 0, *temp_param = 0;
	unsigned i;
	struct var_permute_table *perms = 0;
	struct var_parameter_arrays *param_arrs = 0;
	struct variant_list *variants, *temp_variant;

	sym = _blts_conf_symbol_table_lookup(source->generator_fn_name);
	if (!sym || sym->type != SYM_TYPE_GENERATOR_FN) {
		BLTS_ERROR("Error: No generating function \"%s\".\n", source->generator_fn_name);
		return 0;
	}
	gen_func = sym->gen_fn_definition->gen_func;

	for (i = 0; i < source->n_params; ++i) {
		temp_param = malloc(sizeof *temp_param);
		memset(temp_param, 0, sizeof *temp_param);
		temp_param->param = source->generator_fn_params_syms[i];
		temp_param->values = generate_variations(temp_param->param, variation_style);
		temp_param->next = params;
		params = temp_param;
	}
	params = test_param_list_reverse(params);

	param_arrs = test_param_list_flatten(params);
	if (!param_arrs) {
		BLTS_ERROR("BUG: Failed preparing permutations\n");
		goto done;
	}
	perms = generate_permutation_table(param_arrs);
	if (!param_arrs) {
		BLTS_ERROR("BUG: Permutation generation failed\n");
		goto done;
	}
	variants = variant_list_reverse(generate_permutation_variants(perms, param_arrs));
	if (!variants) {
		BLTS_ERROR("BUG: Failed linking permuted tables to parameter values\n");
		goto done;
	}
	while (variants) {
		temp_bv = blts_config_boxed_value_dup(gen_func(variants->values));
		temp_bv->next = list;
		list = temp_bv;
		while ((variants->values = blts_config_boxed_value_free(variants->values)));
		temp_variant = variants;
		variants = variants->next;
		free(temp_variant);
	}
done:
	test_param_list_free(params);
	var_parameter_arrays_free(param_arrs);
	var_permute_table_free(perms);

	return blts_config_boxed_value_list_reverse(list);
}

static struct boxed_value *generate_variations(struct symbol_table_entry *sym, int variation_style)
{
	struct boxed_value *list = 0;

	if (!sym || sym->type != SYM_TYPE_PARAM) {
		BLTS_ERROR("BUG: Invalid parameter in %s.\n", __func__);
		return 0;
	}
	switch (sym->param_definition->mode) {
	case PARAM_MODE_CONSTLIST:
		list = generate_constlist_variations(sym->param_definition->constlist_args, variation_style);
		break;
	case PARAM_MODE_RANGE:
		list = generate_range_variations(sym->param_definition->range_args, variation_style);
		break;
	case PARAM_MODE_GENERATED:
		list = generate_generated_variations(sym->param_definition->generated_args, variation_style);
		break;
	default:
		BLTS_ERROR("BUG: Unrecognised parameter variation mode (%d)\n", sym->param_definition->mode);
		return 0;
	}
	return list;
}

/* return fixed value if head of param is in fixed_params list, null if not */
static struct boxed_value *get_fixed_param_value(struct test_param_list *param,
	struct test_param_list *fixed_params, struct boxed_value *fixed_values)
{
	if (!param || !fixed_params || !fixed_values)
		return 0;
	while (fixed_params && fixed_values) {
		if (!strcmp(param->param->key, fixed_params->param->key))
			return blts_config_boxed_value_dup(fixed_values);
		fixed_params = fixed_params->next;
		fixed_values = fixed_values->next;
	}
	return 0;
}

/* Parameter variation generation */
struct variant_list *blts_config_generate_test_variations(char *testname, int variation_style)
{
	struct test_param_list *params = 0, *fixed_params = 0, *temp_param = 0;
	struct boxed_value *fixed_values = 0;
	struct variant_list *variants = 0;
	struct var_permute_table *perms = 0;
	struct var_parameter_arrays *param_arrs = 0;
	int ret;

	ret = collect_test_params(testname, &params, &fixed_params, &fixed_values);

	if (ret) {
		BLTS_ERROR("Error determining test parameters\n");
		goto done;
	}

	/* Generate individual parameter variations */
	temp_param = params;
	while (temp_param) {
		/* check if this parameter was fixed, if not, generate list */
		temp_param->values = get_fixed_param_value(temp_param, fixed_params, fixed_values);
		if (!temp_param->values)
			temp_param->values = generate_variations(temp_param->param, variation_style);

/*  		BLTS_TRACE(":: values for %s : ",temp_param->param->key); */
/*  		dump_boxed_value_list(temp_param->values); */
/*  		BLTS_TRACE("\n"); */
		temp_param = temp_param->next;
	}

	switch (variation_style) {
	case CONFIG_VARIATION_COMBINATIONS:
	case CONFIG_VARIATION_FIRST:
	case CONFIG_VARIATION_LAST:
		param_arrs = test_param_list_flatten(params);
		if (!param_arrs) {
			BLTS_ERROR("BUG: Failed preparing permutations\n");
			goto done;
		}
		// debug_dump_param_arrs(param_arrs);
		perms = generate_permutation_table(param_arrs);
		if (!param_arrs) {
			BLTS_ERROR("BUG: Permutation generation failed\n");
			goto done;
		}
		variants = variant_list_reverse(generate_permutation_variants(perms, param_arrs));
		if (!variants) {
			BLTS_ERROR("BUG: Failed linking permuted tables to parameter values\n");
			goto done;
		}
		break;
	case CONFIG_VARIATION_ALLPAIRS:
		BLTS_ERROR("BUG: ALLPAIRS not implemented yet\n");
		goto done;
	default:
		BLTS_ERROR("BUG: Unknown variation style (%d)\n", variation_style);
		goto done;
	}

done:
	test_param_list_free(params);
	test_param_list_free(fixed_params);
	var_parameter_arrays_free(param_arrs);
	var_permute_table_free(perms);
	while ((fixed_values = blts_config_boxed_value_free(fixed_values)));
	return variants;
}

/* Return list of strings with names of parameters the given
 * variation-enabled test uses (NULL==error/none available) */
struct boxed_value *blts_config_get_test_param_names(char *variant_test_name)
{
	struct test_param_list *params = NULL, *fixed_params = NULL, *p;
	struct boxed_value *fixed_values = NULL, *ret = NULL, *val;
	int err;

	err = collect_test_params(variant_test_name, &params, &fixed_params,
				  &fixed_values);

	if(err)
		goto done;

	p = params;
	while(p) {
		val = blts_config_boxed_value_new_string(p->param->key);
		val->next = ret;
		ret = val;
		p = p->next;
	}

done:
	test_param_list_free(params);
	test_param_list_free(fixed_params);
	while (fixed_values)
		fixed_values = blts_config_boxed_value_free(fixed_values);
	return blts_config_boxed_value_list_reverse(ret);
}

/* Declare given test case able to use parameter variation according to
   config file loaded. After this, running the test case results in
   all configured variations to be executed in sequence. Parameters:

   name == test case name, must match test case definition and one
           of the tests in the configuration file

   arg_handler == user function called with parameters defined here,
           for each parameter change (between tests)

   Variable list, as triplets:

   CONFIG_PARAM_<type>, <param_name>, <value>
        Define parameter types and values in order.
	   <type> == one of INT,LONG,STRING,FLOAT,DOUBLE,BOOL
	   <param_name> == label for parameter (must match one defined in config)
	   <value> == value of given type (int/long/char*
	                   /float/double/int)

   CONFIG_PARAM_NONE ends the argument list (and must be present).

   Return 0 on success.
*/
int blts_config_declare_variable_test(char *name, arg_handler_fn arg_handler, ...)
{
	va_list ap;
	struct boxed_value *val;
	struct boxed_value *tag;
	struct symbol_table_entry *sym;
	int type;
	char *param_tag;
	char *str_val;
	int ret;
	unsigned use_defaults = 0;

	if (!name)
		return -EINVAL;

	if (!config_load_complete) {
		use_defaults = 1;
		setup_symbol_table_no_config(name);
	}

	sym = _blts_conf_symbol_table_lookup(name);
	if (sym && sym->type != SYM_TYPE_TEST) {
		BLTS_ERROR("Error: \"%s\" already defined, but not as test.\n", name);
		return -ENOENT;
	} else if (!sym) {
		BLTS_DEBUG("No test in configuration with name \"%s\". Using defaults.\n", name);
		use_defaults = 1;
		setup_symbol_table_no_config(name);
		sym = _blts_conf_symbol_table_lookup(name);
		if (!sym) {
			BLTS_ERROR("BUG: Can't setup defaults for test.\n", name);
			return -ENOENT;
		}
	}

	sym->test_definition->arg_handler = arg_handler;

	va_start(ap, arg_handler);
	ret = 0;
	while (1) {
		type = va_arg(ap, int);
		if (type == CONFIG_PARAM_NONE)
			break;

		param_tag = va_arg(ap, char *);
		if (config_load_complete) {
			if (!_blts_conf_symbol_table_lookup(param_tag)) {
				BLTS_ERROR("BUG: Undefined parameter \"%s\"\n", param_tag);
				ret = -EINVAL;
				goto done;
			}
		}
		tag = blts_config_boxed_value_new_string(param_tag ? param_tag : "");

		switch (type) {
		case CONFIG_PARAM_INT:
			val = blts_config_boxed_value_new_int(va_arg(ap, int));
			break;
		case CONFIG_PARAM_LONG:
			val = blts_config_boxed_value_new_long(va_arg(ap, long));
			break;
		case CONFIG_PARAM_BOOL:
			val = blts_config_boxed_value_new_bool(va_arg(ap, int));
			break;
		case CONFIG_PARAM_FLOAT:
			val = blts_config_boxed_value_new_float(va_arg(ap, double));
			break;
		case CONFIG_PARAM_DOUBLE:
			val = blts_config_boxed_value_new_double(va_arg(ap, double));
			break;
		case CONFIG_PARAM_STRING:
			str_val = va_arg(ap, char *);
			val = blts_config_boxed_value_new_string(str_val ? str_val : "");
			break;
		default:
			BLTS_ERROR("BUG: Invalid type in argument list\n");
			ret = -EINVAL;
			goto done;
		}
		/* Looks ok, so add this to param list. */
		tag->next = sym->test_definition->param_tags;
		sym->test_definition->param_tags = tag;
		val->next = sym->test_definition->param_defaults;
		sym->test_definition->param_defaults = val;

		if (use_defaults) {
			symbol_table_insert_param_with_default(param_tag, sym->key, val);
		}
	}
	sym->test_definition->param_tags =
		blts_config_boxed_value_list_reverse(sym->test_definition->param_tags);
	sym->test_definition->param_defaults =
		blts_config_boxed_value_list_reverse(sym->test_definition->param_defaults);

done:
	va_end(ap);

	//dump_symbol_table(); /* output symbols to trace if enabled */

	return ret;
}

static int process_pgroup_params(struct pgroupdef* pgroup, struct testdef* test_definition)
{
	struct boxed_value *val;
	struct boxed_value *tag;
	char *param_tag;

	for (unsigned int j = 0; j < pgroup->n_params; j++)
	{
		param_tag = pgroup->pgroup_params_syms[j]->key;
		//BLTS_DEBUG("Symbol type %d\n", param->param_pgroup->pgroup_params_syms[j]->type);
		int param_type = CONFIG_PARAM_NONE;
		struct symbol_table_entry* sym = pgroup->pgroup_params_syms[j];
		if (sym->type == SYM_TYPE_PARAM)
		{
			if (sym->param_definition->mode == PARAM_MODE_CONSTLIST)
				param_type = sym->param_definition->constlist_args->values[0].type;
			else if (sym->param_definition->mode == PARAM_MODE_RANGE)
				param_type = sym->param_definition->range_args->low_bound->type;
			else if (sym->param_definition->mode == PARAM_MODE_GENERATED)
				param_type = sym->param_definition->generated_args->generator_fn_params_syms[0]->value->type;
		}
		if (config_load_complete)
		{
			if (!_blts_conf_symbol_table_lookup(param_tag))
			{
				BLTS_ERROR("BUG: Undefined parameter \"%s\"\n", param_tag);
				return -EINVAL;
			}
		}
		tag = blts_config_boxed_value_new_string(param_tag ? param_tag
			: "");

		switch (param_type)
		{
		case CONFIG_PARAM_INT:
			val = blts_config_boxed_value_new_int(0);
			break;
		case CONFIG_PARAM_LONG:
			val = blts_config_boxed_value_new_long(0);
			break;
		case CONFIG_PARAM_BOOL:
			val = blts_config_boxed_value_new_bool(0);
			break;
		case CONFIG_PARAM_FLOAT:
			val = blts_config_boxed_value_new_float(0.0f);
			break;
		case CONFIG_PARAM_DOUBLE:
			val = blts_config_boxed_value_new_double(0.0);
			break;
		case CONFIG_PARAM_STRING:
			val = blts_config_boxed_value_new_string("");
			break;
		default:
			BLTS_ERROR("BUG: Invalid type in argument list\n")
			;
			return -EINVAL;
		}
		/* Looks ok, so add this to param list. */
		tag->next = test_definition->param_tags;
		test_definition->param_tags = tag;
		val->next = test_definition->param_defaults;
		test_definition->param_defaults = val;
	}

	return 0;
}

int blts_config_declare_variable_test_dynamic(char* name, tagged_arg_handler_fn arg_handler)
{
	struct symbol_table_entry *sym;
	int ret;
	unsigned use_defaults = 0;

	if (!name)
		return -EINVAL;

	if (!config_load_complete) {
		use_defaults = 1;
		setup_symbol_table_no_config(name);
	}

	sym = _blts_conf_symbol_table_lookup(name);
	if (sym && sym->type != SYM_TYPE_TEST) {
		BLTS_ERROR("Error: \"%s\" already defined, but not as test.\n", name);
		return -ENOENT;
	} else if (!sym) {
		BLTS_ERROR("No test in configuration with name \"%s\". Cannot obtain parameter data.\n", name);
		return -ENOENT;
	}

	sym->test_definition->arg_handler2 = arg_handler;

	ret = 0;
	for (unsigned int i = 0; i < sym->test_definition->n_settings; i++)
	{
		struct test_setting* param = sym->test_definition->settings[i];
		switch (param->setting_type)
		{
		case TEST_SETTING_PARAMLIST:
			ret = process_pgroup_params(param->param_pgroup, sym->test_definition);
			break;

		case TEST_SETTING_PGROUP:
			ret = process_pgroup_params(param->pgroup->pgroup_definition, sym->test_definition);
			break;

		case TEST_SETTING_FIX:
			// Skip fixed params as they are already in a param list
			break;

		default:
			BLTS_ERROR("Unhandled or invalid setting type %d\n", param->setting_type);
			ret = -EINVAL;
			break;
		}
	}

	if (ret == 0)
	{
		sym->test_definition->param_tags =
			blts_config_boxed_value_list_reverse(sym->test_definition->param_tags);
		sym->test_definition->param_defaults =
			blts_config_boxed_value_list_reverse(sym->test_definition->param_defaults);
	}

	return ret;
}

/* True if test exists and has variable parameters, false if not */
int blts_config_test_is_variable(char* name)
{
	struct symbol_table_entry *sym;
	struct testdef *t;
	if (!symbol_table)
		return 0;
	sym = _blts_conf_symbol_table_lookup(name);
	if (!sym)
		return 0;
	if (sym->type != SYM_TYPE_TEST)
		return 0;
	t = sym->test_definition;
	if (!t)
		return 0;

	if (!t->settings
		|| !t->settings[0]
		|| !t->param_defaults
		|| !t->param_tags
		|| (!t->arg_handler && !t->arg_handler2))
		return 0;
	return 1;
}

int blts_config_debug_dump_variants(char *test, int style)
{
	struct variant_list *variants, *temp;
	int i;

	variants = blts_config_generate_test_variations(test, style);

	if (!variants) {
		BLTS_TRACE("No variants");
		return -1;
	}
	BLTS_TRACE("------ Parameter variations for %s: \n", test);
	i = 0;
	while (variants) {
		i++;
		BLTS_TRACE("%.3d: ",i);
		dump_boxed_value_list(variants->values);
		BLTS_TRACE("\n");
		while ((variants->values = blts_config_boxed_value_free(variants->values)));
		temp = variants;
		variants = variants->next;
		free(temp);
	}
	BLTS_TRACE("------\n");

	return 0;
}

/* Pass parameter list to user-defined function for test. */
void* _blts_config_mutate_user_params(char *testname, struct boxed_value *values, void* user_ptr)
{
	struct symbol_table_entry *sym;
	void *res = NULL;

	sym = _blts_conf_symbol_table_lookup(testname);
	if (!sym) {
		BLTS_ERROR("Error: undefined symbol \"%s\"\n", testname);
		return NULL;
	}
	if (sym->type != SYM_TYPE_TEST) {
		BLTS_ERROR("Error: not a test \"%s\"\n", testname);
		return NULL;
	}
	if (! sym->test_definition->arg_handler && ! sym->test_definition->arg_handler2) {
		BLTS_ERROR("Error: no argument handler for test \"%s\"\n", testname);
		return NULL;
	}

	if (sym->test_definition->arg_handler)
	{
		res = sym->test_definition->arg_handler(values, user_ptr);
	}
	else if (sym->test_definition->arg_handler2)
	{
		res = sym->test_definition->arg_handler2(sym->test_definition->param_tags, values, user_ptr);
	}
	else
	{
		BLTS_ERROR("Error: no argument handler for test \"%s\"\n", testname);
	}

	return res;
}


int blts_config_register_generating_func(char *name, var_generator_func func)
{
	struct symbol_table_entry *sym;
	sym = _blts_conf_symbol_table_lookup(name);
	if (sym) {
		BLTS_ERROR("Error: symbol \"%s\" redefinition\n", name);
		return -EINVAL;
	}
	sym = _blts_conf_symbol_table_entry_new();
	sym->key = strdup(name);
	sym->type = SYM_TYPE_GENERATOR_FN;
	sym->gen_fn_definition = malloc(sizeof (struct generator_fn));
	sym->gen_fn_definition->gen_func = func;
	_blts_conf_symbol_table_insert(sym);
	dump_symbol_table();
	return 0;
}




/* Simple memory pool system for the parser so we don't leak all over the
   place. */

static void *mempool;
static unsigned pool_top;
static unsigned pool_size;

#define __ALIGN(x, a)		__ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))

static void *mempool_stretch(unsigned newsize)
{
	if (newsize < pool_size)
		return NULL;

	pool_size = __ALIGN(newsize, sysconf(_SC_PAGE_SIZE));

	mempool = realloc(mempool, pool_size);
	return mempool;
}

void *_blts_params_mempool_alloc(unsigned size)
{
	void *ret;

	if (size + pool_top > pool_size)
		if (!mempool_stretch(size + pool_top))
			return NULL;
	ret = mempool + pool_top;
	pool_top += size;
	return ret;
}

void _blts_params_mempool_release(void)
{
	if (mempool)
		free(mempool);
	mempool = NULL;
	pool_top = 0;
	pool_size = 0;
}
