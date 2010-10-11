/* call-cases.h -- Call cases for blts-ofono

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

#ifndef CALL_CASES_H
#define CALL_CASES_H

#include <blts_params.h>

int my_ofono_case_voicecall_to(void* user_ptr, int test_num);
int my_ofono_case_voicecall_answer(void* user_ptr, int test_num);
int my_ofono_case_voicecall_to_DTMF(void* user_ptr, int test_num);

// variable data
void *voicecall_variant_set_arg_processor(struct boxed_value *, void *);
void *dtmf_variant_set_arg_processor(struct boxed_value *, void *);
struct boxed_value *dtmf_variant_tones_generator(struct boxed_value *);

#endif /* CALL_CASES_H */
