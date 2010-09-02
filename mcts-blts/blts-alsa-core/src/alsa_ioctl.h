/* alsa_ioctl.h -- IF for ALSA core ioctls

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

#ifndef ALSA_IOCTL_H
#define ALSA_IOCTL_H

#include "alsa_util.h"

/** PCM */

int ioctl_pcm_pversion(pcm_device *hw, int *ver);
int ioctl_pcm_info(pcm_device *hw, struct snd_pcm_info *info);
#ifdef SNDRV_PCM_IOCTL_TTSTAMP
int ioctl_pcm_ttstamp(pcm_device *hw, int *mode);
#endif /* SNDRV_PCM_IOCTL_TTSTAMP */
int ioctl_pcm_start(pcm_device *hw);
int ioctl_pcm_hw_free(pcm_device *hw);
int ioctl_pcm_sync_ptr(pcm_device *hw, unsigned int flags);
int ioctl_pcm_hwsync(pcm_device *hw);
snd_pcm_sframes_t ioctl_pcm_writei(pcm_device *hw, const void *buffer,
	snd_pcm_uframes_t size);
snd_pcm_sframes_t ioctl_pcm_writen(pcm_device *hw, void **bufs,
	snd_pcm_uframes_t size);
snd_pcm_sframes_t ioctl_pcm_readi(pcm_device *hw, const void *buffer,
	snd_pcm_uframes_t size);
snd_pcm_sframes_t ioctl_pcm_readn(pcm_device *hw, void **bufs,
	snd_pcm_uframes_t size);
int ioctl_pcm_prepare(pcm_device *hw);
int ioctl_pcm_refine(pcm_device *hw, snd_pcm_hw_params_t *params);
int ioctl_pcm_sw_params(pcm_device *hw, snd_pcm_sw_params_t * params);
int ioctl_pcm_hw_params(pcm_device *hw, snd_pcm_hw_params_t * params);
int ioctl_pcm_resume(pcm_device *hw);
int ioctl_pcm_drop(pcm_device *hw);
int ioctl_pcm_channel_info(pcm_device *hw, struct snd_pcm_channel_info *info,
	int chan);
int ioctl_pcm_status(pcm_device *hw, struct snd_pcm_status *status);
int ioctl_pcm_delay(pcm_device *hw, snd_pcm_sframes_t *delay);
int ioctl_pcm_link(pcm_device *hw, int fd);
int ioctl_pcm_unlink(pcm_device *hw);

/** CTL */

int ioctl_ctl_pversion(ctl_device *hw, int *ver);
int ioctl_ctl_hw_card_info(ctl_device *hw, snd_ctl_card_info_t *info);
int ioctl_ctl_elem_list(ctl_device *hw, struct snd_ctl_elem_list *list);
int ioctl_ctl_elem_info(ctl_device *hw, struct snd_ctl_elem_info *info);
int ioctl_ctl_elem_read(ctl_device *hw, struct snd_ctl_elem_value *value);
int ioctl_ctl_elem_write(ctl_device *hw, struct snd_ctl_elem_value *value);
int ioctl_ctl_hwdep_next_dev(ctl_device *hw, int *dev_num);
int ioctl_ctl_hwdep_info(ctl_device *hw, unsigned int device,
	struct snd_hwdep_info *info);
int ioctl_ctl_pcm_next_dev(ctl_device *hw, int *dev_num);
int ioctl_ctl_pcm_info(ctl_device *hw, int device, int subdevice, int stream,
	struct snd_pcm_info *info);
int ioctl_ctl_rawmidi_next_dev(ctl_device *hw, int *dev_num);
int ioctl_ctl_rawmidi_info(ctl_device *hw, int device, int subdevice, 
	int stream, struct snd_rawmidi_info *info);
int ioctl_ctl_power(ctl_device *hw, int *state);
int ioctl_ctl_power_state(ctl_device *hw, int *state);
int ioctl_ctl_elem_add(ctl_device *hw, struct snd_ctl_elem_info *info);
int ioctl_ctl_elem_replace(ctl_device *hw, struct snd_ctl_elem_info *info);
int ioctl_ctl_elem_remove(ctl_device *hw, struct snd_ctl_elem_id *id);
int ioctl_ctl_elem_lock(ctl_device *hw, struct snd_ctl_elem_id *id);
int ioctl_ctl_elem_unlock(ctl_device *hw, struct snd_ctl_elem_id *id);


/** Timer */

int ioctl_timer_pversion(timer_device *hw, int *ver);
int ioctl_timer_next_device(timer_device *hw, struct snd_timer_id *id);
int ioctl_timer_tread(timer_device *hw, int *enable);
int ioctl_timer_ginfo(timer_device *hw, struct snd_timer_ginfo *ginfo);
int ioctl_timer_gparams(timer_device *hw, struct snd_timer_gparams *gparams);
int ioctl_timer_gstatus(timer_device *hw, struct snd_timer_gstatus *gstatus);
int ioctl_timer_select(timer_device *hw, struct snd_timer_select *select);
int ioctl_timer_info(timer_device *hw, struct snd_timer_info *info);
int ioctl_timer_params(timer_device *hw, struct snd_timer_params *params);
int ioctl_timer_status(timer_device *hw, struct snd_timer_status *status);
int ioctl_timer_start(timer_device *hw);
int ioctl_timer_stop(timer_device *hw);
int ioctl_timer_continue(timer_device *hw);
int ioctl_timer_pause(timer_device *hw);

/** HW dep */

int ioctl_hwdep_pversion(hwdep_device *hw, int *ver);
int ioctl_hwdep_info(hwdep_device *hw, struct snd_hwdep_info *info);
int ioctl_hwdep_dsp_status(hwdep_device *hw,
	struct snd_hwdep_dsp_status *status);

#endif /* ALSA_IOCTL_H */

