/* alsa_ioctl.c -- ALSA core ioctls

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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include "alsa_ioctl.h"
#include "alsa_profiler.h"

#define CALL_IOCTL(a, b) \
	ioctl_profile(#b); \
	if(ioctl(a, b) < 0) \
	{ \
		err = -errno; \
		BLTS_ERROR(#b" failed (%d)\n", err); \
	}

#define CALL_IOCTL2(a, b, c) \
	ioctl_profile(#b); \
	if(ioctl(a, b, c) < 0) \
	{ \
		err = -errno; \
		BLTS_ERROR(#b" failed (%d)\n", err); \
	}

/*----------------------------------- PCM ------------------------------------*/

int ioctl_pcm_pversion(pcm_device *hw, int *ver)
{
	int err = 0;
	if(!hw || !ver)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_PVERSION, ver)
	return err;
}

int ioctl_pcm_info(pcm_device *hw, struct snd_pcm_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_pcm_info));
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_INFO, info)
	return err;
}

#ifdef SNDRV_PCM_IOCTL_TTSTAMP
int ioctl_pcm_ttstamp(pcm_device *hw, int *mode)
{
	int err = 0;
	if(!hw || !mode)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_TTSTAMP, mode)
	return err;
}
#endif /* SNDRV_PCM_IOCTL_TTSTAMP */

int ioctl_pcm_start(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	if(hw->sync_ptr)
		ioctl_pcm_sync_ptr(hw, 0);
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_START)
	return err;
}

int ioctl_pcm_hw_free(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_HW_FREE)
	return err;
}

int ioctl_pcm_sync_ptr(pcm_device *hw, unsigned int flags)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	hw->sync_ptr->flags = flags;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_SYNC_PTR, hw->sync_ptr)
	return err;
}

int ioctl_pcm_hwsync(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	if(hw->sync_ptr)
		err = ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_HWSYNC);
	else
		CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_HWSYNC)
	return err;
}

snd_pcm_sframes_t ioctl_pcm_writei(pcm_device *hw, const void *buffer,
	snd_pcm_uframes_t size)
{
	int err = 0;
	struct snd_xferi xferi;

	if(!hw || !buffer)
		return -EINVAL;

	xferi.buf = (char*) buffer;
	xferi.frames = size;
	xferi.result = 0;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi)
	if(err >= 0 && hw->sync_ptr)
		ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
	return xferi.result;
}

snd_pcm_sframes_t ioctl_pcm_writen(pcm_device *hw, void **bufs,
	snd_pcm_uframes_t size)
{
	int err = 0;
	struct snd_xfern xfern;

	if(!hw || !bufs)
		return -EINVAL;

	memset(&xfern, 0, sizeof(xfern));
	xfern.bufs = bufs;
	xfern.frames = size;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &xfern)
	if(err >= 0 && hw->sync_ptr)
		ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
	return xfern.result;
}

snd_pcm_sframes_t ioctl_pcm_readi(pcm_device *hw, const void *buffer,
	snd_pcm_uframes_t size)
{
	int err = 0;
	struct snd_xferi xferi;

	if(!hw || !buffer)
		return -EINVAL;

	xferi.buf = (char*) buffer;
	xferi.frames = size;
	xferi.result = 0;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_READI_FRAMES, &xferi)
	if (err < 0)
		return err;
	return xferi.result;
}

snd_pcm_sframes_t ioctl_pcm_readn(pcm_device *hw, void **bufs,
	snd_pcm_uframes_t size)
{
	int err = 0;
	struct snd_xfern xfern;

	if(!hw || !bufs)
		return -EINVAL;

	memset(&xfern, 0, sizeof(xfern));
	xfern.bufs = bufs;
	xfern.frames = size;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_READN_FRAMES, &xfern)
	if(err >= 0 && hw->sync_ptr)
		ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);

	if(err < 0)
		return err;
		
	return xfern.result;
}

int ioctl_pcm_prepare(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_PREPARE)
	if(err >= 0 && hw->sync_ptr)
		ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
	return err;
}

int ioctl_pcm_drop(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_DROP)
	return err;
}

int ioctl_pcm_resume(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_RESUME)
	return err;
}

int ioctl_pcm_hw_params(pcm_device *hw, snd_pcm_hw_params_t *params)
{
	int err = 0;
	if(!hw || !params)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_HW_PARAMS, params)
	if(err >= 0)
	{
		params->info &= ~0xf0000000;
		if(hw->sync_ptr)
			err = ioctl_pcm_sync_ptr(hw, 0);
	}
	return err;
}

int ioctl_pcm_sw_params(pcm_device *hw, snd_pcm_sw_params_t *params)
{
	int err = 0;
	if(!hw || !params)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_SW_PARAMS, params)
	return err;
}

int ioctl_pcm_refine(pcm_device *hw, snd_pcm_hw_params_t *params)
{
	int err = 0;
	if(!hw || !params)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_HW_REFINE, params)
	if(err >= 0)
	{
		if(params->info != ~0U)
		{
			params->info &= ~0xf0000000;
			params->info |= (hw->monotonic ? 0x80000000 : 0);
		}
	}

	return err;
}

int ioctl_pcm_channel_info(pcm_device *hw, struct snd_pcm_channel_info *info,
	int chan)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_pcm_channel_info));
	info->channel = chan;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_CHANNEL_INFO, info)
	return err;
}

int ioctl_pcm_status(pcm_device *hw, struct snd_pcm_status *status)
{
	int err = 0;
	if(!hw || !status)
		return -EINVAL;
	memset(status, 0, sizeof(struct snd_pcm_status));
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_STATUS, status)
	return err;
}

int ioctl_pcm_delay(pcm_device *hw, snd_pcm_sframes_t *delay)
{
	int err = 0;
	if(!hw || !delay)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_DELAY, delay)
	return err;
}

int ioctl_pcm_link(pcm_device *hw, int fd)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_PCM_IOCTL_LINK, fd)
	return err;
}

int ioctl_pcm_unlink(pcm_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_PCM_IOCTL_UNLINK)
	return err;
}

/*----------------------------------- CTL ------------------------------------*/

int ioctl_ctl_pversion(ctl_device *hw, int *ver)
{
	int err = 0;
	if(!hw || !ver)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_PVERSION, ver)
	return err;
}

int ioctl_ctl_hw_card_info(ctl_device *hw, snd_ctl_card_info_t *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_CARD_INFO, info)
	return err;
}

int ioctl_ctl_elem_list(ctl_device *hw, struct snd_ctl_elem_list *list)
{
	int err = 0;
	if(!hw || !list)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list)
	return err;
}

int ioctl_ctl_elem_info(ctl_device *hw, struct snd_ctl_elem_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info)
	return err;
}

int ioctl_ctl_elem_read(ctl_device *hw, struct snd_ctl_elem_value *value)
{
	int err = 0;
	if(!hw || !value)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_READ, value)
	return err;
}

int ioctl_ctl_elem_write(ctl_device *hw, struct snd_ctl_elem_value *value)
{
	int err = 0;
	if(!hw || !value)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, value)
	return err;
}

int ioctl_ctl_hwdep_next_dev(ctl_device *hw, int *dev_num)
{
	int err = 0;
	if(!hw || !dev_num)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_HWDEP_NEXT_DEVICE, dev_num)
	return err;
}

int ioctl_ctl_hwdep_info(ctl_device *hw, unsigned int device,
	struct snd_hwdep_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_hwdep_info));
	info->device = device;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_HWDEP_INFO, info)
	return err;
}

int ioctl_ctl_pcm_next_dev(ctl_device *hw, int *dev_num)
{
	int err = 0;
	if(!hw || !dev_num)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE, dev_num)
	return err;
}

int ioctl_ctl_pcm_info(ctl_device *hw, int device, int subdevice, int stream,
	struct snd_pcm_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_pcm_info));
	info->device = device;
	info->subdevice = subdevice;
	info->stream = stream;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_PCM_INFO, info)
	return err;
}

int ioctl_ctl_rawmidi_next_dev(ctl_device *hw, int *dev_num)
{
	int err = 0;
	if(!hw || !dev_num)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_RAWMIDI_NEXT_DEVICE, dev_num)
	return err;
}

int ioctl_ctl_rawmidi_info(ctl_device *hw, int device, int subdevice,
	int stream, struct snd_rawmidi_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_rawmidi_info));
	info->device = device;
	info->subdevice = subdevice;
	info->stream = stream;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_RAWMIDI_INFO, info)
	return err;
}

int ioctl_ctl_power(ctl_device *hw, int *state)
{
	int err = 0;
	if(!hw || !state)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_POWER, state)
	return err;
}

int ioctl_ctl_power_state(ctl_device *hw, int *state)
{
	int err = 0;
	if(!hw || !state)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_POWER_STATE, state)
	return err;
}

int ioctl_ctl_elem_add(ctl_device *hw, struct snd_ctl_elem_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_ADD, info)
	return err;
}

int ioctl_ctl_elem_replace(ctl_device *hw, struct snd_ctl_elem_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_REPLACE, info)
	return err;
}

int ioctl_ctl_elem_remove(ctl_device *hw, struct snd_ctl_elem_id *id)
{
	int err = 0;
	if(!hw || !id)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_REMOVE, id)
	return err;
}

int ioctl_ctl_elem_lock(ctl_device *hw, struct snd_ctl_elem_id *id)
{
	int err = 0;
	if(!hw || !id)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_LOCK, id)
	return err;
}

int ioctl_ctl_elem_unlock(ctl_device *hw, struct snd_ctl_elem_id *id)
{
	int err = 0;
	if(!hw || !id)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, id)
	return err;
}

/*---------------------------------- Timer -----------------------------------*/

int ioctl_timer_pversion(timer_device *hw, int *ver)
{
	int err = 0;
	if(!hw || !ver)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_PVERSION, ver)
	return err;
}

int ioctl_timer_info(timer_device *hw, struct snd_timer_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_timer_info));
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_INFO, info)
	return err;
}

int ioctl_timer_next_device(timer_device *hw, struct snd_timer_id *id)
{
	int err = 0;
	if(!hw || !id)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_NEXT_DEVICE, id)
	return err;
}

int ioctl_timer_select(timer_device *hw, struct snd_timer_select *select)
{
	int err = 0;
	if(!hw || !select)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_SELECT, select)
	return err;
}

int ioctl_timer_start(timer_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_TIMER_IOCTL_START);
	return err;
}

int ioctl_timer_stop(timer_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_TIMER_IOCTL_STOP);
	return err;
}

int ioctl_timer_continue(timer_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_TIMER_IOCTL_CONTINUE);
	return err;
}

int ioctl_timer_pause(timer_device *hw)
{
	int err = 0;
	if(!hw)
		return -EINVAL;
	CALL_IOCTL(hw->fd, SNDRV_TIMER_IOCTL_PAUSE);
	return err;
}

int ioctl_timer_tread(timer_device *hw, int *enable)
{
	int err = 0;
	if(!hw || !enable)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_TREAD, enable);
	return err;
}

int ioctl_timer_ginfo(timer_device *hw, struct snd_timer_ginfo *ginfo)
{
	int err = 0;
	if(!hw || !ginfo)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_GINFO, ginfo);
	return err;
}

int ioctl_timer_gparams(timer_device *hw, struct snd_timer_gparams *gparams)
{
	int err = 0;
	if(!hw || !gparams)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_GPARAMS, gparams);
	return err;
}

int ioctl_timer_gstatus(timer_device *hw, struct snd_timer_gstatus *gstatus)
{
	int err = 0;
	if(!hw || !gstatus)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_GSTATUS, gstatus);
	return err;
}

int ioctl_timer_params(timer_device *hw, struct snd_timer_params *params)
{
	int err = 0;
	if(!hw || !params)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_PARAMS, params);
	return err;
}

int ioctl_timer_status(timer_device *hw, struct snd_timer_status *status)
{
	int err = 0;
	if(!hw || !status)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_TIMER_IOCTL_STATUS, status);
	return err;
}

/*---------------------------------- HWdep -----------------------------------*/

int ioctl_hwdep_pversion(hwdep_device *hw, int *ver)
{
	int err = 0;
	if(!hw || !ver)
		return -EINVAL;
	CALL_IOCTL2(hw->fd, SNDRV_HWDEP_IOCTL_PVERSION, ver)
	return err;
}

int ioctl_hwdep_info(hwdep_device *hw, struct snd_hwdep_info *info)
{
	int err = 0;
	if(!hw || !info)
		return -EINVAL;
	memset(info, 0, sizeof(struct snd_hwdep_info));
	CALL_IOCTL2(hw->fd, SNDRV_HWDEP_IOCTL_INFO, info)
	return err;
}

int ioctl_hwdep_dsp_status(hwdep_device *hw,
	struct snd_hwdep_dsp_status *status)
{
	int err = 0;
	if(!hw || !status)
		return -EINVAL;
	memset(status, 0, sizeof(struct snd_hwdep_dsp_status));
	CALL_IOCTL2(hw->fd, SNDRV_HWDEP_IOCTL_DSP_STATUS, status)
	return err;
}

