/* wlan-core-cmdqueue.h -- WLAN core command queue functions

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

#ifndef CMD_QUEUE_H_
#define CMD_QUEUE_H_

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

#include <pthread.h>

#include "wlan-core-definitions.h"

int wait_for_msg(int cmd_id, double timeout);

int add_to_cmd_queue(int cmd);

int first_cmd_from_queue();

void clear_cmd_queue();

int wait_for_cmd(int cmd_id, double timeout);

#endif /* CMD_QUEUE_H_ */
