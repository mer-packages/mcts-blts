/* sms-cases.h -- SMS cases

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

#ifndef SMS_CASES_H
#define SMS_CASES_H

#include <blts_params.h>

int blts_ofono_send_sms_default(void*,int);
int blts_ofono_receive_sms_default(void*,int);
int ofono_sms_center_number(void*,int);

//variable parameters
void *sms_send_variant_set_arg_processor(struct boxed_value *, void *);
void *sms_smsc_variant_set_arg_processor(struct boxed_value *, void *);
void *sms_recv_variant_set_arg_processor(struct boxed_value *, void *);
struct boxed_value *sms_variant_message_generator(struct boxed_value *);

#endif /* SMS_CASES_H */
