/* interface.cpp -- Source code file for MIN's interface

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

#include "interface.h"


extern char *module_date;
extern char *module_time;

TTestModuleType module_type     = ETestClass;
unsigned int    module_version  = 200924;

DLList *variables;
int             _look4callname (const void *a, const void *b);
int             _init_vars ();
int             _send_vars ();

ScriptVariable *_find_var (const char *varname);


/** Compare function for callname
 */
int _look4callname (const void *a, const void *b)
{
        TestCaseInfoTC *tci = (TestCaseInfoTC *) a;
        return strcmp (tci->name_, (char *)b);
}

/** Initialize scripter local variables  
 *  @return 0 on success, -1 on error
 */
int _init_vars ()
{
        void *shmaddr, *tmp;
        unsigned int size;
        char *var_buff, *variable_str = (char *)INITPTR, *p;
        int retval = ENOERR;
        int shmid;        
        MinItemParser *mip;
        ScriptVariable *variable;

        variables = dl_list_create();
        shmid = shmget (getppid(), sizeof (unsigned int),  0660);
        if (shmid == -1) {
                MIN_WARN ("Could not create shared memory segment");
                return -1;
        }
        
        shmaddr = sm_attach (shmid);
        if (shmaddr == INITPTR) {
                MIN_WARN ("Could not attach to shared memory segment");
                return -1;
        }

        retval = sm_read (shmaddr, &size, sizeof (unsigned int));
        if (retval != ENOERR) {
                MIN_WARN ("Read from shared memory segment failed");
                retval = -1;
                goto exit;
        } else if (size == 0) {
                MIN_WARN ("No variables in shared memory segment");
                goto exit;
        }
        /*
        ** Read the variable buffer from shared memory
        */
        var_buff = NEW2 (char, size + 1 );
        sm_read ((int *)shmaddr + 1, var_buff, size); 
        var_buff [size] = '\0';
        
        /*
        ** Create Item Parser for variable parsing
        */
        mip = mip_create (var_buff, 0, size);
        if (mip == INITPTR) {
                MIN_WARN ("Failed to create item parser "
                             "from variables");
                
                retval =  -1;
                goto exit;
        }
        /*
        ** Parse the variables and save them to variable list
        */
        mip_get_string (mip, (char *)INITPTR, &variable_str);
        while (variable_str != INITPTR) {
                if ((p = strrchr (variable_str, '='))) {
                        /* 
                        ** initialized variable 
                        */
                        *p = '\0';
                        p++;
                        if (_find_var (variable_str) != INITPTR) {
                                MIN_WARN ("variable with name %s "
                                           "already exists",  variable_str);
                                goto loopend;
                        }
                        variable = NEW (ScriptVariable);
                        variable->var_name_ = NEW2 (char, 
                                                    strlen (variable_str) +1);
                        strcpy (variable->var_name_, variable_str);
                        if (strlen (p) > 0) {
                                variable->is_initialized_ = ESTrue;
                                variable->var_value_ = NEW2 (char, strlen(p)
                                                             + 1);
                                strcpy (variable->var_value_, p);
                        } else {
                                variable->is_initialized_ = ESFalse;

                                MIN_WARN ("missing variable value (%s)", 
                                           variable_str);
                        }
                } else {
                        /*
                        ** unitialized variable
                        */
                        if (_find_var (variable_str) != INITPTR) {
                                MIN_WARN ("variable with name %s already "
                                           "exists", variable_str);
                                goto loopend;
                        }
                        variable = NEW (ScriptVariable);
                        variable->is_initialized_ = ESFalse;
                        variable->var_name_ = NEW2 (char, 
                                                    strlen (variable_str) +1);
                        strcpy (variable->var_name_, variable_str);

                }
                dl_list_add (variables, variable);
        loopend:
                if (mip_get_next_string (mip, &variable_str) != ENOERR)
                        break;
        }

        free (var_buff);
        free (variable_str);
        mip_destroy (&mip);
exit:
        
        sm_detach (shmaddr);

        return retval;
}



/** Send the variables back to scripter.
 *  @return 0 on success, 1 on error
 */
int _send_vars ()
{
        DLListIterator   it;
        ScriptVariable *var;
        Text           *vars;
        int             shmid;
        void           *sh_mem_handle = NULL, *tmp;


        shmid = shmget (getppid(), sizeof (unsigned int),  0660);
        if (shmid == -1) {
                MIN_WARN ("Could not create shared memory segment");
                goto exit;
        }

        vars = tx_create ((char *)INITPTR);

        for (it = dl_list_head(variables); it != DLListNULLIterator; 
             it = dl_list_next(it)) {
		var = (ScriptVariable *)dl_list_data (it);
                /*
                ** We only send back variables with a value
                */
                if (var->is_initialized_ == ESFalse) 
                        continue;
                tx_c_append (vars, var->var_name_);
                tx_c_append (vars, "=");
                tx_c_append (vars, var->var_value_);
                tx_c_append (vars, " ");
        }

        sh_mem_handle = sm_attach (shmid);
        if ((vars->size_ + sizeof (unsigned int)) >  sm_get_segsz (shmid)) {
                tx_destroy (&vars);
                return 1;
        }

        
        sm_write (sh_mem_handle,
                  (void *)&vars->size_,
                  sizeof (unsigned int));
        tmp = (int *)sh_mem_handle + 1;
        sm_write (tmp,
                  (void *)(tx_share_buf (vars)),
                  vars->size_);

        MIN_DEBUG ("SENT VARIABLES: %s", tx_share_buf(vars));
        sm_detach (sh_mem_handle);
        tx_destroy (&vars);
exit:
        for (it = dl_list_head(variables); it != DLListNULLIterator;
             it = dl_list_next(it)) {
	        var = (ScriptVariable *)dl_list_data (it);
                free (var->var_name_);
                if (var->is_initialized_)
                        free (var->var_value_);
                free (var);
        }
        dl_list_free (&variables);
        return 0;
}



/** Find the variable from list 
 *  @param varname variable name
 *  @return pointer to scripter variable or INITPTR if no variable matching 
 *          argument is found.
 */
ScriptVariable *_find_var (const char *varname)
{
        DLListIterator  it;
        ScriptVariable *var;

        it = dl_list_head (variables);
        while (it != INITPTR) {
	        var = (ScriptVariable *)dl_list_data (it);
                if (!strcmp (var->var_name_, varname)) {
                        return var;
                }
                it = dl_list_next (it);
        }
        return (ScriptVariable *)INITPTR;
}



/** Run method from test this class.
 *  @param item MinItemparser containing name of test class function and 
 *         parameters
 *  @return the return value of test class function
 */
int ts_run_method (MinItemParser * item)
{
        DLList         *l;
        DLListIterator  it;
        int             retval;
        char           *callname;

        l = dl_list_create ();
        callname = (char *)INITPTR;

        /*
         * Take callname  
         */
        retval = mip_get_next_string (item, &callname);
        if (retval != ENOERR) {
                retval = -1;
                goto EXIT;
        }

        /*
         * Obtain list of test cases 
         */
        ts_get_test_cases (&l);

        /*
         * Look for test with name set to callname 
         */
        it = dl_list_find (dl_list_head (l)
                           , dl_list_tail (l)
                           , _look4callname, callname);

        if (it == DLListNULLIterator) {
                retval = -1;
                free (callname);
                it = dl_list_head (l);
                while (it != DLListNULLIterator) {
                        free (dl_list_data (it));
                        dl_list_remove_it (it);
                        it = dl_list_head (l);
                }
                dl_list_free (&l);
                goto EXIT;
        }

        /*
         * Intialize variables
         */
        _init_vars ();
        /*
         * Call function 
         */
        retval = ((TestCaseInfoTC *) dl_list_data (it))->test_ (item);

        /*
         * Send variables back to scripter
         */
        if (_send_vars ()) {
                MIN_WARN ("Variable sending failed");
                retval = -1;
        }

        /*
         * Cleanup 
         */
        free (callname);
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

extern "C" {
/** return  test module type */
unsigned int get_module_type()
{ return module_type; }

/** return test module pim version */
unsigned int get_module_version()
{ return module_version; }

/** return build date */
char* get_module_date()
{ return module_date; }

/** return build time */
char* get_module_time()
{ return module_time; }

}


/** Assign string value to script variable
 * @param varname name of variable
 * @param varval value to assign
 * @return 0 on success, -1 on error (e.g. variable not declared)
 */
int SetLocalValue (const char *varname, const char *varval)
{
        ScriptVariable *var;

        if ((var = _find_var (varname)) == INITPTR) {
                MIN_WARN ("no variable by the name %s found", varname);
                return -1;
        }

        if (var->is_initialized_ && var->var_value_ != INITPTR &&
            var->var_value_ != NULL)
                free (var->var_value_);
        var->is_initialized_ = ESTrue;
        
        if (varval != INITPTR && varval != NULL) {
                var->var_value_ = NEW2 (char, strlen (varval) + 1);
                strcpy (var->var_value_, varval);
        } else {
                var->var_value_ = NEW2 (char, strlen ("NULL") + 1);
                strcpy (var->var_value_, "NULL");
        }

        return 0;
}

/** Assign integer value to script variable
 * @param varname name of variable
 * @param varval value to assign
 * @return 0 on success, -1 on error
 */
int SetLocalValueInt (const char *varname, const long value)
{
        ScriptVariable *var;
        
        if ((var = _find_var (varname)) == INITPTR) {
                MIN_WARN ("no variable by the name %s found", varname);
                return -1;
        }
        if (var->is_initialized_ && var->var_value_ != INITPTR &&
            var->var_value_ != NULL)
                free (var->var_value_);
        var->is_initialized_ = ESTrue;
        
        var->var_value_ = NEW2 (char, 32);
        sprintf (var->var_value_, "%ld", value);

        return 0;
}

/** Get value of script variable as an integer
 * @param varname name of variable
 * @param value [out] variable value
 * @return 0 on success, -1 on error
 */
int GetLocalValueInt (const char *varname, long *value)
{
        ScriptVariable *var;

        if ((var = _find_var (varname)) == INITPTR) {
                MIN_WARN ("no variable by the name %s found", varname);
                return -1;
        }
        
        if (var->is_initialized_ == ESFalse) {
                MIN_WARN ("variable %s is not initialized", varname);
                return -1;
        }       

        *value = strtol (var->var_value_, NULL, 10);
        
        if (*value == 0 && errno) { /* The value is 0, or there was error */
                MIN_WARN ("conversion from string to integer failed %s",
                           strerror (errno));
                return -1;
        }
        return 0;
}

/** Get value of script variable as a string
 * @param varname name of variable
 * @param value [out] variable value
 * @return 0 on success, -1 on error
 */
int GetLocalValue (const char *varname, char **value)
{
        ScriptVariable *var;

        if ((var = _find_var (varname)) == INITPTR) {
                MIN_WARN ("no variable by the name %s found", varname);
                return -1;
        }
        
        if (var->is_initialized_ == ESFalse) {
                MIN_WARN ("variable %s is not initialized", varname);
                return -1;
        }       
        
        *value = var->var_value_;

        return 0;
}

