/* usb-util.h -- USB utility functions

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

#ifndef USB_UTIL_H
#define USB_UTIL_H

#include "usb-general.h"

typedef struct
{
	int data_transfer_timeout;
	char *host_driver;
	char *host_driver_path;
	char *peripheral_driver;
	char *peripheral_driver_path;
	char* usb_speed;
	int usb_max_power;
	int endpoint_count;
	char* endpoint_type;
	int endpoint_max_packet_size;
	int endpoint_interval;
	int endpoint_transfer_size;
	char *endpoint_configuration_file;
} my_usb_data;

#endif /* USB_UTIL_H */
