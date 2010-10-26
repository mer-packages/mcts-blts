/* alsa_util.h -- Various helper functions and definitions

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

#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H

#include <linux/types.h>
#include <sys/ioctl.h>
#include <sound/asound.h>
#include <blts_log.h>
#include <blts_timing.h>
#include "alsa_config.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

#define MAX_CARDS 32
#define MAX_PCM_DEVICES 64
#define MAX_DEV_NAME_LEN 64
#define MAX_CHANNELS 10

enum {
	TEST_ASYNC_MODE_NONE = 0,
	TEST_ASYNC_MODE_SIGNAL,
	TEST_ASYNC_MODE_TIMER,
};

typedef struct snd_pcm_hw_params snd_pcm_hw_params_t;
typedef struct snd_pcm_sw_params snd_pcm_sw_params_t;
typedef struct snd_mask snd_mask_t;
typedef struct snd_interval snd_interval_t;
typedef struct snd_ctl_card_info snd_ctl_card_info_t;

typedef struct {
	int card;
	int device;
	int dir;
	int duration;
	int freq;
	snd_pcm_format_t format;
	int rate;
	int channels;
	int hw_resampling;
	int access;
	int async;
	int link_card, link_device;
	int period_size;
	int volume;
} pcm_params;

typedef struct {
	void *addr;
} pcm_mmap_chan;

typedef struct {
	int fd;
	int protocol;
	int card, device;
	int sync_ptr_ioctl;
	struct snd_pcm_sync_ptr *sync_ptr;
	volatile struct snd_pcm_mmap_status *mmap_status;
	struct snd_pcm_mmap_control *mmap_control;
	unsigned int period_size;
	unsigned int buffer_size;
	unsigned int frame_bits;
	unsigned int start_threshold;
	int configured;
	int monotonic;
	pcm_mmap_chan mmap_channels[MAX_CHANNELS];

	pcm_params params;
} pcm_device;

typedef struct {
	int fd;
	int protocol;
	int card;
} ctl_device;

typedef struct {
	int fd;
	int protocol;
} timer_device;

typedef struct {
	int fd;
	int protocol;
	int card, device;
} hwdep_device;

typedef struct {
	char* str;
	int value;
} gen_vs;

struct slist {
	struct slist *next;
	void *item;
};

int generate_sine(unsigned char* samples, int count, double *_phase,
	unsigned int channels, unsigned int samplerate, unsigned int freq,
	snd_pcm_format_t format, unsigned int frame_bits,
	unsigned int volume);
int bits_per_sample(snd_pcm_format_t format);
int calc_framesize(pcm_device* hw);
int interval_refine(snd_interval_t *i, const snd_interval_t *v);
int mask_test(const snd_mask_t *mask, unsigned int val);
void pcm_hw_param_any(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var);
void pcm_hw_params_any(snd_pcm_hw_params_t *params);
int pcm_hw_param_set_minmax(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int min, int mindir, unsigned int max, int maxdir);
int pcm_hw_param_set(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int val, int dir);
int pcm_hw_param_get(const snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int *val, int *dir);
int interval_refine_max(snd_interval_t *i, unsigned int max, int openmax);
int str_to_format(const char* str);
int str_to_access(const char* str);
int str_to_async(const char* str);
const char* format_to_str(int format);
const char* access_to_str(int access);
const char* async_to_str(int val);
struct slist *slist_prepend(struct slist *list, void *item);
struct slist *slist_delete_head(struct slist *list, void (*free_func)(void *));
int check_pcm_params(pcm_params* params);

#endif /* ALSA_UTIL_H */

