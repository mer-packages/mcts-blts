/* ofono-util.h -- oFono utility functions

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

#ifndef OFONO_UTIL_H
#define OFONO_UTIL_H

#include "ofono-general.h"

typedef struct
{
	char phonenumber[3][32];
	char *modem[MAX_MODEMS];
	int number_modems;
	char *current_modem;
	DBusGConnection *connection;
	GMainLoop *mainloop;
	DBusGProxy *proxy_to_use;
	long timeout;

	char *remote_address;
	char *smsc_address;
	char *bearer;
	char *sms_generated_message;
	char *forward_address;

	char *old_pin;
	char *new_pin;
	char *pin_type;
	char *barrings_pin;

	int user_timeout;
	char* volume;

	char* accu_cm_max;
	char* ppu;
	char* currency;

	char* dtmf_tone;	//dtmf case variable data

	/* Option flags */

	unsigned fl_dontcleanup;  /* When set, skip call cleanups */

} my_ofono_data;

gboolean check_state(gpointer key, gpointer value, gpointer user_data);
void get_modem_list(char* data, gpointer user_data);
//void find_modem(char *key, GValue* value, gpointer user_data);
void find_modem(gpointer modem,  gpointer user_data);
int my_ofono_get_modem(my_ofono_data* data);
void hash_entry_gvalue_print(gpointer key, GValue* val, __attribute__((unused)) gpointer user_data);

int ensure_calls_cleared(my_ofono_data *data);
int reset_supplementary_services(my_ofono_data* data);
int set_modems_power_on(my_ofono_data *data);
int set_modems_online(my_ofono_data *data);

void display_dbus_glib_error(GError *error);

#endif /* OFONO_UTIL_H */
