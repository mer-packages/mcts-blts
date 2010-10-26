/* alsa_timer.c -- ALSA timer IF functionality

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "alsa_util.h"
#include "alsa_timer.h"
#include "alsa_ioctl.h"

timer_device* timer_open()
{
	char filename[MAX_DEV_NAME_LEN];
	timer_device* hw = NULL;

	hw = calloc(1, sizeof(timer_device));
	if(!hw)
	{
		BLTS_LOGGED_PERROR("Failed to allocate timer_device structure");
		return NULL;
	}

	sprintf(filename, "/dev/snd/timer");

	BLTS_TRACE("Opening timer %s\n", filename);
	hw->fd = open(filename, O_RDWR);
	if(!hw->fd)
	{
		BLTS_ERROR("Failed to open device '%s'\n", filename);
		return NULL;
	}

	if(ioctl_timer_pversion(hw, &hw->protocol))
		goto error_exit;

	BLTS_TRACE("Protocol version %d.%d.%d\n",
		SNDRV_PROTOCOL_MAJOR(hw->protocol),
		SNDRV_PROTOCOL_MINOR(hw->protocol),
		SNDRV_PROTOCOL_MICRO(hw->protocol));

	if(hw->protocol < SNDRV_PROTOCOL_VERSION(2, 0, 5))
	{
		BLTS_ERROR("Protocol version < 2.0.5 not supported\n");
		goto error_exit;
	}

	return hw;

error_exit:
	if(hw)
	{
		if(hw->fd)
			close(hw->fd);
		free(hw);
	}

	return NULL;
}

int timer_close(timer_device* hw)
{
	if(!hw)
		return -EINVAL;

	if(hw->fd)
		close(hw->fd);

	free(hw);

	return 0;
}

int timer_print_info(timer_device* hw)
{
	int err = 0;
	struct snd_timer_info info;
	struct snd_timer_id id;
	struct snd_timer_select sel;
	struct snd_timer_ginfo ginfo;
	struct snd_timer_gstatus gstatus;
	struct snd_timer_status status;

	id.card = id.device = id.dev_class = -1;

	while(1)
	{
		err = ioctl_timer_next_device(hw, &id);
		if(err)
			return err;

		if(id.card == -1 && id.device == -1 && id.dev_class == -1)
			break;

		memset(&sel, 0, sizeof(sel));
		sel.id = id;

		err = ioctl_timer_select(hw, &sel);
		if(err)
			return err;

		err = ioctl_timer_info(hw, &info);
		if(err)
			return err;

		memset(&ginfo, 0, sizeof(ginfo));
		ginfo.tid = id;
		err = ioctl_timer_ginfo(hw, &ginfo);
		if(err)
			return err;

		memset(&gstatus, 0, sizeof(gstatus));
		gstatus.tid = id;
		err = ioctl_timer_gstatus(hw, &gstatus);
		if(err)
			return err;

		memset(&status, 0, sizeof(status));
		err = ioctl_timer_status(hw, &status);
		if(err)
			return err;

		BLTS_DEBUG("Timer '%s':\n", info.name);
		BLTS_DEBUG("\tcard: %d\n", info.card);
		BLTS_DEBUG("\tid: %s\n", info.id);
		BLTS_DEBUG("\tflags: %d\n", info.flags);
		BLTS_DEBUG("\tglobal resolution (in ns): %u\n", ginfo.resolution);
		BLTS_DEBUG("\tglobal resolution min: %u\n", ginfo.resolution_min);
		BLTS_DEBUG("\tglobal resolution max: %u\n", ginfo.resolution_max);
		BLTS_DEBUG("\tglobal precise resolution: %u/%u\n", gstatus.resolution_num,
			gstatus.resolution_den);
		BLTS_DEBUG("\tactive clients: %u\n", ginfo.clients);
		BLTS_DEBUG("\ttstamp = %ld.%ld\n", status.tstamp.tv_sec,
			status.tstamp.tv_nsec);
		BLTS_DEBUG("\tresolution: %u\n", status.resolution);
		BLTS_DEBUG("\tlost: %u\n", status.lost);
		BLTS_DEBUG("\toverrun: %u\n", status.overrun);
		BLTS_DEBUG("\tqueue: %u\n", status.queue);
	}

	return 0;
}

static int timer_print_status_real(timer_device* hw, int level)
{
	int ret;
	struct snd_timer_info info;
	struct snd_timer_status status;

	ret = ioctl_timer_info(hw, &info);
	if (ret) {
		BLTS_ERROR("Error in timer info ioctl.\n");
		return ret;
	}
	ret = ioctl_timer_status(hw, &status);
	if (ret) {
		BLTS_ERROR("Error in timer status ioctl.\n");
		return ret;
	}

	blts_log_print_level(level, "Timer '%s' status:\n", info.name);
	blts_log_print_level(level, "\ttstamp = { %ld, %ld }\n", status.tstamp.tv_sec,
		status.tstamp.tv_nsec);
	blts_log_print_level(level, "\tresolution = %u\n", status.resolution);
	blts_log_print_level(level, "\tlost = %u\n", status.lost);
	blts_log_print_level(level, "\toverrun = %u\n", status.overrun);
	blts_log_print_level(level, "\tqueue = %u\n", status.queue);

	return 0;
}

int timer_print_status(timer_device* hw)
{
	return timer_print_status_real(hw, LEVEL_DEBUG);
}

int timer_print_status_trace(timer_device *hw)
{
	return timer_print_status_real(hw, LEVEL_TRACE);
}

static int timer_select_specific(timer_device *hw, int card, int device)
{
	int err = 0;
	struct snd_timer_info info;
	struct snd_timer_id id;
	struct snd_timer_select sel;

	memset(&id, 0, sizeof id);
	id.card = id.device = id.dev_class = -1;

	while(1)
	{
		err = ioctl_timer_next_device(hw, &id);
		if(err)
			return err;

		if(id.card == -1 && id.device == -1 && id.dev_class == -1)
			break;

		memset(&sel, 0, sizeof(sel));
		sel.id = id;

		err = ioctl_timer_select(hw, &sel);
		if(err)
			return err;

		err = ioctl_timer_info(hw, &info);
		if(err)
			return err;
		if(device >= 0) {
			if(info.card == card
				&& sel.id.card == card
				&& sel.id.device == device)
				return 0;
		}
		if(info.card == card && sel.id.card == card)
			return 0;
	}

	return -EINVAL;
}

int timer_select_for_card(timer_device *hw, int card)
{
	return timer_select_specific(hw, card, -1);
}

int timer_select_for_device(timer_device *hw, int card, int device)
{
	return timer_select_specific(hw, card, device);
}


int timer_set_period(timer_device *hw, unsigned long period_ns)
{
	int ret;
	struct snd_timer_info info;
	struct snd_timer_params params;

	ret = ioctl_timer_info(hw, &info);
	if (ret)
		return ret;

	memset(&params, 0, sizeof params);
	params.flags |= SNDRV_TIMER_PSFLG_AUTO;
//	params.filter = SNDRV_TIMER_EVENT_TICK;
	if (info.flags & SNDRV_TIMER_FLG_SLAVE)
		params.ticks = 1;
	else
		params.ticks = (1000000000LL / info.resolution) * period_ns;

	ret = ioctl_timer_params(hw, &params);
	if (ret)
		return ret;

	return timer_print_status_trace(hw);
}

int timer_read(timer_device *hw, unsigned *resolution, unsigned *ticks)
{
	unsigned buf[2];
	int ret;

	if (!hw)
		return -EINVAL;

	ret = read(hw->fd, buf, ARRAY_SIZE(buf));
	if (ret)
		return ret;

	if (resolution)
		*resolution = buf[0];
	if (ticks)
		*ticks = buf[1];

	return 0;
}

int timer_get_resolution_ns(timer_device *hw, unsigned long *resolution)
{
	int ret;
	struct snd_timer_status status;

	if (!hw || !resolution)
		return -EINVAL;
	ret = ioctl_timer_status(hw, &status);
	if (ret)
		return ret;
	*resolution = status.resolution;
	return 0;
}
