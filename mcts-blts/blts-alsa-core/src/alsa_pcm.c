/* alsa_pcm.c -- ALSA PCM IF functionality

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
#include <bits/time.h>
#include <time.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>

#include "alsa_util.h"
#include "alsa_pcm.h"
#include "alsa_ioctl.h"
#include "alsa_async.h"
#include "alsa_timer.h"

const gen_vs pcm_states[] =
{
	{ "SNDRV_PCM_STATE_OPEN", SNDRV_PCM_STATE_OPEN },
	{ "SNDRV_PCM_STATE_SETUP", SNDRV_PCM_STATE_SETUP },
	{ "SNDRV_PCM_STATE_PREPARED", SNDRV_PCM_STATE_PREPARED },
	{ "SNDRV_PCM_STATE_RUNNING", SNDRV_PCM_STATE_RUNNING },
	{ "SNDRV_PCM_STATE_XRUN", SNDRV_PCM_STATE_XRUN },
	{ "SNDRV_PCM_STATE_DRAINING", SNDRV_PCM_STATE_DRAINING },
	{ "SNDRV_PCM_STATE_PAUSED", SNDRV_PCM_STATE_PAUSED },
	{ "SNDRV_PCM_STATE_SUSPENDED", SNDRV_PCM_STATE_SUSPENDED },
	{ "SNDRV_PCM_STATE_DISCONNECTED", SNDRV_PCM_STATE_DISCONNECTED },
	{ NULL, 0 }
};

const gen_vs pcm_hw_params[] = {
	{ "SNDRV_PCM_HW_PARAM_ACCESS", SNDRV_PCM_HW_PARAM_ACCESS },
	{ "SNDRV_PCM_HW_PARAM_FORMAT", SNDRV_PCM_HW_PARAM_FORMAT },
	{ "SNDRV_PCM_HW_PARAM_SUBFORMAT", SNDRV_PCM_HW_PARAM_SUBFORMAT },
	{ "SNDRV_PCM_HW_PARAM_SAMPLE_BITS", SNDRV_PCM_HW_PARAM_SAMPLE_BITS },
	{ "SNDRV_PCM_HW_PARAM_FRAME_BITS", SNDRV_PCM_HW_PARAM_FRAME_BITS },
	{ "SNDRV_PCM_HW_PARAM_CHANNELS", SNDRV_PCM_HW_PARAM_CHANNELS },
	{ "SNDRV_PCM_HW_PARAM_RATE", SNDRV_PCM_HW_PARAM_RATE },
	{ "SNDRV_PCM_HW_PARAM_PERIOD_TIME", SNDRV_PCM_HW_PARAM_PERIOD_TIME },
	{ "SNDRV_PCM_HW_PARAM_PERIOD_SIZE", SNDRV_PCM_HW_PARAM_PERIOD_SIZE },
	{ "SNDRV_PCM_HW_PARAM_PERIOD_BYTES", SNDRV_PCM_HW_PARAM_PERIOD_BYTES },
	{ "SNDRV_PCM_HW_PARAM_PERIODS", SNDRV_PCM_HW_PARAM_PERIODS },
	{ "SNDRV_PCM_HW_PARAM_BUFFER_TIME", SNDRV_PCM_HW_PARAM_BUFFER_TIME },
	{ "SNDRV_PCM_HW_PARAM_BUFFER_SIZE", SNDRV_PCM_HW_PARAM_BUFFER_SIZE },
	{ "SNDRV_PCM_HW_PARAM_BUFFER_BYTES", SNDRV_PCM_HW_PARAM_BUFFER_BYTES },
	{ "SNDRV_PCM_HW_PARAM_TICK_TIME", SNDRV_PCM_HW_PARAM_TICK_TIME },
	{ NULL, 0 }
};

struct async_playback_state {
	double phase;
	int n_buffers;
	unsigned char **buffers;
	snd_pcm_hw_params_t hw_params;
	snd_pcm_sw_params_t sw_params;
	unsigned update_size_frames;
	timer_device *timer;
	int framesize;

	int trigger_fd;

	long out_total;
};

/* These can be used to track opened devices for linking in different pcm
   threads. */
struct pcm_stored_devices_list
{
	int fd;
	int card;
	int device;
	struct pcm_stored_devices_list *next;
};
static pthread_mutex_t stored_devices_lock = PTHREAD_MUTEX_INITIALIZER;
static struct pcm_stored_devices_list *stored_devices;

/* State for thread synchronization */
static unsigned pcm_thread_count;
static unsigned pcm_thread_barrier_ready_count;
static pthread_cond_t pcm_thread_barrier_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pcm_thread_barrier_lock = PTHREAD_MUTEX_INITIALIZER;

/* Incremented when async io ops are triggered */
static sig_atomic_t async_ops_pending = 0;

static void pcm_playback_async_cb_signal(pcm_device *hw,
	struct async_playback_state *state);

static struct async_playback_state *async_playback_state_init(int n_buffers,
	unsigned char **buffers, snd_pcm_hw_params_t hw_params,
	snd_pcm_sw_params_t sw_params);
static void async_playback_state_free(struct async_playback_state *state);
static int async_playback_loop(pcm_device *hw,
	struct async_playback_state *state);


void pcm_thread_barrier()
{
	struct timespec timeout;
	int ret;

	pthread_mutex_lock(&pcm_thread_barrier_lock);

	pcm_thread_barrier_ready_count++;

	BLTS_TRACE("%u at barrier, count = %u\n", pthread_self(),
		pcm_thread_barrier_ready_count);

	if (pcm_thread_count == pcm_thread_barrier_ready_count) {
		pcm_thread_barrier_ready_count = 0;
		pthread_cond_broadcast(&pcm_thread_barrier_cond);
	} else {
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_sec += 10;
 		ret = pthread_cond_timedwait(&pcm_thread_barrier_cond,
			&pcm_thread_barrier_lock, &timeout);
		if (ret == ETIMEDOUT)
			BLTS_ERROR("Warning: Thread barrier timed out\n");
	}
	pthread_mutex_unlock(&pcm_thread_barrier_lock);
}



static char* state_to_str(int state)
{
	int t = 0;
	while(pcm_states[t].str != NULL)
	{
		if(pcm_states[t].value == state)
			return pcm_states[t].str;
		t++;
	}
	return "Unknown";
}

static char* hw_param_to_str(int state)
{
	int t = 0;
	while(pcm_hw_params[t].str != NULL)
	{
		if(pcm_hw_params[t].value == state)
			return pcm_hw_params[t].str;
		t++;
	}
	return "Unknown";
}

static int dump_pcm_status(pcm_device *hw)
{
	int err;
	snd_pcm_sframes_t delay;
	struct snd_pcm_status status;

	err = ioctl_pcm_delay(hw, &delay);
	if(err)
		return err;

	err = ioctl_pcm_status(hw, &status);
	if(err)
		return err;

	BLTS_TRACE("\nState: %s\n", state_to_str(status.state));
	BLTS_TRACE("Appl ptr: %u\n", status.appl_ptr);
	BLTS_TRACE("Hw ptr: %u\n", status.hw_ptr);
	BLTS_TRACE("Avail: %u\n", status.avail);
	BLTS_TRACE("Avail max: %u\n", status.avail_max);
	BLTS_TRACE("Overrange: %u\n", status.overrange);
	BLTS_TRACE("Delay: %d, %d\n", delay, status.delay);
	BLTS_TRACE("Tstamp: %ld.%ld\n", status.tstamp.tv_sec, status.tstamp.tv_nsec);
	BLTS_TRACE("Trigger tstamp: %ld.%ld\n", status.trigger_tstamp.tv_sec,
		status.trigger_tstamp.tv_nsec);
	BLTS_TRACE("Suspended state: %s\n", state_to_str(status.suspended_state));

	return 0;
}

static void dump_mmap_status(pcm_device *hw)
{
	BLTS_TRACE("MMAP status:\n");
	BLTS_TRACE("state: %s\n", state_to_str(hw->mmap_status->state));
	BLTS_TRACE("hw_ptr: %d\n", hw->mmap_status->hw_ptr);
	BLTS_TRACE("tstamp: %ld.%ld\n", hw->mmap_status->tstamp.tv_sec,
		hw->mmap_status->tstamp.tv_nsec);
	BLTS_TRACE("suspended state: %s\n",
		state_to_str(hw->mmap_status->suspended_state));
	BLTS_TRACE("appl_ptr: %d\n", hw->mmap_control->appl_ptr);
	BLTS_TRACE("avail_min: %d\n", hw->mmap_control->avail_min);
}

static void dump_hw_params(snd_pcm_hw_params_t *params)
{
	unsigned int t, i;

	BLTS_TRACE("\nHardware parameters:\n");

	for(t = SNDRV_PCM_HW_PARAM_FIRST_MASK;
		t <= SNDRV_PCM_HW_PARAM_LAST_MASK; t++)
	{
		const snd_mask_t *mask =
			&params->masks[t - SNDRV_PCM_HW_PARAM_FIRST_MASK];

		BLTS_TRACE("%s:\n", hw_param_to_str(t));

		for(i = 0; i <= SNDRV_MASK_MAX; ++i)
		{
			if(mask_test(mask, i))
			{
				switch(t)
				{
					case SNDRV_PCM_HW_PARAM_ACCESS:
						if(i <= SNDRV_PCM_ACCESS_LAST)
							BLTS_TRACE("  %s\n", access_to_str(i));
						break;
					case SNDRV_PCM_HW_PARAM_FORMAT:
						if(i <= SNDRV_PCM_FORMAT_LAST)
							BLTS_TRACE("  %s\n", format_to_str(i));
						break;
					case SNDRV_PCM_HW_PARAM_SUBFORMAT:
						/* Currently ALSA has only one subformat */
						if(i <= SNDRV_PCM_SUBFORMAT_LAST)
						{
							if(i == SNDRV_PCM_SUBFORMAT_STD)
								BLTS_TRACE("  SUBFORMAT_STD\n")
							else
								BLTS_TRACE("  unknown (%d)\n", i)
						}
						break;
					default:
						break;
				}
			}
		}
	}

	for(t = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;
		t <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; t++)
	{
		const snd_interval_t *interval =
			&((params)->intervals[(t) - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
		BLTS_TRACE("%s:\n", hw_param_to_str(t));

		if(interval->min == interval->max ||
			(interval->min + 1 == interval->max && interval->openmax))
		{
			/* single value */
			BLTS_TRACE("  %u\n", interval->min);
		}
		else
		{
			/* value can be between min and max */
			BLTS_TRACE("  %c%u %u%c\n",
				interval->openmin ? '(' : '[',
				interval->min, interval->max,
				interval->openmax ? ')' : ']');
		}
	}

	BLTS_TRACE("\n");
}

static size_t page_align(size_t size)
{
	size_t r;
	long psz = sysconf(_SC_PAGE_SIZE);
	r = size % psz;
	if (r)
		return size + psz - r;
	return size;
}

static int mmap_alloc_chan(pcm_device* hw)
{
	int err, t;
	char* ptr = NULL;
	int size = page_align(hw->buffer_size * calc_framesize(hw));
	struct snd_pcm_channel_info info;

	for(t = 0; t < hw->params.channels; t++)
	{
			err = ioctl_pcm_channel_info(hw, &info, t);
			if(err)
				return err;

			BLTS_TRACE("Map %d bytes for channel %d\n", size, t);
			ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,
				hw->fd, info.offset);
			if(ptr == MAP_FAILED)
			{
				BLTS_ERROR("MMAP failed! (%d)\n", -errno);
				return -errno;
			}
			memset(ptr, 0, size);
			hw->mmap_channels[t].addr = ptr;
	}
	return 0;
}

static int mmap_free_chan(pcm_device* hw)
{
	int err, t;
	int size = page_align(hw->buffer_size * calc_framesize(hw));
	struct snd_pcm_channel_info info;

	for(t = 0; t < hw->params.channels; t++)
	{
			err = ioctl_pcm_channel_info(hw, &info, t);
			if(err)
				return err;

			BLTS_TRACE("Unmap %d bytes for channel %d\n", size, t);
			err = munmap(hw->mmap_channels[t].addr, size);
			if(err < 0)
				BLTS_ERROR("munmap failed (%d)\n", -errno);
			hw->mmap_channels[t].addr = NULL;
	}
	return 0;
}

static int xrun_recovery(pcm_device *handle, int err)
{
	if(err == -EPIPE)
	{
		BLTS_TRACE("overrun\n");
		err = ioctl_pcm_prepare(handle);
		if(err < 0)
		{
			BLTS_ERROR("Can't recover from underrun, prepare failed: %d\n", err);
			return err;
		}
		return 0;
	}
	else if(err == -ESTRPIPE)
	{
		while((err = ioctl_pcm_resume(handle)) == -EAGAIN)
			sleep(1);

		if(err < 0)
		{
			err = ioctl_pcm_prepare(handle);
			if(err < 0)
			{
				BLTS_ERROR("Can't recover from suspend, prepare failed: %d\n",
					err);
				return err;
			}
		}
		return 0;
	}
	return err;
}

static int setup_playback_params(pcm_device *hw,
	snd_pcm_hw_params_t* hw_params,
	snd_pcm_sw_params_t* sw_params)
{
	int err;

	/* Get current params */
	err = ioctl_pcm_refine(hw, hw_params);
	if(err)
		return err;

	dump_hw_params(hw_params);

	if(hw->params.hw_resampling)
		hw_params->flags &= ~SNDRV_PCM_HW_PARAMS_NORESAMPLE;
	else
		hw_params->flags |= SNDRV_PCM_HW_PARAMS_NORESAMPLE;

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_ACCESS,
					 hw->params.access, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set access mode\n");
		return err;
	}

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_CHANNELS,
		hw->params.channels, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set channel count\n");
		return err;
	}

	err = pcm_hw_param_set_minmax(hw_params, SNDRV_PCM_HW_PARAM_RATE,
		hw->params.rate, 0, hw->params.rate + 1, -1);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set sample rate\n");
		return err;
	}

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_FORMAT,
		hw->params.format, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set sample format\n");
		return err;
	}

	if(hw->params.period_size > 0)
	{
		err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE,
			hw->params.period_size, 0);
		if(err < 0)
		{
			BLTS_ERROR("Failed to set sample format %d\n", err);
			return err;
		}
	}

	/* Refine and set the params */
	err = ioctl_pcm_refine(hw, hw_params);
	if(err)
		return err;
	err = ioctl_pcm_hw_params(hw, hw_params);
	if(err)
		return err;

	dump_hw_params(hw_params);

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE,
		&hw->period_size, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get period size %d\n", err);
		return err;
	}

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_BUFFER_SIZE,
		&hw->buffer_size, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get buffer size %d\n", err);
		return err;
	}

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
		&hw->frame_bits, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get frame size %d\n", err);
		return err;
	}

	/* Setup SW params */
	sw_params->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	sw_params->period_step = 1;
	sw_params->sleep_min = 0;
	sw_params->avail_min = hw->period_size;
	sw_params->xfer_align = 1;
	hw->start_threshold = sw_params->start_threshold =
		(hw->buffer_size / hw->period_size) * hw->period_size;
	sw_params->stop_threshold = hw->buffer_size;
	sw_params->silence_threshold = 0;
	sw_params->silence_size = 0;
	sw_params->boundary = hw->buffer_size;
	while(sw_params->boundary * 2 <= LONG_MAX - hw->buffer_size)
		sw_params->boundary *= 2;

	err = ioctl_pcm_sw_params(hw, sw_params);
	if(err)
		return err;

	return 0;
}

static int setup_recording_params(pcm_device *hw,
	snd_pcm_hw_params_t* hw_params)
{
	int err;

	/* Get current params */
	err = ioctl_pcm_refine(hw, hw_params);
	if(err)
		return err;

	dump_hw_params(hw_params);

	if(hw->params.hw_resampling)
		hw_params->flags &= ~SNDRV_PCM_HW_PARAMS_NORESAMPLE;
	else
		hw_params->flags |= SNDRV_PCM_HW_PARAMS_NORESAMPLE;

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_ACCESS,
					 hw->params.access, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set access mode\n");
		return err;
	}

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_CHANNELS,
		hw->params.channels, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set channel count\n");
		return err;
	}

	err = pcm_hw_param_set_minmax(hw_params, SNDRV_PCM_HW_PARAM_RATE,
		hw->params.rate, 0, hw->params.rate + 1, -1);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set sample rate\n");
		return err;
	}

	err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_FORMAT,
		hw->params.format, 0);
	if(err < 0)
	{
		BLTS_ERROR("Failed to set sample format\n");
		return err;
	}

	if(hw->params.period_size > 0)
	{
		err = pcm_hw_param_set(hw_params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE,
			hw->params.period_size, 0);
		if(err < 0)
		{
			BLTS_ERROR("Failed to set sample format %d\n", err);
			return err;
		}
	}

	/* Refine and set the params */
	err = ioctl_pcm_refine(hw, hw_params);
	if(err)
		return err;
	err = ioctl_pcm_hw_params(hw, hw_params);
	if(err)
		return err;

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE,
		&hw->period_size, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get period size %d\n", err);
		return err;
	}

	dump_hw_params(hw_params);

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_BUFFER_SIZE,
		&hw->buffer_size, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get buffer size %d\n", err);
		return err;
	}

	err = pcm_hw_param_get(hw_params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
		&hw->frame_bits, 0);
	if(err)
	{
		BLTS_ERROR("Failed to get frame size %d\n", err);
		return err;
	}

	hw->start_threshold = (hw->buffer_size / hw->period_size) * hw->period_size;

	return 0;
}

static snd_pcm_state_t read_hw_state(pcm_device *hw)
{
	int err;
	if(hw->sync_ptr)
	{
		err = ioctl_pcm_sync_ptr(hw, 0);
		if (err < 0)
			return err;
	}
	return (snd_pcm_state_t)hw->mmap_status->state;
}

static int play_buffer_avail(pcm_device *hw)
{
	int avail, err;

	if(hw->sync_ptr)
	{
		err = ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
		if (err < 0)
			return err;
		avail = hw->sync_ptr->s.status.hw_ptr + hw->buffer_size -
			hw->sync_ptr->c.control.appl_ptr;
	}
	else
	{
		avail = hw->mmap_status->hw_ptr + hw->buffer_size -
			hw->mmap_control->appl_ptr;
	}

	if(avail < 0)
		avail += hw->buffer_size;
	else if(avail >= (int)hw->buffer_size)
		avail -= hw->buffer_size;

	return avail;
}

static int rec_buffer_avail(pcm_device* hw)
{
	int avail, err;

	if(hw->sync_ptr)
	{
		err = ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
		if (err < 0)
			return err;
		avail = hw->sync_ptr->s.status.hw_ptr - hw->sync_ptr->c.control.appl_ptr;
	}
	else
	{
		avail = hw->mmap_status->hw_ptr - hw->mmap_control->appl_ptr;
	}
	if (avail < 0)
		avail += hw->buffer_size;
	return avail;
}

static int poll_pcm_play_no_timer(pcm_device *hw)
{
	if(hw->sync_ptr)
	{
		while(play_buffer_avail(hw) < (int)hw->sync_ptr->c.control.avail_min)
			usleep(10);
	}
	else
	{
		while(play_buffer_avail(hw) < (int)hw->mmap_control->avail_min)
			usleep(10);
	}
	return 0;
}

static int poll_pcm_rec_no_timer(pcm_device *hw)
{
	if(hw->sync_ptr)
	{
		while(rec_buffer_avail(hw) < (int)hw->sync_ptr->c.control.avail_min)
			usleep(10);
	}
	else
	{
		while(rec_buffer_avail(hw) < (int)hw->mmap_control->avail_min)
			usleep(10);
	}
	return 0;
}

static snd_pcm_sframes_t pcm_mmap_writei(pcm_device *hw, const char *buffer,
	snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t frames;
	snd_pcm_sframes_t avail;
	snd_pcm_sframes_t offset = 0;
	snd_pcm_uframes_t xfer = 0;
	snd_pcm_sframes_t err = 0;
	snd_pcm_state_t state;
	snd_pcm_uframes_t c_off;
	int t;
	int framesize = calc_framesize(hw);

	/* appl_ptr must be ahead of hw_ptr, otherwise we'll get underrun */
	if(hw->sync_ptr)
	{
		err = ioctl_pcm_sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
		if (err < 0)
			return err;
		c_off = hw->sync_ptr->c.control.appl_ptr;
		hw->sync_ptr->c.control.appl_ptr += size;
	}
	else
	{
		c_off = hw->mmap_control->appl_ptr;
		hw->mmap_control->appl_ptr += size;
	}

	while(size > 0)
	{
		state = read_hw_state(hw);
		if(state == SNDRV_PCM_STATE_RUNNING)
		{
			err = ioctl_pcm_hwsync(hw);
			if(err < 0)
				break;
		}

		avail = play_buffer_avail(hw);
		if(avail < 0)
		{
			err = avail;
			break;
		}

		/* wait until we enough have free space in buffers */
		if(hw->sync_ptr)
		{
			if((state == SNDRV_PCM_STATE_RUNNING) &&
				(snd_pcm_uframes_t)avail < hw->period_size &&
				size > (snd_pcm_uframes_t)avail)
			{
				err = poll_pcm_play_no_timer(hw);
				if (err < 0)
					break;
				continue;
			}
		}
		else
		{
			if((state == SNDRV_PCM_STATE_RUNNING) &&
				(snd_pcm_uframes_t)avail < hw->mmap_control->avail_min &&
				size > (snd_pcm_uframes_t)avail)
			{
				err = poll_pcm_play_no_timer(hw);
				if (err < 0)
					break;
				continue;
			}
		}

		frames = size;
		if(frames > (snd_pcm_uframes_t) avail)
			frames = avail;
		if(!frames)
			break;

		for(t = 0; t < hw->params.channels; t++)
		{
			memcpy(&((unsigned char*)hw->mmap_channels[t].addr)
				[c_off%hw->buffer_size * framesize],
				&buffer[offset * framesize],
				frames * framesize);
		}
		c_off += frames;

		if(state == SNDRV_PCM_STATE_PREPARED)
		{
			/* Start playback when we have filled the buffer(s) with at least
			 * hw->start_threshold frames */
			snd_pcm_sframes_t hw_avail = hw->buffer_size - avail;
			hw_avail += frames;
			state = read_hw_state(hw);
			if(state == SNDRV_PCM_STATE_PREPARED &&
				hw_avail >= (snd_pcm_sframes_t) hw->start_threshold)
			{
				BLTS_TRACE("start!\n");
				err = ioctl_pcm_start(hw);
				if (err < 0)
					break;
			}
		}
		offset += frames;
		size -= frames;
		xfer += frames;
	}

	return xfer > 0 ? (snd_pcm_sframes_t) xfer : err;
}

static snd_pcm_sframes_t pcm_mmap_readi(pcm_device *hw, const char *buffer,
	snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t frames;
	snd_pcm_sframes_t avail;
	snd_pcm_sframes_t offset = 0;
	snd_pcm_uframes_t xfer = 0;
	snd_pcm_sframes_t err = 0;
	snd_pcm_state_t state;

	state = read_hw_state(hw);
	switch (state)
	{
		case SNDRV_PCM_STATE_PREPARED:
			err = ioctl_pcm_start(hw);
			if (err < 0)
				goto cleanup;
			break;
		case SNDRV_PCM_STATE_DRAINING:
		case SNDRV_PCM_STATE_RUNNING:
			break;
		case SNDRV_PCM_STATE_XRUN:
			return -EPIPE;
		case SNDRV_PCM_STATE_SUSPENDED:
			return -ESTRPIPE;
		case SNDRV_PCM_STATE_DISCONNECTED:
			return -ENODEV;
		default:
			return -EBADFD;
	}

	while(size > 0)
	{
		state = read_hw_state(hw);
		if(state == SNDRV_PCM_STATE_RUNNING)
		{
			err = ioctl_pcm_hwsync(hw);
			if(err < 0)
				goto cleanup;
		}

		avail = rec_buffer_avail(hw);
		if(avail < 0)
		{
			err = avail;
			goto cleanup;
		}

		if(hw->sync_ptr)
		{
			if((snd_pcm_uframes_t)avail < hw->period_size &&
				size > (snd_pcm_uframes_t)avail)
			{
				err = poll_pcm_rec_no_timer(hw);
				if (err < 0)
					break;
				continue;
			}
		}
		else
		{
			if ((snd_pcm_uframes_t)avail < hw->mmap_control->avail_min &&
				size > (snd_pcm_uframes_t)avail)
			{
				err = poll_pcm_rec_no_timer(hw);
				if (err < 0)
					break;
				continue;
			}
		}

		frames = size;
		if(frames > (snd_pcm_uframes_t) avail)
			frames = avail;
		if(!frames)
			break;

		/* TODO: Read the data */
		(void)(buffer);

		if(hw->sync_ptr)
			hw->sync_ptr->c.control.appl_ptr += frames;
		else
			hw->mmap_control->appl_ptr += frames;

		offset += frames;
		size -= frames;
		xfer += frames;
	}
cleanup:
	return xfer > 0 ? (snd_pcm_sframes_t) xfer : err;
}

int pcm_play_sine(pcm_device *hw)
{
	double phase = 0;
	unsigned char *ptr;
	int err = -1, clean_err, cur_time, prev_time = 0;
	int cptr, t, bufcount, framesize;
	unsigned char *buffers[MAX_CHANNELS];
	snd_pcm_hw_params_t hw_params;
	snd_pcm_sw_params_t sw_params;
	struct async_playback_state *async_state = 0;
	unsigned pcm_link_ok = 0;
	unsigned long loop_count = 0;

	memset(&sw_params, 0, sizeof(sw_params));
	memset(buffers, 0, sizeof(buffers));

	pcm_hw_params_any(&hw_params);

	err = setup_playback_params(hw, &hw_params, &sw_params);
	if(err)
	{
		BLTS_ERROR("Failed to setup hw/sw params %d\n", err);
		goto cleanup;
	}

	hw->configured = 1;

	BLTS_TRACE("Period size: %u, buffer size: %u\n", hw->period_size,
		hw->buffer_size);

	/* Allocate separate buffers for each channel in case of non-interlevead
	 * playback */
	if(hw->params.access == SNDRV_PCM_ACCESS_RW_NONINTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
		bufcount = hw->params.channels;
	else
		bufcount = 1;

	framesize = calc_framesize(hw);

	for(t = 0; t < bufcount; t++)
	{
		buffers[t] = malloc(hw->period_size * framesize);
		if(!buffers[t])
		{
			BLTS_LOGGED_PERROR("Failed to allocate playback buffer");
			goto cleanup;
		}
	}

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		err = mmap_alloc_chan(hw);
		if(err)
			goto cleanup;
		dump_mmap_status(hw);
	}

	err = ioctl_pcm_prepare(hw);
	if(err)
		goto cleanup;

	pcm_thread_barrier();

	if (hw->params.async)
	{
		async_state = async_playback_state_init(bufcount, buffers, hw_params,
			sw_params);
		if (!async_state)
		{
			BLTS_ERROR("Async init failed.\n");
			err = -ENOMEM;
			goto cleanup;
		}
	}

	if (hw->params.async)
	{
		err = async_playback_loop(hw, async_state);
		if (err) {
			BLTS_ERROR("Asynchronous playback failed.\n");
			goto cleanup;
		}
	}
	else
	while((cur_time = (int)timing_elapsed()) < hw->params.duration)
	{
		/* FIXME: Maybe play silence instead until every thread is
		   really ready? In app/lib code, we'd do proper buffer sync
		   here anyway. */
		if (hw->params.link_card >= 0
			&& hw->params.link_device >= 0
			&& !pcm_link_ok
			&& loop_count > hw->start_threshold)
		{
			err = pcm_link_device(hw);
			BLTS_DEBUG("PCM link %s.\n", err ? "failed" : "ok");
			if (!err)
				pcm_link_ok++;
		}

		if(hw->params.access == SNDRV_PCM_ACCESS_RW_NONINTERLEAVED ||
			hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
		{
			err = generate_sine(buffers[0], hw->period_size, &phase,
				1, hw->params.rate, hw->params.freq, hw->params.format,
				hw->frame_bits, hw->params.volume);

			if(!err)
			{
				for(t = 1; t < bufcount; t++)
					memcpy(buffers[t], buffers[0], hw->period_size * framesize);
			}
		}
		else
		{
			err = generate_sine(buffers[0], hw->period_size, &phase,
				hw->params.channels, hw->params.rate, hw->params.freq,
				hw->params.format, hw->frame_bits, hw->params.volume);
		}

		if(err)
		{
			BLTS_ERROR("Failed to generate sine wave\n");
			goto cleanup;
		}

		ptr = buffers[0];
		cptr = hw->period_size;
		while(cptr > 0)
		{
			if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED)
				err = pcm_mmap_writei(hw, (char*)ptr, cptr);
			else if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
				err = pcm_mmap_writei(hw, (char*)ptr, cptr); /* TODO: Real implementation */
			else if(hw->params.access == SNDRV_PCM_ACCESS_RW_NONINTERLEAVED)
				err = ioctl_pcm_writen(hw, (void**)buffers, cptr);
			else /* SNDRV_PCM_ACCESS_RW_INTERLEAVED */
				err = ioctl_pcm_writei(hw, ptr, cptr);

			if(err == -EAGAIN)
				continue;

			if(err < 0)
			{
				err = xrun_recovery(hw, err);
				if(err)
				{
					BLTS_ERROR("Write error: %d\n", err);
					goto cleanup;
				}
				/* Recovered, skip one period */
				break;
			}
			ptr += err * framesize;
			cptr -= err;
		}
		loop_count++;
		/* print out pcm status on every second when using verbose logging */
		if(cur_time != prev_time)
		{
			err = dump_pcm_status(hw);
			if(err)
				goto cleanup;
			prev_time = cur_time;
		}
	}

	if (hw->params.link_card >= 0 && hw->params.link_card >= 0)
	{
		if (!pcm_link_ok && !err)
			err = -1;
		clean_err = pcm_unlink_device(hw);
		BLTS_DEBUG("PCM unlink %s.\n", clean_err ? "failed" : "ok");
		if(!err)
			err = clean_err;
		pcm_link_ok = 0;
	}
	pcm_thread_barrier();

	ioctl_pcm_drop(hw);

	err = 0;

cleanup:

	if(pcm_link_ok)
		pcm_unlink_device(hw);

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		dump_mmap_status(hw);
		mmap_free_chan(hw);
	}

	if(async_state)
		async_playback_state_free(async_state);

	t = 0;
	while(buffers[t])
		free(buffers[t++]);

	return err;
}

int pcm_record(pcm_device *hw)
{
	int err = -1, clean_err, cur_time, prev_time = 0;;
	unsigned int rec_frames = 0;
	snd_pcm_hw_params_t hw_params;
	int t, bufcount, framesize;
	unsigned char *buffers[MAX_CHANNELS];
	unsigned pcm_link_ok = 0;

	memset(buffers, 0, sizeof(buffers));

	pcm_hw_params_any(&hw_params);

	err = setup_recording_params(hw, &hw_params);
	if(err)
	{
		BLTS_ERROR("Failed to setup hw/sw params %d\n", err);
		goto cleanup;
	}

	hw->configured = 1;

	BLTS_TRACE("Period size: %u, buffer size: %u\n", hw->period_size,
		hw->buffer_size);

	/* Allocate separate buffers for each channel in case of non-interlevead
	 * recording */
	if(hw->params.access == SNDRV_PCM_ACCESS_RW_NONINTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
		bufcount = hw->params.channels;
	else
		bufcount = 1;

	framesize = calc_framesize(hw);

	for(t = 0; t < bufcount; t++)
	{
		buffers[t] = malloc(hw->period_size * framesize);
		if(!buffers[t])
		{
			BLTS_LOGGED_PERROR("Failed to allocate recording buffer");
			goto cleanup;
		}
	}

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		err = mmap_alloc_chan(hw);
		if(err)
			goto cleanup;
		dump_mmap_status(hw);
	}

	err = ioctl_pcm_prepare(hw);
	if(err)
		goto cleanup;

	pcm_thread_barrier();

	while((cur_time = (int)timing_elapsed()) < hw->params.duration)
	{
		if (hw->params.link_card >= 0
			&& hw->params.link_device >= 0
			&& !pcm_link_ok
			&& rec_frames > hw->start_threshold)
		{
			err = pcm_link_device(hw);
			BLTS_DEBUG("PCM link %s.\n", err ? "failed" : "ok");
			if (!err)
				pcm_link_ok++;
		}

		if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED)
			err = pcm_mmap_readi(hw, (char*)buffers[0], hw->period_size);
		else if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
			err = pcm_mmap_readi(hw, (char*)buffers[0], hw->period_size); /* TODO: Real implementation */
		else if(hw->params.access == SNDRV_PCM_ACCESS_RW_NONINTERLEAVED)
			err = ioctl_pcm_readn(hw, (void**)buffers, hw->period_size);
		else /* SNDRV_PCM_ACCESS_RW_INTERLEAVED */
			err = ioctl_pcm_readi(hw, buffers[0], hw->period_size);

		if(err == -EPIPE)
		{
			err = ioctl_pcm_prepare(hw);
			if(err < 0)
			{
				BLTS_ERROR("Can't recover from overrun, prepare failed: %d\n",
					err);
				goto cleanup;
			}
		}
		else if(err < 0)
		{
			return -1;
		}
		else if(err != (int)hw->period_size)
		{
			BLTS_TRACE("Short read, read %d frames\n", err);
		}
		rec_frames += err;

		/* print out pcm status on every second when using verbose logging */
		if(cur_time != prev_time)
		{
			err = dump_pcm_status(hw);
			if(err)
				goto cleanup;
			prev_time = cur_time;
		}
	}

	if (hw->params.link_card >= 0 && hw->params.link_card >= 0)
	{
		if (!pcm_link_ok && !err)
			err = -1;
		clean_err = pcm_unlink_device(hw);
		BLTS_DEBUG("PCM unlink %s.\n", clean_err ? "failed" : "ok");
		if(!err)
			err = clean_err;
		pcm_link_ok = 0;
	}
	pcm_thread_barrier();

	ioctl_pcm_drop(hw);

	BLTS_DEBUG("Recorded %u frames (%u bytes)\n", rec_frames,
		framesize * rec_frames);

	err = 0;

cleanup:
	if (pcm_link_ok)
		pcm_unlink_device(hw);

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		dump_mmap_status(hw);
		mmap_free_chan(hw);
	}

	t = 0;
	while(buffers[t])
		free(buffers[t++]);

	return err;
}

pcm_device* pcm_open(int card, int device, int dir)
{
	char filename[MAX_DEV_NAME_LEN];
	pcm_device *hw = NULL;

	if(card < 0 || card >= MAX_CARDS)
	{
		BLTS_ERROR("Invalid card index %d\n", card);
		return NULL;
	}

	hw = calloc(1, sizeof(pcm_device));
	if(!hw)
	{
		BLTS_LOGGED_PERROR("Failed to allocate pcm_device structure");
		return NULL;
	}

	sprintf(filename, "/dev/snd/pcmC%dD%d%s", card, device,
		(dir == STREAM_DIR_OUT)?"p":"c");

	BLTS_TRACE("Opening device %s\n", filename);
	hw->fd = open(filename, O_RDWR);
	if(!hw->fd)
	{
		BLTS_ERROR("Failed to open device '%s'\n", filename);
		goto error_exit;
	}

	if(ioctl_pcm_pversion(hw, &hw->protocol))
		goto error_exit;

	BLTS_TRACE("Protocol version %d.%d.%d\n",
		SNDRV_PROTOCOL_MAJOR(hw->protocol),
		SNDRV_PROTOCOL_MINOR(hw->protocol),
		SNDRV_PROTOCOL_MICRO(hw->protocol));

	if(hw->protocol < SNDRV_PROTOCOL_VERSION(2, 0, 9))
	{
		BLTS_ERROR("Protocol version < 2.0.9 not supported\n");
		goto error_exit;
	}

	hw->card = card;
	hw->device = device;
	hw->params.format = -1;

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

static int mmap_init(pcm_device* hw)
{
	void *ptr;
	int err = 0;

	ptr = MAP_FAILED;
	if(!hw->sync_ptr_ioctl)
	{
		ptr = mmap(NULL, page_align(sizeof(struct snd_pcm_mmap_status)),
			PROT_READ, MAP_FILE|MAP_SHARED,
			hw->fd, SNDRV_PCM_MMAP_OFFSET_STATUS);
	}

	if(ptr == MAP_FAILED || !ptr)
	{
		BLTS_TRACE("status mmap failed\n");
		hw->sync_ptr = calloc(1, sizeof(struct snd_pcm_sync_ptr));
		if (hw->sync_ptr == NULL)
			return -ENOMEM;
		hw->sync_ptr->c.control.appl_ptr = 0;
		hw->sync_ptr->c.control.avail_min = 1;
		err = ioctl_pcm_sync_ptr(hw, 0);
		if (err < 0)
			return err;
		hw->mmap_status = &hw->sync_ptr->s.status;
		hw->mmap_control = &hw->sync_ptr->c.control;
		hw->sync_ptr_ioctl = 1;
	}
	else
	{
		BLTS_TRACE("successfully mapped status\n");
		hw->mmap_status = ptr;
	}

	if(!hw->sync_ptr)
	{
		ptr = mmap(NULL, page_align(sizeof(struct snd_pcm_mmap_control)),
			PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,
			hw->fd, SNDRV_PCM_MMAP_OFFSET_CONTROL);
		if(ptr == MAP_FAILED || !ptr)
		{
			BLTS_ERROR("control mmap failed\n");
			return err;
		}
		BLTS_TRACE("successfully mapped control\n");
		hw->mmap_control = ptr;
	}
	else
	{
		hw->mmap_control->avail_min = 1;
	}

	return 0;
}


static int mmap_uninit(pcm_device* hw)
{
	if(!hw)
		return -EINVAL;

	if(hw->sync_ptr_ioctl)
	{
		free(hw->sync_ptr);
		hw->sync_ptr = NULL;
	}
	else
	{
		if(hw->mmap_status && munmap((void*)hw->mmap_status,
			page_align(sizeof(*hw->mmap_status))) < 0)
		{
			BLTS_ERROR("status munmap failed\n");
			return -1;
		}
	}

	if(hw->sync_ptr_ioctl)
	{
		free(hw->sync_ptr);
		hw->sync_ptr = NULL;
	}
	else
	{
		if(hw->mmap_control && munmap(hw->mmap_control,
			page_align(sizeof(*hw->mmap_control))) < 0)
		{
			BLTS_ERROR("control munmap failed\n");
			return -1;
		}
	}

	return 0;
}

int pcm_init(pcm_device* hw)
{
	int err = 0;
#if defined(CLOCK_MONOTONIC) && defined(SNDRV_PCM_IOCTL_TTSTAMP)
	struct timespec timespec;

	if(clock_gettime(CLOCK_MONOTONIC, &timespec) == 0)
	{
		int mode = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC;
		err = ioctl_pcm_ttstamp(hw, &mode);
		if(err)
			return err;
		hw->monotonic = 1;
	}
#endif

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		err = mmap_init(hw);
	}

	return err;
}

int pcm_close(pcm_device* hw)
{
	int ret = 0;

	if(!hw)
		return -EINVAL;

	if(hw->params.access == SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ||
		hw->params.access == SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED)
	{
		ret = mmap_uninit(hw);
	}

	if(hw->configured)
		ioctl_pcm_hw_free(hw);

	if(hw->fd)
		close(hw->fd);

	free(hw);

	return ret;
}

int pcm_print_info(pcm_device* hw)
{
	int err = 0;
	struct snd_pcm_info info;

	err = ioctl_pcm_info(hw, &info);
	if(err)
		return err;

	BLTS_DEBUG("PCM '%s':\n", info.name);
	BLTS_DEBUG("\tcard: %d\n", info.card);
	BLTS_DEBUG("\tdevice: %d\n", info.device);
	BLTS_DEBUG("\tid: %s\n", info.id);
	BLTS_DEBUG("\tsubname: %s\n", info.subname);
	BLTS_DEBUG("\tsubdevice count: %d\n", info.subdevices_count);

	return 0;
}


static struct async_playback_state *async_playback_state_init(int n_buffers,
	unsigned char **buffers, snd_pcm_hw_params_t hw_params,
	snd_pcm_sw_params_t sw_params)
{
	struct async_playback_state *state;

	state = malloc(sizeof *state);
	if (!state)
		return 0;
	memset(state, 0, sizeof *state);

	state->n_buffers = n_buffers;
	state->buffers = buffers;
	state->hw_params = hw_params;
	state->sw_params = sw_params;

	return state;
}

static void async_playback_state_free(struct async_playback_state *state)
{
	if (!state)
		return;

	free(state);
}

static int pcm_async_playback(pcm_device *hw, struct async_playback_state *state)
{
	int t, ret = 0, remain;
	unsigned char *p;

	remain = state->update_size_frames;

	for (t = 0; t < state->n_buffers; ++t) {
		ret = generate_sine(state->buffers[t], remain, &(state->phase),
			hw->params.channels, hw->params.rate, hw->params.freq,
			hw->params.format, hw->frame_bits, hw->params.volume);
	}
	if (ret) {
		BLTS_ERROR("Failed generating sine wave.\n");
		return ret;
	}

	p = state->buffers[0];
	while (remain > 0) {
		ret = ioctl_pcm_writei(hw, p, remain);
		if (ret == -EAGAIN)
			continue;
		if (ret < 0) {
			ret = xrun_recovery(hw, ret);
			if (ret) {
				BLTS_ERROR("Write error.\n");
				return ret;
			}
			ret = -EPIPE;
			break;
		}
		p += ret * state->framesize;
		remain -= ret;
		state->out_total += ret;
	}
	if (ret > 0)
		ret = 0;

	return ret;
}

static void pcm_playback_async_cb_signal(__attribute__((unused)) pcm_device *hw,
	__attribute__((unused)) struct async_playback_state *state)
{
	async_ops_pending++;
}

static int async_playback_init(pcm_device *hw, struct async_playback_state *state)
{
	int ret;
	long timer_period_ns;

	state->update_size_frames = hw->period_size;

	switch (hw->params.async) {
	case TEST_ASYNC_MODE_SIGNAL:
		state->trigger_fd = hw->fd;
		break;
 	case TEST_ASYNC_MODE_TIMER:
		state->timer = timer_open();
		if (!state->timer) {
			BLTS_ERROR("Failed to open timer.\n");
			ret = -1;
			goto cleanup;
		}

		ret = timer_select_for_device(state->timer, hw->card, hw->device);
		if (ret) {
			BLTS_ERROR("Failed reading timer info.\n");
			goto cleanup;
		}

		timer_period_ns = 1000000000LL / ((long)(hw->params.rate) * hw->params.channels)
			* hw->period_size;
		ret = timer_set_period(state->timer, timer_period_ns);
		if (ret) {
			BLTS_ERROR("Failed setting timer period.\n");
			goto cleanup;
		}

		state->trigger_fd = state->timer->fd;

 		break;
	default:
		BLTS_ERROR("Unknown async mode (this is a bug).\n");
		ret = -1;
		goto cleanup;
		break;
	}

	ret = async_handler_add((async_callback) pcm_playback_async_cb_signal,
		hw, state->trigger_fd, state);
	if (ret) {
		BLTS_ERROR("Callback add failed.\n");
	}

cleanup:
	return ret;
}

static int async_playback_loop(pcm_device *hw, struct async_playback_state *state)
{
	int ret, err, n_xruns = 0;
	long recovery_point, expected_out;
	struct timespec sleep_interval;
	double t;

	if (hw->params.access != SNDRV_PCM_ACCESS_RW_INTERLEAVED) {
		BLTS_ERROR("Unsupported access type.\n");
		ret = -1;
		goto cleanup;
	}

	sleep_interval.tv_sec = 0;
	sleep_interval.tv_nsec = 100L * 1000 * 1000; /* 100 ms */

	state->framesize = calc_framesize(hw);

	ret = async_playback_init(hw, state);
	if (ret)
		goto cleanup;

	expected_out = hw->params.duration * hw->params.rate;

	while(state->out_total < (long)hw->start_threshold) {
		ret = pcm_async_playback(hw, state);
		if (ret) {
			BLTS_ERROR("Error during initial buffering\n");
			goto cleanup;
		}
	}

	if (state->timer) {
		ret = ioctl_timer_start(state->timer);
		if (ret) {
			BLTS_ERROR("Timer start failed.\n");
			goto cleanup;
		}
	}

	ret = async_io_enable(state->trigger_fd);
	if (ret) {
		BLTS_ERROR("Async playback start failed.\n");
		goto cleanup;
	}
	while (1) {
		t = timing_elapsed();
		while (async_ops_pending) {
			ret = pcm_async_playback(hw, state);
			if (ret)
				break;
			async_ops_pending--;
		}
		if (ret == -EPIPE) {
			n_xruns++;
			recovery_point = state->out_total +(long)hw->start_threshold;
			state->phase = 0.0;
			while(state->out_total < recovery_point) {
				ret = pcm_async_playback(hw, state);
				if (ret) {
					BLTS_ERROR("Recovery failure while refilling buffer.\n");
					goto cleanup;
				}
			}
			async_ops_pending = 0;
		} else if (ret) {
			BLTS_ERROR("Error during playback.\n");
			goto cleanup;
		}
		if (t > hw->params.duration)
			break;
		/* This is interrupted on signal receipt (see eg. man 7 signal),
		   but kept short since we only check for timeout just above. */
		ret = nanosleep(&sleep_interval, 0);
		if (ret && errno != EINTR) {
			/* should never happen, but.. */
			BLTS_ERROR("Error during sleep\n");
			goto cleanup;
		}
	}

	if (n_xruns)
		BLTS_WARNING("Warning: over/underruns during test.\n");

	/* This is basically a heuristic since we don't know if enough
	   callbacks have been signalled for a more exact test to make sense. */
	if (((state->out_total) / 1000) < (expected_out / 1000)-1) {
		BLTS_ERROR("Wrote %lu * 1000 frames (expected > %lu * 1000 frames), test fails.\n",
			(state->out_total) / 1000, (expected_out / 1000)-1);
		ret = ret ? ret : -1;
	}

cleanup:
	if (state->timer) {
		err = ioctl_timer_stop(state->timer);
		if (err) {
			BLTS_ERROR("Timer stop failed.\n");
			ret = ret ? ret : err;
		}
	}

	err = async_io_disable(state->trigger_fd);
	if (err) {
		BLTS_ERROR("Async disable on device failed.\n");
		ret = ret ? ret : err;
	}

	if (state->timer) {
		timer_print_status_trace(state->timer);
		err = timer_close(state->timer);
		state->timer = 0;
		if (err) {
			BLTS_ERROR("Timer close failed.\n");
			ret = ret ? ret : err;
		}
	}

	async_handler_clear();

	return ret;
}

/* Set up the device list */
void pcm_stored_devices_init()
{
	if (stored_devices)
		pcm_stored_devices_free();
}

/* Store card/device/used fd data for use by others. */
void pcm_device_fd_store(int card, int device, int fd)
{
	struct pcm_stored_devices_list *p;

	p = malloc(sizeof *p);
	p->card = card;
	p->device = device;
	p->fd = fd;

	pthread_mutex_lock(&stored_devices_lock);

	p->next = stored_devices;
	stored_devices = p;

	pthread_mutex_unlock(&stored_devices_lock);
}

/* Get stored fd of given card/device (or <0 if not available). Retries for
   timeout msec. */
int pcm_get_device_fd(int card, int device, int timeout)
{
	struct pcm_stored_devices_list *p;
	int fd = -1;
	double t_end;

	t_end = timing_elapsed() + 1.E-3 * timeout;
	while (timing_elapsed() < t_end) {
		pthread_mutex_lock(&stored_devices_lock);

		p = stored_devices;
		while (p) {
			if (p->card == card && p->device == device) {
				fd = p->fd;
				break;
			}
			p = p->next;
		}

		pthread_mutex_unlock(&stored_devices_lock);
		if (fd >= 0) {
			break;
		}

		usleep(10000);
	}
	return fd;
}

/* Free stored devices list */
void pcm_stored_devices_free()
{
	struct pcm_stored_devices_list *p;

	pthread_mutex_lock(&stored_devices_lock);

	while (stored_devices) {
		p = stored_devices;
		stored_devices = stored_devices->next;
		free(p);
	}

	pthread_mutex_unlock(&stored_devices_lock);
}

/* Find device to link to, and link. */
int pcm_link_device(pcm_device *hw)
{
	int err, fd;
	fd = pcm_get_device_fd(hw->params.link_card, hw->params.link_device, 2000);

	if(fd < 0) {
		BLTS_ERROR("Cannot find device (%d:%d) to link to.\n",
			hw->params.link_card, hw->params.link_device);
		return -ENOENT;
	}

	BLTS_TRACE("Linking (%d:%d) with (%d:%d) (fd %d).\n", hw->card, hw->device,
		hw->params.link_card, hw->params.link_device, fd);

	err = ioctl_pcm_link(hw, fd);
	if (err)
		BLTS_ERROR("Could not link device (%d:%d) to (%d:%d)\n", hw->card,
			hw->device, hw->params.link_card, hw->params.link_device);
	return err;
}

/* Unlink device */
int pcm_unlink_device(pcm_device *hw)
{
	return ioctl_pcm_unlink(hw);
}

/* Called before threads started. Thread sync depends on this. */
void pcm_thread_init(unsigned n_pcm_threads)
{
	pcm_thread_count = n_pcm_threads;
}
