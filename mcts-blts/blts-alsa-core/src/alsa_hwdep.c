/* alsa_hwdep.c -- ALSA hwdep IF functionality

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
#include "alsa_hwdep.h"
#include "alsa_ioctl.h"

hwdep_device* hwdep_open(int card, int device)
{
	char filename[MAX_DEV_NAME_LEN];
	hwdep_device* hw = NULL;

	if(card < 0 || card >= MAX_CARDS)
	{
		BLTS_ERROR("Invalid card index %d\n", card);
		return NULL;
	}

	hw = calloc(1, sizeof(hwdep_device));
	if(!hw)
	{
		logged_perror("Failed to allocate hwdep_device structure");
		return NULL;
	}

	sprintf(filename, "/dev/snd/hwC%dD%d", card, device);

	BLTS_TRACE("Opening hwdep %s\n", filename);
	hw->fd = open(filename, O_RDWR);
	if(!hw->fd)
	{
		BLTS_ERROR("Failed to open device '%s'\n", filename);
		return NULL;
	}

	if(ioctl_hwdep_pversion(hw, &hw->protocol))
		goto error_exit;

	BLTS_TRACE("Protocol version %d.%d.%d\n",
		SNDRV_PROTOCOL_MAJOR(hw->protocol),
		SNDRV_PROTOCOL_MINOR(hw->protocol),
		SNDRV_PROTOCOL_MICRO(hw->protocol));

	if(hw->protocol < SNDRV_PROTOCOL_VERSION(1, 0, 1))
	{
		BLTS_ERROR("Protocol version < 1.0.1 not supported\n");
		goto error_exit;
	}
	
	hw->card = card;
	hw->device = device;

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

int hwdep_close(hwdep_device* hw)
{
	if(!hw)
		return -EINVAL;

	if(hw->fd)
		close(hw->fd);

	free(hw);

	return 0;
}

int hwdep_print_info(hwdep_device* hw)
{
	int err = 0;
	struct snd_hwdep_info info;
	struct snd_hwdep_dsp_status status;

	err = ioctl_hwdep_info(hw, &info);
	if(err)
		return err;

	BLTS_DEBUG("HW dep info:\n");
	BLTS_DEBUG("\tdevice: %d\n", info.device);
	BLTS_DEBUG("\tcard: %d\n", info.card);
	BLTS_DEBUG("\tid: %s\n", info.id);
	BLTS_DEBUG("\tname: %s\n", info.name);
	BLTS_DEBUG("\tiface: %d\n", info.iface);

	/* NOTE: DSP is available only with drivers (with stock 2.6.31 kernel);
	 * Digigram pcxhr compatible
	 * Tascam US-X2Y USB soundcards
	 * Digigram miXart soundcards
	 * Digigram VX soundcards
	 */
	err = ioctl_hwdep_dsp_status(hw, &status);
	if(err == -ENXIO)
	{
		BLTS_DEBUG("No DSP.\n");
		return 0;
	}
	else if(err)
		return err;

	BLTS_DEBUG("DSP info:\n");
	BLTS_DEBUG("\tversion: %d\n", status.version);
	BLTS_DEBUG("\tid: %s\n", status.id);
	BLTS_DEBUG("\tnum_dsps: %d\n", status.num_dsps);
	BLTS_DEBUG("\tdsp_loaded: %d\n", status.dsp_loaded);
	BLTS_DEBUG("\tchip_ready: %d\n", status.chip_ready);

	return 0;
}

