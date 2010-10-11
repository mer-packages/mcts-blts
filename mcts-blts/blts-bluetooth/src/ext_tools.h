/* ext_tools.h -- External tools wrapper

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

#ifndef EXT_TOOLS_H
#define EXT_TOOLS_H

#include <stdint.h>

/* hcidump related --> */

/** representation for one packet capture by hcidump */
struct hcidump_trace {
	long tv_sec;
	long tv_usec;
	int incoming;
	uint8_t *data;
	unsigned data_len;
};

typedef int (*hcidump_trace_handler)(struct hcidump_trace *trace_event, void *user_data);

int start_hcidump(char* dump_file, char *hci_device);
int stop_hcidump();
int parse_hcidump(char* dump_file, hcidump_trace_handler trace_handler, void *user_data);

#endif /* EXT_TOOLS_H */
