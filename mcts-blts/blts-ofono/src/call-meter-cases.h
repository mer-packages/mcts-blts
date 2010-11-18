/* call-meter-cases.h -- Call meter cases for blts-ofono

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

#ifndef CALL_METER_CASES_H
#define CALL_METER_CASES_H

#include <blts_params.h>


int blts_ofono_read_call_meter_data(void*,int);
int blts_ofono_set_call_meter_data(void*,int);
int blts_ofono_reset_call_meter_data(void*,int);
int blts_ofono_test_near_max_warning(void*, int);

void *call_meter_variant_read_arg_processor(struct boxed_value *, void *);
void *call_meter_variant_set_arg_processor(struct boxed_value *, void *);
void *call_meter_variant_reset_arg_processor(struct boxed_value*, void*);

#endif /* CALL_METER_CASES_H */
