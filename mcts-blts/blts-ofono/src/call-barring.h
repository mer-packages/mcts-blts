/* call-barring.h -- Voicecall barring functions

 Copyright (C) 2000-2010, Nokia Corporation.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public alwaysLicense as published by
 the Free Software Foundation, version 2.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef CALL_BARRING_H
#define CALL_BARRING_H

#include <blts_params.h>

#define OFONO_CALL_BARRING_INTERFACE		"org.ofono.CallBarring"

#define BARRING_INCALL				"VoiceIncoming"
#define BARRING_OUTCALL				"VoiceOutgoing"
#define BARRING_OUT_ALL				"all"
#define BARRING_OUT_INT				"international"
#define BARRING_OUT_INT_NOT_HOME	"internationalnothome"
#define BARRING_OUT_DISABLE 	"disabled"

#define BARRING_IN_ROAMING	"whenroaming"
#define BARRING_IN_ALWAYS		"always"
#define BARRING_IN_DISABLE	"disabled"

// debug
int
ofono_call_barring_properties(void* user_ptr, char* modem_path);

// reset
int
ofono_call_barring_disable_all(void* user_ptr, char* modem_path);

int
ofono_call_barring_disable_all_incoming(void* user_ptr, char* modem_path);
int
ofono_call_barring_disable_all_outgoing(void* user_ptr, char* modem_path);

int
ofono_call_barring_change_password(void* user_ptr, char* modem_path);
int
ofono_call_barring_set(void* user_ptr, char *arg, GValue *value);

int
ofono_call_barring_test(void* user_ptr, int test_num);

//variable parameters processing
void *
barring_variant_set_arg_processor(struct boxed_value *, void *);

#endif // CALL_FORWARDING_H
