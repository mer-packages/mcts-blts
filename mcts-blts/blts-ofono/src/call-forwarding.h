/* call-forwarding.h -- Voicecall forwarding functions

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


#ifndef CALL_FORWARDING_H
#define CALL_FORWARDING_H

#define OFONO_CALL_FORWARDING_INTERFACE		"org.ofono.CallForwarding"

#include <blts_params.h>

//disable
int ofono_call_forwarding_disable_all(void* user_ptr, char* modem_path, char *condition);	// NULL, "", "all", "conditional"
// forwardings
int ofono_call_forwarding_set_unconditional(void* user_ptr, char* modem_path, char *number);
int ofono_call_forwarding_set_busy(void* user_ptr, char* modem_path, char *number);
int ofono_call_forwarding_set_no_reply(void* user_ptr, char* modem_path, char *number);
int ofono_call_forwarding_set_not_reachable(void* user_ptr, char* modem_path, char *number);
int ofono_call_forwarding_set_no_reply_timeout(void* user_ptr, char* modem_path, uint timeout);

//variable parameters
void *forwarding_if_busy_variant_set_arg_processor(struct boxed_value *, void *);
void *forwarding_variant_set_arg_processor(struct boxed_value *, void *);

// debug
int ofono_call_forwarding_properties(void* user_ptr, char* modem_path);

int ofono_call_forwarding_check_settings(char *forwarding, char *number, void* user_ptr, char* modem_path);

#endif // CALL_FORWARDING_H
