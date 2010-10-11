/* interface.h --
 *
 * This file is part of MCTS 
 * 
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
 * 
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com; 
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * version 2.1 as published by the Free Software Foundation. 
 * 
 * This library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
 * 02110-1301 USA 
 * 
 */

#ifndef MIN_SCRIPTER_INTERFACE_H
#define MIN_SCRIPTER_INTERFACE_H


/* THIS IS DIRECTLY FROM MIN TEMPLATE. DONT'T CHANGE!*/

/* INCLUDES */
extern "C" {
#include <test_module_api.h>
#include <min_parser.h>
#include <min_logger.h>
#include <min_test_event_if.h>
#include <min_ipc_mechanism.h>
}

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

/** Usefull type definitions */
typedef struct _TestCaseInfoTC TestCaseInfoTC;
typedef struct _ScriptVariable ScriptVariable;

/** Pointer to the test case function. */
typedef int (*ptr2testtc)( MinItemParser * tcr );

/** Detailed info about a single Test Case. 
 *  Used only in Test Class!
 */
struct _TestCaseInfoTC
{
        /** Test Case name which must be unique in the scope of test module! */
        char name_[MaxTestCaseName];
        /** pointer to test function. */
        ptr2testtc test_;
        /** id of the test case */
        unsigned int id_;
};

/** Script variable. Received from scripter when test method is called.
 *  Used only in Test Class!
 */
struct _ScriptVariable
{
        /** Name of variable */
        char *var_name_;
        /** True if variable is initialized (has value). 
         *  this flag should never unset by test class */
        TSBool is_initialized_;
        /** Variable value stored as char buffer */
        char *var_value_;
};

extern "C" {
int ts_run_method( MinItemParser* mip );
int ts_get_test_cases( DLList** list );
int SetLocalValue (const char *varname, const char *varval);
int SetLocalValueInt (const char *varname, const long value);
int GetLocalValueInt (const char *varname, long *value);
int GetLocalValue (const char *varname, char **value);
}

#endif /* MIN_SCRIPTER_INTERFACE_H */


