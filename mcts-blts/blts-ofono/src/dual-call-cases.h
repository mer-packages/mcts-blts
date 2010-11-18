/* dual-call-cases.h -- Dual call cases for blts-ofono

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

#ifndef DUAL_CALL_CASES_H
#define DUAL_CALL_CASES_H

#include <blts_params.h>

int blts_ofono_case_dual_call_transfer(void* user_ptr, int test_num);
int blts_ofono_case_dual_call_swap(void* user_ptr, int test_num);
int blts_ofono_case_dual_call_release_and_answer(void* user_ptr, int test_num);
int blts_ofono_case_dual_call_hold_and_answer(void* user_ptr, int test_num);

//variable parameters processing
void *
dual_call_case_variant_set_arg_processor(struct boxed_value *, void *);

#endif /* DUAL_CALL_CASES_H */
