%top {
/* param-scanner.l -- Flex scanner for generic test configurations

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
}
%option noyywrap nounput
/* %option outfile="param-scanner.c" header="param-scanner.h" */
%option bison-bridge bison-locations

%{
#include <stdlib.h>
#include <string.h>
#include "param-parser.h"
#include "blts_params_local.h"

extern void yyerror(char const *);
%}

/* float helpers (Yes, really. See the C99 standard...) */
dseq		([[:digit:]]+)
dseq_opt	([[:digit:]]*)
frac		(({dseq_opt}"."{dseq})|{dseq}".")
exp		([eE][+-]?{dseq})
exp_opt		({exp}?)
fsuff		[flFL]
fsuff_opt	({fsuff}?)
hpref		(0[xX])
hdseq		([[:xdigit:]]+)
hdseq_opt	([[:xdigit:]]*)
hfrac		(({hdseq_opt}"."{hdseq})|({hdseq}"."))
bexp		([pP][+-]?{dseq})
dfc		(({frac}{exp_opt}{fsuff_opt})|({dseq}{exp}{fsuff_opt}))
hfc		(({hpref}{hfrac}{bexp}{fsuff_opt})|({hpref}{hdseq}{bexp}{fsuff_opt}))

c99_floating_point_constant	({dfc}|{hfc})

/* integers */
decimal		([[:digit:]]{-}[0])[[:digit:]]*
hex		0[xX][[:xdigit:]]+
octal		0[0123456]*

/* C99 string literal */
litstring 	L?\"([^\"\\\n]|(\\['\"?\\abfnrtv])|(\\([0123456]{1,3}))|(\\x[[:xdigit:]]+)|(\\u([[:xdigit:]]{4}))|(\\U([[:xdigit:]]{8})))*\"

/* C99 identifiers */
ucn		((\\u([[:xdigit:]]{4}))|(\\U([[:xdigit:]]{8})))
nondigit	[_[:alpha:]]
c99_id		([_[:alpha:]]|{ucn})([_[:alnum:]]|{ucn})*

/* booleans */
bool		(true|false|TRUE|FALSE|on|ON|off|OFF)

/* type specifiers */
intspec		int
longspec	long
floatspec	float
doublespec	double
boolspec	boolean
stringspec	string

/* whitespace */
ws		[ \t]+
cr		[\n]+

/* section markers */
glob_start	\[globals\]
glob_end	\[end_globals\]
param_start	\[parameter\]
param_end	\[end_parameter\]
pgroup_start	\[pgroup\]
pgroup_end	\[end_pgroup\]
test_start	\[test\]
test_end	\[end_test\]
testset_start	\[testset\]
testset_end	\[end_testset\]

/* keywords */

kw_name		name
kw_const	const
kw_range	range
kw_generate	generate
kw_params	params
kw_pgroup	pgroup
kw_fix		fix
kw_serial	serial
kw_parallel	parallel
kw_threads	threads
kw_include	include

/* comment */
comment		#.*\n

/* this grabs the include file name */
%x incl
%%


{hex}     |
{octal}   |
{decimal} {
	yylval->int_val = strtol(yytext, 0, 0);
	return LONG;
}

{c99_floating_point_constant}	{
	yylval->float_val = strtod(yytext, 0);
	return DOUBLE;
}

{litstring} {
	/* Get rid of quote marks */
	if (strlen(yytext) < 2) {
		yyerror("Invalid quoted string");
                yyterminate();
        }
	yylval->str_val = _blts_conf_unquote_string_lit(yytext);
	if (!yylval->str_val) {
		yyerror("Error in unquote_string");
		yyterminate();
	}
	return STRING;
}

{glob_start}	{ return GLOB_START; }
{glob_end}	{ return GLOB_END; }
{param_start}	{ return PARAM_START; }
{param_end}	{ return PARAM_END; }
{pgroup_start}	{ return PGROUP_START; }
{pgroup_end}	{ return PGROUP_END; }
{test_start}	{ return TEST_START; }
{test_end}	{ return TEST_END; }
{testset_start}	{ return TESTSET_START; }
{testset_end}	{ return TESTSET_END; }

{kw_name}	{ return NAME; }
{kw_const}	{ return CONST; }
{kw_range}	{ return RANGE; }
{kw_generate}	{ return GENERATE; }
{kw_params}	{ return PARAMS; }
{kw_pgroup}	{ return PGROUP; }
{kw_fix}	{ return FIX; }
{kw_serial}	{ return SERIAL; }
{kw_parallel}	{ return PARALLEL; }
{kw_threads}	{ return THREADS; }

{kw_include}	BEGIN(incl);
<incl>{ws}
<incl>{litstring}\n {
	unsigned len = strlen(yytext);
	if (len < 2) {
		yyerror("Invalid quoted string");
		yyterminate();
        }
	/* strip whitespace + \n from end of match */
	char *filename = _blts_conf_unquote_string_lit(yytext);
	if (!filename) {
		yyerror("Error in unquote_string");
		yyterminate();
	}
	FILE *f = fopen(filename, "r");
	if (!f) {
		yyerror("Cannot open include file");
		yyterminate();
	}
	yypush_buffer_state(yy_create_buffer(f, YY_BUF_SIZE));
	BEGIN(INITIAL);
}
<<EOF>> {
	yypop_buffer_state();
	if (!YY_CURRENT_BUFFER) {
		yyterminate();
	}
	return CR;
}

{intspec}	{ return INTSPEC; }
{longspec}	{ return LONGSPEC; }
{floatspec}	{ return FLOATSPEC; }
{doublespec}	{ return DOUBLESPEC; }
{boolspec}	{ return BOOLSPEC; }
{stringspec}	{ return STRINGSPEC; }

{bool} {
	if(        !strncmp(yytext, "true", 4)
		|| !strncmp(yytext, "TRUE", 4)
		|| !strncmp(yytext, "on", 2)
		|| !strncmp(yytext, "ON", 2)) {
		yylval->int_val = 1;
	} else {
		yylval->int_val = 0;
	}
	return BOOL;
}

{c99_id} {
	unsigned len = strlen(yytext) + 1;
	yylval->str_val = _blts_params_mempool_alloc(len);
	strcpy(yylval->str_val, yytext);
	return IDENT;
}

{ws}

{comment} {
	return CR;
}

{cr} {
	return CR;
}

. {return 0;}

%%
