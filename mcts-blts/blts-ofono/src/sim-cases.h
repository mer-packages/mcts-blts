/* sim-cases.h -- SIM cases for blts-ofono

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

#ifndef SIM_CASES_H
#define SIM_CASES_H

#include <blts_params.h>

// functions
int ofono_change_pin(void* user_ptr, int test_num);
int ofono_enter_pin(void* user_ptr, int test_num);
int ofono_reset_pin(void* user_ptr, int test_num);
int ofono_lock_pin(void* user_ptr, int test_num);
int ofono_unlock_pin(void* user_ptr, int test_num);
// debug
int ofono_sim_properties(void* user_ptr, int test_num);

//variable parameters
void *sim_variant_set_arg_processor(struct boxed_value *, void *);


#endif /* SIM_CASES_H */
