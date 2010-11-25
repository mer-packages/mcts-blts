/* wlan-core-cmdqueue.c -- WLAN core command queue functions

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


#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#include <blts_log.h>
#include <blts_timing.h>

#include "wlan-core-cmdqueue.h"
#include "wlan-core-utils.h"
#include "wlan-core-debug.h"

struct cmd_que_item {
	struct cmd_que_item *next;
	int cmd_id;
};

static struct cmd_que_item *cmd_que_tail = NULL;
static struct cmd_que_item *cmd_que_head = NULL;

static pthread_mutex_t cmd_que_mutex = PTHREAD_MUTEX_INITIALIZER;

int wait_for_cmd(int cmd_id, double timeout)
{
	int cmd = -1;

	BLTS_DEBUG("Waiting for command %s (0x%x)...\n",
			nl80211_cmd_as_string(cmd_id), cmd_id);
	timing_start();

	while (timing_elapsed() < timeout )
	{
		cmd = first_cmd_from_queue();
		BLTS_DEBUG(".");

		if(cmd < 0)
			usleep(100000);

		if (cmd)
		{
			if (cmd == cmd_id)
			{
				BLTS_DEBUG("Command received\n");
				timing_stop();
				return 0;
			}
		}
		usleep(100000);
	}
	timing_stop();
	BLTS_DEBUG("Command not received within %.3f seconds\n", timeout);

	return -1;
}

int add_to_cmd_queue(int cmd)
{
	struct cmd_que_item *item;
	int ret = 0;

	pthread_mutex_lock(&cmd_que_mutex);

	item = malloc(sizeof(struct cmd_que_item));
	if (!item)
	{
		BLTS_LOGGED_PERROR("malloc");
		ret = -errno;
		goto cleanup;
	}

	memset(item, 0, sizeof(*item));
	item->cmd_id = cmd;

	if (cmd_que_tail)
		cmd_que_tail->next = item;
	else
		cmd_que_head = item;
	cmd_que_tail = item;

cleanup:
	pthread_mutex_unlock(&cmd_que_mutex);

	return ret;
}

int first_cmd_from_queue()
{
	int cmd = -1;
	struct cmd_que_item *item;

	pthread_mutex_lock(&cmd_que_mutex);
	if (!cmd_que_head )
		goto cleanup;

	item = cmd_que_head;
	cmd = item->cmd_id;

	BLTS_DEBUG("\nCOMMAND: %s\n", nl80211_cmd_as_string(cmd));

	if (item->next)
		cmd_que_head = item->next;
	else
		cmd_que_head = cmd_que_tail = NULL;

	free(item);

cleanup:
	pthread_mutex_unlock(&cmd_que_mutex);

	return cmd;
}

void clear_cmd_queue()
{
	BLTS_DEBUG("\nclear_cmd_queue\n");
	struct cmd_que_item *item, *next;

	pthread_mutex_lock(&cmd_que_mutex);

	item = cmd_que_head;

	while (item) {
		next = item->next;
		free(item);
		item = next;
	}

	cmd_que_head = NULL;
	cmd_que_tail = NULL;

	pthread_mutex_unlock(&cmd_que_mutex);
}
