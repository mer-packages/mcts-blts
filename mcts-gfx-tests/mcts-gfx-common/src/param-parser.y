/* param-parser.y -- Yacc/Bison parser for generic test configurations

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

%error-verbose
%locations
%pure-parser
 // %define api.pure
%{
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
//#define YYPARSE_PARAM scanner
//#define YYLEX_PARAM scanner

#include "blts_log.h"
#include "blts_params_local.h"
#include "param-parser.h"

int yylex(YYSTYPE *, YYLTYPE *);
void yyerror(char const *);
extern FILE *yyin;

struct idlist {
	struct idlist *next;
	struct symbol_table_entry *sym;
};
struct settinglist {
	struct settinglist *next;
	struct test_setting *setting;
};

static int commit_current_symbol();
static void value_list_prepend_boxed(struct boxed_value *val);
static void ident_list_create_symbol_array(struct symbol_table_entry ***new_array, unsigned *array_size);
static void settinglist_create_setting_array(struct test_setting ***new_array, unsigned *array_size);
static struct idlist *identlist_lookup_from_boxed_keys(struct boxed_value *list);
static void identlist_free(struct idlist *list);
static void settinglist_free(struct settinglist *list);

static struct symbol_table_entry *current_sym; /* Context: candidate for the current symbol */

static struct boxed_value *current_value_list_head;  /* Context: head of argument list being processed */
static struct idlist *current_ident_list_head; /* Context: head of identifier list being processed */
static struct settinglist *current_setting_list_head; /* Context: head of test settings list being processed */

static int parse_errors;

%}

/* data types */
%union {
	long int_val;
	char *str_val;
	double float_val;
}
%{
%}
%token <int_val> INT
%token INTSPEC
%token <int_val> LONG
%token LONGSPEC
%token <int_val> BOOL
%token BOOLSPEC
%token <float_val> FLOAT
%token FLOATSPEC
%token <float_val> DOUBLE
%token DOUBLESPEC
%token <str_val> STRING
%token STRINGSPEC


/* sections */
%token GLOB_START
%token GLOB_END
%token PARAM_START
%token PARAM_END
%token PGROUP_START
%token PGROUP_END
%token TEST_START
%token TEST_END
%token TESTSET_START
%token TESTSET_END

/* keywords */
%token NAME
%token CONST
%token RANGE
%token GENERATE
%token PARAMS
%token PGROUP
%token FIX
%token SERIAL
%token PARALLEL
%token THREADS


/* misc */
%token <str_val> IDENT
%token CR



%%
input:		/* empty */
	|	input CR
	|	input globals
	|	input parameter
	|	input pgroup
	|	input test
	|	input testset
	|	input error CR
	;

/* ====== Global defines */
/* Includes get processed in scanner */
globals:	GLOB_START CR globaldefines GLOB_END;

globaldefines:	/* empty */
	|	globaldefines valuedef
	|	globaldefines CR           /* <-- TODO: handle end-of-include-file better than this */
	;

valuedef:	IDENT		{
					current_sym = _blts_conf_symbol_table_entry_new();
					current_sym->key = strdup($1);
					current_sym->type = SYM_TYPE_BOXED_VALUE;
				}
		value CR	{
					int ret;
					ret = commit_current_symbol();
					if (ret)
						YYERROR;
				}
	;

/* ====== Parameter definition */
parameter:	PARAM_START CR paramdefinition PARAM_END;

paramdefinition: NAME IDENT CR	{
					current_sym = _blts_conf_symbol_table_entry_new();
					current_sym->key = strdup($2);
					current_sym->type = SYM_TYPE_PARAM;
					current_sym->param_definition = malloc(sizeof (struct paramdef));
					memset(current_sym->param_definition, 0, sizeof (struct paramdef));
				}
		paramvalues CR	{
					int ret;
					ret = commit_current_symbol();
					if (ret)
						YYERROR;
				}
	;

paramvalues:	paramconstlist
	|	paramrange
	|	paramgenerate
	;

paramconstlist:	CONST		{
					current_sym->param_definition->mode = PARAM_MODE_CONSTLIST;
					current_value_list_head = 0;
					current_sym->param_definition->constlist_args = malloc(sizeof (struct param_constlist_args));
					memset(current_sym->param_definition->constlist_args, 0, sizeof (struct param_constlist_args));
				}
		value values	{
					current_value_list_head = blts_config_boxed_value_list_reverse(current_value_list_head);
					current_sym->param_definition->constlist_args->values = current_value_list_head;
					current_value_list_head = 0;
				}
	;

paramrange:	RANGE		{
					current_sym->param_definition->mode = PARAM_MODE_RANGE;
					current_value_list_head = 0;
					current_sym->param_definition->range_args = malloc(sizeof (struct param_range_args));
					memset(current_sym->param_definition->range_args, 0, sizeof (struct param_range_args));
				}
		numerictypespec numericvalue numericvalue {
					if (current_value_list_head) {
						current_sym->param_definition->range_args->low_bound = current_value_list_head->next;
						current_sym->param_definition->range_args->high_bound = current_value_list_head;
					}
					current_value_list_head = 0;
				}
	;

paramgenerate:	GENERATE IDENT	{
					current_sym->param_definition->mode = PARAM_MODE_GENERATED;
					current_sym->param_definition->generated_args = malloc(sizeof (struct param_generated_args));
					memset(current_sym->param_definition->generated_args, 0, sizeof (struct param_generated_args));
					current_sym->param_definition->generated_args->generator_fn_name = strdup($2);
					current_ident_list_head = 0;
				}
		idlist		{
					ident_list_create_symbol_array(
						&(current_sym->param_definition->generated_args->generator_fn_params_syms),
						&(current_sym->param_definition->generated_args->n_params));
					identlist_free(current_ident_list_head);
					current_ident_list_head = 0;
				}
	;

/* ====== Parameter group */
pgroup:		PGROUP_START CR pgroupdefinition PGROUP_END;

pgroupdefinition: NAME IDENT CR	{
					current_sym = _blts_conf_symbol_table_entry_new();
					current_sym->key = strdup($2);
					current_sym->type = SYM_TYPE_PGROUP;
					current_sym->pgroup_definition = malloc(sizeof (struct pgroupdef));
					memset(current_sym->pgroup_definition, 0, sizeof (struct pgroupdef));
					current_ident_list_head = 0;
				}
		pgroupcontents	{
					ident_list_create_symbol_array(
						&(current_sym->pgroup_definition->pgroup_params_syms),
						&(current_sym->pgroup_definition->n_params));
					identlist_free(current_ident_list_head);
					current_ident_list_head = 0;
					int ret;
					ret = commit_current_symbol();
					if (ret)
						YYERROR;
		}
	;

pgroupcontents:	/* empty */
	|	pgroupcontents idlist CR
	|	error CR
	;

/* ====== Test definition */
test:		TEST_START CR testdefinition TEST_END;

testdefinition:	NAME STRING CR	{
					current_sym = _blts_conf_symbol_table_entry_new();
					current_sym->key = strdup($2);

					current_sym->type = SYM_TYPE_TEST;
					current_sym->test_definition = malloc(sizeof (struct testdef));
					memset(current_sym->test_definition, 0, sizeof (struct testdef));
					current_setting_list_head = 0;
				}
		testsettings	{
			if (current_setting_list_head) {
					settinglist_create_setting_array(
						&(current_sym->test_definition->settings),
						&(current_sym->test_definition->n_settings));
					settinglist_free(current_setting_list_head);
					current_setting_list_head = 0;
			}
					int ret;
					ret = commit_current_symbol();
					if (ret)
						YYERROR;
				}
	;

testsettings:	/* empty */
	|	testsettings PARAMS {   /* Prepend new entry in settingslist for this test; prepare idlist for parsing */
					struct settinglist *li;
					struct test_setting *setting;
					li = malloc(sizeof *li);
					setting = malloc(sizeof *setting);
					li->setting = setting;
					li->next = current_setting_list_head;
					setting->setting_type = TEST_SETTING_PARAMLIST;
					setting->param_pgroup = malloc(sizeof (struct pgroupdef));
					memset(setting->param_pgroup, 0, sizeof (struct pgroupdef));
					current_setting_list_head = li;
					current_ident_list_head = 0;
				}
		idlist CR	{
					ident_list_create_symbol_array(
						&(current_setting_list_head->setting->param_pgroup->pgroup_params_syms),
						&(current_setting_list_head->setting->param_pgroup->n_params));
					identlist_free(current_ident_list_head);
				}

	|	testsettings PGROUP IDENT CR {
					struct settinglist *li;
					struct test_setting *setting;
					struct symbol_table_entry *sym;
					sym = _blts_conf_symbol_table_lookup($3);
					if (!sym) {
						BLTS_ERROR("PGroup %s not defined\n", $3);
						yyerror("unknown pgroup");
						YYERROR;
					}
					li = malloc(sizeof *li);
					setting = malloc(sizeof *setting);
					li->setting = setting;
					setting->setting_type = TEST_SETTING_PGROUP;
					setting->pgroup = sym;
					li->next = current_setting_list_head;
					current_setting_list_head = li;
				}

	|	testsettings FIX IDENT {
					struct settinglist *li;
					struct test_setting *setting;
					struct symbol_table_entry *sym;
					sym = _blts_conf_symbol_table_lookup($3);
					if (!sym) {
						BLTS_ERROR("Parameter %s not defined\n", $3);
						yyerror("unknown parameter");
						YYERROR;
					}
					li = malloc(sizeof *li);
					setting = malloc(sizeof *setting);
					li->setting = setting;
					setting->setting_type = TEST_SETTING_FIX;
					setting->fix_value = malloc(sizeof (struct fixed_value));
					memset(setting->fix_value, 0, sizeof (struct fixed_value));
					setting->fix_value->param = sym;
					current_value_list_head = 0;
					li->next = current_setting_list_head;
					current_setting_list_head = li;
				}
		value CR	{
					current_setting_list_head->setting->fix_value->value = current_value_list_head;
					current_value_list_head = 0;
				}
        |       CR
	|	error CR
	;


/* ====== Testset definition */
testset:	TESTSET_START CR testsetdefinition TESTSET_END;

testsetdefinition: NAME STRING CR {
					current_sym = _blts_conf_symbol_table_entry_new();
					current_sym->key = strdup($2);
					current_sym->type = SYM_TYPE_TESTSET;
					current_sym->testset_definition = malloc(sizeof (struct testsetdef));
					memset(current_sym->testset_definition, 0, sizeof (struct testsetdef));
					current_setting_list_head = 0;
				}
		testsetsettings	{
					settinglist_create_setting_array(
						&(current_sym->testset_definition->settings),
						&(current_sym->testset_definition->n_settings));
					settinglist_free(current_setting_list_head);
					current_setting_list_head = 0;
					current_ident_list_head = 0;
				}
		testsetcontents {
					if (!current_ident_list_head) {
						yyerror("failed to parse test list");
						YYERROR;
					}
					ident_list_create_symbol_array(
						&(current_sym->testset_definition->tests),
						&(current_sym->testset_definition->n_tests));
					identlist_free(current_ident_list_head);
					current_ident_list_head = 0;
					int ret;
					ret = commit_current_symbol();
					if (ret)
						YYERROR;
				}
	;

testsetsettings: /* empty */
	| testsetsettings FIX IDENT {   /* Maybe allow full test settings here? */
					struct settinglist *li;
					struct test_setting *setting;
					struct symbol_table_entry *sym;
					sym = _blts_conf_symbol_table_lookup($3);
					if (!sym) {
						BLTS_ERROR("Parameter %s not defined\n", $3);
						yyerror("unknown parameter");
						YYERROR;
					}
					li = malloc(sizeof *li);
					setting = malloc(sizeof *setting);
					li->setting = setting;
					setting->setting_type = TEST_SETTING_FIX;
					setting->fix_value = malloc(sizeof (struct fixed_value));
					memset(setting->fix_value, 0, sizeof (struct fixed_value));
					setting->fix_value->param = sym;
					current_value_list_head = 0;
					li->next = current_setting_list_head;
					current_setting_list_head = li;
				}
	value CR		{
					current_setting_list_head->setting->fix_value->value = current_value_list_head;
					current_value_list_head = 0;
				}
	;

testsetcontents: SERIAL		{
					current_sym->testset_definition->exec_type = TESTSET_EXEC_SERIAL;
					current_value_list_head = 0;
				}
		testlist CR	{
					current_ident_list_head = identlist_lookup_from_boxed_keys(current_value_list_head);
					current_value_list_head = 0;
				}

	|	PARALLEL	{
					current_sym->testset_definition->exec_type = TESTSET_EXEC_PARALLEL;
					current_ident_list_head = 0;
				}
		testlist CR	{
					current_ident_list_head = identlist_lookup_from_boxed_keys(current_value_list_head);
					current_value_list_head = 0;
				}

	|	THREADS		{
					current_sym->testset_definition->exec_type = TESTSET_EXEC_THREADS;
					current_ident_list_head = 0;
				}
		testlist CR	{
					current_ident_list_head = identlist_lookup_from_boxed_keys(current_value_list_head);
					current_value_list_head = 0;
				}

	|	error CR
	;


/* ====== terminal-ish things */

values: /* empty */
	|	values value
	;

value: intvalue | boolvalue | floatvalue | longvalue | doublevalue | stringvalue | globalvalue;
numericvalue: intvalue | longvalue | floatvalue | doublevalue | globalnumericvalue;


globalnumericvalue: globalvalue 	{
						int type;
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							type = current_sym->value->type;
						default:
							type = current_value_list_head->type;
						}
						if (type != CONFIG_PARAM_DOUBLE && type != CONFIG_PARAM_LONG) {
							yyerror("global variable not a number");
							YYERROR;
						}
					}
	;
globalvalue:	IDENT			{
						struct symbol_table_entry *sym;
						sym = _blts_conf_symbol_table_lookup($1);
						if (!sym || sym->type != SYM_TYPE_BOXED_VALUE) {
							BLTS_ERROR("Global variable %s not defined\n", $1);
							yyerror("unknown variable");
							YYERROR;
						}
						struct boxed_value *val = blts_config_boxed_value_dup(sym->value);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

intvalue:	INT			{
						struct boxed_value *val = blts_config_boxed_value_new_int($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

longvalue:	LONG			{
						struct boxed_value *val = blts_config_boxed_value_new_long($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

boolvalue:	BOOL			{
						struct boxed_value *val = blts_config_boxed_value_new_bool($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

floatvalue:	FLOAT			{
						struct boxed_value *val = blts_config_boxed_value_new_float($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

doublevalue:	DOUBLE			{
						struct boxed_value *val = blts_config_boxed_value_new_double($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;

stringvalue:	STRING			{
						struct boxed_value *val = blts_config_boxed_value_new_string($1);
						switch (current_sym->type) {
						case SYM_TYPE_BOXED_VALUE:
							current_sym->value = val;
						default:
							value_list_prepend_boxed(val);
						}
					}
	;


numerictypespec: INTSPEC		{
						if (current_sym->type == SYM_TYPE_PARAM)
							current_sym->param_definition->type = CONFIG_PARAM_LONG;
					}
	|	FLOATSPEC		{
						if (current_sym->type == SYM_TYPE_PARAM)
							current_sym->param_definition->type = CONFIG_PARAM_DOUBLE;
					}
	|       LONGSPEC		{
						if (current_sym->type == SYM_TYPE_PARAM)
							current_sym->param_definition->type = CONFIG_PARAM_LONG;
					}
	|	DOUBLESPEC		{
						if (current_sym->type == SYM_TYPE_PARAM)
							current_sym->param_definition->type = CONFIG_PARAM_DOUBLE;
					}
	;

idlist:		/* empty */
	|	idlist IDENT 		{
						struct symbol_table_entry *sym;
						struct idlist *li = 0;
						sym = _blts_conf_symbol_table_lookup($2);
						if (!sym) {
							BLTS_ERROR("\"%s\" not defined\n", $2);
							yyerror("unknown identifier");
							YYERROR;
						}
						li = malloc(sizeof *li);
						li->sym = sym;
						li->next = current_ident_list_head;
						current_ident_list_head = li;
					}
	;

testlist:	/* empty */
	|	testlist stringvalue
	;

%%

void yyerror(char const *e)
{
	/* TODO : Location info */
	BLTS_ERROR("Parse error: %s\n",e);
	parse_errors++;
	/* Need to discard current context here. We'll try to
	   recover at the next CR. */

	/* FIXME: Freeing things unconditionally here will segfault on some
	   syntax errors. */

	/* if (current_sym) */
/* 		_blts_conf_symbol_table_entry_free(current_sym); */

/* 	while((current_value_list_head = */
/* 		blts_config_boxed_value_free(current_value_list_head))); */

/*         identlist_free(current_ident_list_head); */
/*         settinglist_free(current_setting_list_head); */

	current_value_list_head = 0;
	current_ident_list_head = 0;
	current_setting_list_head = 0;
	current_sym = 0;
}

static int commit_current_symbol()
{
	int ret;
	if (!current_sym) {
		BLTS_TRACE("Refusing to insert null symbol\n");
		return -EINVAL;
	}
	ret = _blts_conf_symbol_table_insert(current_sym);
	if (ret)
		yyerror("cannot insert symbol");
	return ret;
}

static void value_list_prepend_boxed(struct boxed_value *val)
{
	val->next = current_value_list_head;
	current_value_list_head = val;
}

static void ident_list_create_symbol_array(struct symbol_table_entry ***new_array, unsigned *array_size)
{
	struct symbol_table_entry **arr;
	struct idlist *head, *prev, *next;
	unsigned size = 0, i = 0;
	if (!new_array || !array_size || !current_ident_list_head) {
		return;
	}
	/* Due to parsing order, the list is backwards; reverse it. Also count size. */
	prev = 0;
	head = current_ident_list_head;
	while (head) {
		size++;
		next = head->next;
		head->next = prev;
		prev = head;
		head = next;
	}
	current_ident_list_head = prev;
	head = current_ident_list_head;
	arr = malloc(size * sizeof *arr);
	memset(arr, 0, size * sizeof *arr);
	while (head) {
		arr[i++] = head->sym;
		head = head->next;
	}
	*new_array = arr;
	*array_size = size;
}

static void settinglist_create_setting_array(struct test_setting ***new_array, unsigned *array_size)
{
	struct test_setting **arr;
	struct settinglist *head, *prev, *next;
	unsigned size = 0, i = 0;
	if (!new_array || !array_size || !current_setting_list_head) {
		return;
	}
	/* Due to parsing order, the list is backwards; reverse it. Also count size. */
	prev = 0;
	head = current_setting_list_head;
	while (head) {
		size++;
		next = head->next;
		head->next = prev;
		prev = head;
		head = next;
	}
	current_setting_list_head = prev;
	head = current_setting_list_head;
	arr = malloc(size * sizeof *arr);
	memset(arr, 0, size * sizeof *arr);
	while (head) {
		arr[i++] = head->setting;
		head = head->next;
	}

	*new_array = arr;
	*array_size = size;
}

/* Do symbol table lookups for list of boxed string-valued keys. */
static struct idlist *identlist_lookup_from_boxed_keys(struct boxed_value *list)
{
	struct idlist *ids = 0, *ids_head = 0, *prev = 0;
	struct boxed_value *values;
	values = list;
	while (values) {
		ids = malloc(sizeof *ids);
		memset(ids, 0, sizeof *ids);
		if (!ids_head)
			ids_head = ids;
		if (prev)
			prev->next = ids;
		if (values->type != CONFIG_PARAM_STRING)
			/* Type mismatch */
			goto error;
		ids->sym = _blts_conf_symbol_table_lookup(values->str_val);
		if (!ids->sym)
			goto error;
		prev = ids;
		values = values->next;
	}
	return ids_head;

error:
	while((ids = ids_head)) {
		ids_head = ids->next;
		free(ids);
	}
	return 0;
}

/* Frees the given idlist. Doesn't touch content. */
static void identlist_free(struct idlist *list)
{
	struct idlist *head, *next=0;
	head = list;
	while(head) {
		next = head->next;
		free(head);
		head = next;
	}
}

/* Frees the given settinglist. Doesn't touch content. */
static void settinglist_free(struct settinglist *list)
{
	struct settinglist *head, *next=0;
	head = list;
	while(head) {
		next = head->next;
		free(head);
		head = next;
	}
}

int _blts_conf_parser_start(FILE *f)
{
	int ret;
	yyin = f;
	current_sym = 0;
	current_value_list_head = 0;
	current_ident_list_head = 0;
	current_setting_list_head = 0;
	parse_errors = 0;
	ret = yyparse();
	_blts_params_mempool_release();

	if (ret)
		return ret;
	return parse_errors;
}
