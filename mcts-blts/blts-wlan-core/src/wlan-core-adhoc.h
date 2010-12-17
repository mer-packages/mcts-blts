/* wlan-core-adhoc.h -- WLAN core adhoc functions

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

#ifndef ADHOC_H_
#define ADHOC_H_

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
#include "wlan-core-connect.h"

int join_established_open_adhoc_network(wlan_core_data* data);
int create_open_adhoc_network(wlan_core_data* data);

int do_test_data_sending(wlan_core_data* data, struct associate_params *params);

#endif /*ADHOC_H_ */
