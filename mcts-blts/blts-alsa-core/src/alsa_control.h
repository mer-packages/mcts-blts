/* alsa_control.h -- ALSA control IF functionality

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


#ifndef ALSA_CONTROL_H
#define ALSA_CONTROL_H

#include "alsa_util.h"

typedef struct {
	struct snd_ctl_elem_info info;
	struct snd_ctl_elem_value value;
} ctl_element;

ctl_device* control_open(int card);
int control_close(ctl_device* hw);
ctl_element* control_read(ctl_device* hw, const char* name);
ctl_element* control_read_by_id(ctl_device* hw, unsigned int id);
int control_write(ctl_device* hw, ctl_element* element, long long value);
int control_write_single(ctl_device* hw, ctl_element* element, int chan,
	long long value);
int contol_value_by_name(ctl_device* hw, ctl_element* element, char *name,
	long long *value);

int control_print_card_info(ctl_device* hw);
int control_print_element_list(ctl_device* hw);
int control_print_element_value(ctl_device* hw, ctl_element* element);
const char* power_state_to_string(int state);
int control_enumerate_devices(ctl_device* hw);

#endif /* ALSA_CONTROL_H */

