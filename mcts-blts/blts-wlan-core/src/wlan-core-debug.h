/* wlan-core-debug.h -- WLAN core debug and trace functions

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

#ifndef DEBUG_H_
#define DEBUG_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>			/* getopt_long() */

#include <fcntl.h>			/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>

#include "wlan-core-definitions.h"

int print_event(struct nl_msg *msg, void *arg);

void hexdump(const char *title, const u8 *buf,	size_t len);
void hexdump_ascii(const char *title, const u8 *buf, size_t len);
const char *nl80211_cmd_as_string(unsigned cmd);

#endif /*DEBUG_H_*/
