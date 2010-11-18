/* alsa_util.c -- Various helper functions

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
#include <math.h>
#include <limits.h>

#include "alsa_util.h"

#define hw_param_mask(params,var) \
	&((params)->masks[(var) - SNDRV_PCM_HW_PARAM_FIRST_MASK])

#define hw_param_interval(params,var) \
	&((params)->intervals[(var) - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL])

#define MASK_SIZE (SNDRV_MASK_MAX / 32)
#define MASK_OFS(i)	((i) >> 5)
#define MASK_BIT(i)	(1U << ((i) & 31))

typedef struct
{
	snd_pcm_format_t format;
	int size;
} format_sample_size;

static const format_sample_size format_sample_sizes[] =
{
	{SNDRV_PCM_FORMAT_S8, 8},
	{SNDRV_PCM_FORMAT_U8, 8},
	{SNDRV_PCM_FORMAT_S16_LE, 16},
	{SNDRV_PCM_FORMAT_S16_BE, 16},
	{SNDRV_PCM_FORMAT_U16_LE, 16},
	{SNDRV_PCM_FORMAT_U16_BE, 16},
	{SNDRV_PCM_FORMAT_S24_LE, 24},
	{SNDRV_PCM_FORMAT_S24_BE, 24},
	{SNDRV_PCM_FORMAT_U24_LE, 24},
	{SNDRV_PCM_FORMAT_U24_BE, 24},
	{SNDRV_PCM_FORMAT_S32_LE, 32},
	{SNDRV_PCM_FORMAT_S32_BE, 32},
	{SNDRV_PCM_FORMAT_U32_LE, 32},
	{SNDRV_PCM_FORMAT_U32_BE, 32},
	{SNDRV_PCM_FORMAT_FLOAT_LE, 32},
	{SNDRV_PCM_FORMAT_FLOAT_BE, 32},
	{SNDRV_PCM_FORMAT_FLOAT64_LE, 64},
	{SNDRV_PCM_FORMAT_FLOAT64_BE, 64},
};

const gen_vs format_values[] = {
	{ "S8", SNDRV_PCM_FORMAT_S8 },
	{ "U8", SNDRV_PCM_FORMAT_U8 },
	{ "S16_LE", SNDRV_PCM_FORMAT_S16_LE },
	{ "S16_BE", SNDRV_PCM_FORMAT_S16_BE },
	{ "U16_LE", SNDRV_PCM_FORMAT_U16_LE },
	{ "U16_BE", SNDRV_PCM_FORMAT_U16_BE },
	{ "S24_LE", SNDRV_PCM_FORMAT_S24_LE },
	{ "S24_BE", SNDRV_PCM_FORMAT_S24_BE },
	{ "U24_LE", SNDRV_PCM_FORMAT_U24_LE },
	{ "U24_BE", SNDRV_PCM_FORMAT_U24_BE },
	{ "S32_LE", SNDRV_PCM_FORMAT_S32_LE },
	{ "S32_BE", SNDRV_PCM_FORMAT_S32_BE },
	{ "U32_LE", SNDRV_PCM_FORMAT_U32_LE },
	{ "U32_BE", SNDRV_PCM_FORMAT_U32_BE },
	{ "FLOAT_LE", SNDRV_PCM_FORMAT_FLOAT_LE },
	{ "FLOAT_BE", SNDRV_PCM_FORMAT_FLOAT_BE },
	{ "FLOAT64_LE", SNDRV_PCM_FORMAT_FLOAT64_LE },
	{ "FLOAT64_BE", SNDRV_PCM_FORMAT_FLOAT64_BE },
	{ "IEC958_SUBFRAME_LE", SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE  },
	{ "IEC958_SUBFRAME_BE", SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE  },
	{ "MU_LAW", SNDRV_PCM_FORMAT_MU_LAW },
	{ "A_LAW", SNDRV_PCM_FORMAT_A_LAW },
	{ "IMA_ADPCM", SNDRV_PCM_FORMAT_IMA_ADPCM },
	{ "FORMAT_MPEG", SNDRV_PCM_FORMAT_MPEG },
	{ "GSM", SNDRV_PCM_FORMAT_GSM },
	{ "SPECIAL", SNDRV_PCM_FORMAT_SPECIAL },
	{ "S24_3LE", SNDRV_PCM_FORMAT_S24_3LE },
	{ "S24_3BE", SNDRV_PCM_FORMAT_S24_3BE },
	{ "U24_3LE", SNDRV_PCM_FORMAT_U24_3LE },
	{ "U24_3BE", SNDRV_PCM_FORMAT_U24_3BE },
	{ "S20_3LE", SNDRV_PCM_FORMAT_S20_3LE },
	{ "S20_3BE", SNDRV_PCM_FORMAT_S20_3BE },
	{ "U20_3LE", SNDRV_PCM_FORMAT_U20_3LE },
	{ "U20_3BE", SNDRV_PCM_FORMAT_U20_3BE },
	{ "S18_3LE", SNDRV_PCM_FORMAT_S18_3LE },
	{ "S18_3BE", SNDRV_PCM_FORMAT_S18_3BE },
	{ "U18_3LE", SNDRV_PCM_FORMAT_U18_3LE },
	{ "U18_3BE", SNDRV_PCM_FORMAT_U18_3BE },
	{ NULL, 0 }
};

const gen_vs access_values[] = {
	{ "MMAP_INTERLEAVED", SNDRV_PCM_ACCESS_MMAP_INTERLEAVED },
	{ "MMAP_NONINTERLEAVED", SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED },
	{ "MMAP_COMPLEX", SNDRV_PCM_ACCESS_MMAP_COMPLEX },
	{ "INTERLEAVED", SNDRV_PCM_ACCESS_RW_INTERLEAVED },
	{ "NONINTERLEAVED", SNDRV_PCM_ACCESS_RW_NONINTERLEAVED },
	{ NULL, 0 }
};

const gen_vs async_values[] = {
	{ "SYNC", TEST_ASYNC_MODE_NONE },
	{ "ASYNC_SIGNAL", TEST_ASYNC_MODE_SIGNAL },
	{ "ASYNC_TIMER", TEST_ASYNC_MODE_TIMER },
	{ NULL, 0 }
};

int bits_per_sample(snd_pcm_format_t format)
{
	unsigned int t;

	for(t = 0; t < ARRAY_SIZE(format_sample_sizes); t++)
	{
		if(format == format_sample_sizes[t].format)
			return format_sample_sizes[t].size;
	}

	BLTS_ERROR("Unknown sample format %d\n", format);

	return -EINVAL;
}

int generate_sine(unsigned char* samples, int count, double *_phase,
	unsigned int channels, unsigned int samplerate, unsigned int freq,
	snd_pcm_format_t format, unsigned int frame_bits, unsigned int volume)
{
	static double max_phase = 2. * M_PI;
	double phase = *_phase;
	double step = max_phase * freq / (double)samplerate;
	int sample;
	unsigned int t, i;
	unsigned int bps = bits_per_sample(format);
	unsigned int leftover_bits = frame_bits / channels - bps;
	unsigned int maxval = ((1 << (bps - 1)) - 1) / 100 * volume;

	if(!samples || !_phase || bps < 1)
		return -EINVAL;

	while(count-- > 0)
	{
		sample = (int)(sin(phase) * maxval);
		for(t  = 0;  t < channels; t++)
		{
			for(i = 0; i < bps / 8; i++)
				*(samples++) = ((unsigned char*)&sample)[i];
			for(i = 0; i < leftover_bits / 8; i++)
				*(samples++) = 0;
		}
		phase += step;
		if(phase >= max_phase)
			phase -= max_phase;
	}
	*_phase = phase;

	return 0;
}

int calc_framesize(pcm_device* hw)
{
	return hw->frame_bits / 8;
}

int mask_test(const snd_mask_t *mask, unsigned int val)
{
	return mask->bits[MASK_OFS(val)] & MASK_BIT(val);
}

int mask_empty(const snd_mask_t *mask)
{
	int i;
	for(i = 0; i < MASK_SIZE; i++)
		if(mask->bits[i])
			return 0;
	return 1;
}

int mask_single(const snd_mask_t *mask)
{
	int i, c = 0;
	for(i = 0; i < MASK_SIZE; i++)
	{
		if(! mask->bits[i])
			continue;
		if(mask->bits[i] & (mask->bits[i] - 1))
			return 0;
		if(c)
			return 0;
		c++;
	}
	return 1;
}

void mask_leave(snd_mask_t *mask, unsigned int val)
{
	unsigned int v;
	v = mask->bits[MASK_OFS(val)] & MASK_BIT(val);
	memset(mask, 0, sizeof(*mask));
	mask->bits[MASK_OFS(val)] = v;
}

int mask_refine_set(snd_mask_t *mask, unsigned int val)
{
	int changed;
	if(mask_empty(mask))
		return -ENOENT;
	changed = !mask_single(mask);
	mask_leave(mask, val);
	if(mask_empty(mask))
		return -EINVAL;
	return changed;
}

void mask_reset_range(snd_mask_t *mask, unsigned int from, unsigned int to)
{
	unsigned int i;
	for(i = from; i <= to; i++)
		mask->bits[MASK_OFS(i)] &= ~MASK_BIT(i);
}

void interval_any(snd_interval_t *i)
{
	memset(i, 0, sizeof(snd_interval_t));
	i->max = UINT_MAX;
}

int interval_refine_set(snd_interval_t *i, unsigned int val)
{
	snd_interval_t t;
	t.empty = 0;
	t.min = t.max = val;
	t.openmin = t.openmax = 0;
	t.integer = 1;
	return interval_refine(i, &t);
}

int interval_refine(snd_interval_t *i, const snd_interval_t *v)
{
	int changed = 0;

	if(i->empty)
		return -ENOENT;

	if(i->min < v->min)
	{
		i->min = v->min;
		i->openmin = v->openmin;
		changed = 1;
	}
	else if(i->min == v->min && !i->openmin && v->openmin)
	{
		i->openmin = 1;
		changed = 1;
	}

	if(i->max > v->max)
	{
		i->max = v->max;
		i->openmax = v->openmax;
		changed = 1;
	}
	else if(i->max == v->max && !i->openmax && v->openmax)
	{
		i->openmax = 1;
		changed = 1;
	}

	if(!i->integer && v->integer)
	{
		i->integer = 1;
		changed = 1;
	}

	if(i->integer)
	{
		if(i->openmin)
		{
			i->min++;
			i->openmin = 0;
		}
		if(i->openmax)
		{
			i->max--;
			i->openmax = 0;
		}
	}
	else if(!i->openmin && !i->openmax && i->min == i->max)
		i->integer = 1;

	if((i->min > i->max || (i->min == i->max && (i->openmin || i->openmax))))
	{
		i->empty = 1;
		return -EINVAL;
	}

	return changed;
}

unsigned int ld2(u_int32_t v)
{
	unsigned r = 0;

	if(v >= 0x10000)
	{
		v >>= 16;
		r += 16;
	}

	if(v >= 0x100)
	{
		v >>= 8;
		r += 8;
	}

	if(v >= 0x10)
	{
		v >>= 4;
		r += 4;
	}

	if(v >= 4)
	{
		v >>= 2;
		r += 2;
	}

	if(v >= 2)
		r++;

	return r;
}

unsigned int mask_min(const snd_mask_t *mask)
{
	int i;
	for(i = 0; i < MASK_SIZE; i++)
		if(mask->bits[i])
			return ffs(mask->bits[i]) - 1 + (i << 5);
	return 0;
}

unsigned int mask_max(const snd_mask_t *mask)
{
	int i;
	for(i = MASK_SIZE - 1; i >= 0; i--)
		if(mask->bits[i])
			return ld2(mask->bits[i]) + (i << 5);
	return 0;
}

int mask_refine_min(snd_mask_t *mask, unsigned int val)
{
	if(mask_empty(mask))
		return -ENOENT;
	if(mask_min(mask) >= val)
		return 0;
	mask_reset_range(mask, 0, val - 1);
	if(mask_empty(mask))
		return -EINVAL;
	return 1;
}

int mask_refine_max(snd_mask_t *mask, unsigned int val)
{
	if(mask_empty(mask))
		return -ENOENT;
	if(mask_max(mask) <= val)
		return 0;
	mask_reset_range(mask, val + 1, SNDRV_MASK_MAX);
	if(mask_empty(mask))
		return -EINVAL;
	return 1;
}

int interval_refine_min(snd_interval_t *i, unsigned int min, int openmin)
{
	int changed = 0;

	if(i->empty)
		return -ENOENT;

	if(i->min < min)
	{
		i->min = min;
		i->openmin = openmin;
		changed = 1;
	}
	else if(i->min == min && !i->openmin && openmin)
	{
		i->openmin = 1;
		changed = 1;
	}

	if(i->integer)
	{
		if(i->openmin)
		{
			i->min++;
			i->openmin = 0;
		}
	}

	if((i->min > i->max || (i->min == i->max && (i->openmin || i->openmax))))
	{
		i->empty = 1;
		return -EINVAL;
	}

	return changed;
}

int interval_refine_max(snd_interval_t *i, unsigned int max, int openmax)
{
	int changed = 0;

	if(i->empty)
		return -ENOENT;

	if(i->max > max)
	{
		i->max = max;
		i->openmax = openmax;
		changed = 1;
	}
	else if(i->max == max && !i->openmax && openmax)
	{
		i->openmax = 1;
		changed = 1;
	}

	if(i->integer)
	{
		if(i->openmax)
		{
			i->max--;
			i->openmax = 0;
		}
	}

	if((i->min > i->max || (i->min == i->max && (i->openmin || i->openmax))))
	{
		i->empty = 1;
		return -EINVAL;
	}

	return changed;
}

int pcm_hw_param_get(const snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int *val, int *dir)
{
	if((var >= SNDRV_PCM_HW_PARAM_FIRST_MASK &&
		var <= SNDRV_PCM_HW_PARAM_LAST_MASK))
	{
		const snd_mask_t *mask = hw_param_mask(params, var);
		if(mask_empty(mask) || !mask_single(mask))
			return -EINVAL;
		if(dir)
			*dir = 0;
		if(val)
			*val = mask_min(mask);
		return 0;
	}
	else if((var >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL))
	{
		const snd_interval_t *i = hw_param_interval(params, var);
		if(i->empty || !(i->min == i->max || (i->min + 1 == i->max && i->openmax)))
			return -EINVAL;
		if(dir)
			*dir = i->openmin;
		if(val)
			*val = i->min;
		return 0;
	}

	return -EINVAL;
}

int pcm_hw_param_set(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int val, int dir)
{
	int changed;
	if((var >= SNDRV_PCM_HW_PARAM_FIRST_MASK &&
		var <= SNDRV_PCM_HW_PARAM_LAST_MASK))
	{
		snd_mask_t *m = hw_param_mask(params, var);
		if(val == 0 && dir < 0)
		{
			changed = -EINVAL;
			memset(m, 0, sizeof(*m));
		}
		else
		{
			if(dir > 0)
				val++;
			else if(dir < 0)
				val--;
			changed = mask_refine_set(hw_param_mask(params, var), val);
		}
	}
	else if((var >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL))
	{
		snd_interval_t *i = hw_param_interval(params, var);
		if(val == 0 && dir < 0)
		{
			changed = -EINVAL;
			i->empty = 1;
		}
		else if(dir == 0)
			changed = interval_refine_set(i, val);
		else
		{
			snd_interval_t t;
			t.openmin = 1;
			t.openmax = 1;
			t.empty = 0;
			t.integer = 0;
			if(dir < 0)
			{
				t.min = val - 1;
				t.max = val;
			}
			else
			{
				t.min = val;
				t.max = val + 1;
			}
			changed = interval_refine(i, &t);
		}
	}
	else
	{
		return -EINVAL;
	}

	if(changed)
	{
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}

	return changed;
}

int pcm_hw_param_set_minmax(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var,
	unsigned int min, int mindir, unsigned int max, int maxdir)
{
	int changed, c1, c2;
	int openmin = 0, openmax = 0;

	if(mindir)
	{
		if(mindir > 0)
		{
			openmin = 1;
		}
		else if(mindir < 0)
		{
			if(min > 0)
			{
				openmin = 1;
				min--;
			}
		}
	}

	if(maxdir)
	{
		if(maxdir < 0)
		{
			openmax = 1;
		}
		else if(maxdir > 0)
		{
			openmax = 1;
			max++;
		}
	}

	if((var >= SNDRV_PCM_HW_PARAM_FIRST_MASK &&
		var <= SNDRV_PCM_HW_PARAM_LAST_MASK))
	{
		snd_mask_t *mask = hw_param_mask(params, var);
		if(max == 0 && openmax)
		{
			memset(mask, 0, sizeof(*mask));
			changed = -EINVAL;
		}
		else
		{
			c1 = mask_refine_min(mask, min + !!openmin);
			if(c1 < 0)
				changed = c1;
			else {
				c2 = mask_refine_max(mask, max - !!openmax);
				if(c2 < 0)
					changed = c2;
				else
					changed = (c1 || c2);
			}
		}
	}
	else if((var >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL))
	{
		snd_interval_t *i = hw_param_interval(params, var);
		c1 = interval_refine_min(i, min, openmin);
		if(c1 < 0)
			changed = c1;
		else {
			c2 = interval_refine_max(i, max, openmax);
			if(c2 < 0)
				changed = c2;
			else
				changed = (c1 || c2);
		}
	}
	else
	{
		return -EINVAL;
	}

	if(changed)
	{
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}

	return changed;
}

void pcm_hw_param_any(snd_pcm_hw_params_t *params, snd_pcm_hw_param_t var)
{
	if((var >= SNDRV_PCM_HW_PARAM_FIRST_MASK &&
		var <= SNDRV_PCM_HW_PARAM_LAST_MASK))
	{
		memset(hw_param_mask(params, var), 0xff, MASK_SIZE * sizeof(u_int32_t));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
		return;
	}

	if((var >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL))
	{
		interval_any(hw_param_interval(params, var));
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
		return;
	}
}

void pcm_hw_params_any(snd_pcm_hw_params_t *params)
{
	unsigned int k;

	memset(params, 0, sizeof(*params));

	for(k = SNDRV_PCM_HW_PARAM_FIRST_MASK; k <= SNDRV_PCM_HW_PARAM_LAST_MASK; k++)
		pcm_hw_param_any(params, k);

	for(k = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; k <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; k++)
		pcm_hw_param_any(params, k);

	params->rmask = ~0U;
	params->cmask = 0;
	params->info = ~0U;
}

int str_to_format(const char* str)
{
	int t = 0;

	while(format_values[t].str != NULL)
	{
		if(!strncasecmp(str, format_values[t].str, strlen(format_values[t].str)))
			return format_values[t].value;
		t++;
	}
	return -1;
}

int str_to_access(const char* str)
{
	int t = 0;

	while(access_values[t].str != NULL)
	{
		if(!strncasecmp(str, access_values[t].str, strlen(access_values[t].str)))
			return access_values[t].value;
		t++;
	}
	return -1;
}

const char* format_to_str(int format)
{
	int t = 0;

	while(format_values[t].str != NULL)
	{
		if(format_values[t].value == format)
			return format_values[t].str;
		t++;
	}
	return "Unknown";
}

const char* access_to_str(int format)
{
	int t = 0;

	while(access_values[t].str != NULL)
	{
		if(access_values[t].value == format)
			return access_values[t].str;
		t++;
	}
	return "Unknown";
}

const char* async_to_str(int val)
{
	int t = 0;

	while(async_values[t].str != NULL)
	{
		if(async_values[t].value == val)
			return async_values[t].str;
		t++;
	}
	return "Unknown";
}

int str_to_async(const char* str)
{
	int t = 0;

	while(async_values[t].str != NULL)
	{
		if(!strncasecmp(str, async_values[t].str, strlen(async_values[t].str)))
			return async_values[t].value;
		t++;
	}
	return -1;
}

/* Add item to list head, return new head; list can be null. */
struct slist *slist_prepend(struct slist *list, void *item)
{
	struct slist *head;
	if (!item)
		return 0;
	head = malloc(sizeof *head);
	if (!head)
		return 0;
	head->next = list;
	head->item = item;
	return head;
}

/* Delete list head, optionally call free_func for content. Return new head
   (null if empty). */
struct slist *slist_delete_head(struct slist *list, void (*free_func)(void *))
{
	struct slist *new_head;
	if (!list)
		return 0;
	new_head = list->next;
	if (free_func && list->item)
		free_func(list->item);
	free(list);
	return new_head;
}

int check_pcm_params(pcm_params* params)
{
	int err = 0;

	if(params->card < 0 || params->card >= MAX_CARDS)
	{
		BLTS_ERROR("Invalid card (%d)\n", params->card);
		err = -EINVAL;
	}

	if(params->device < 0 || params->device >= MAX_PCM_DEVICES)
	{
		BLTS_ERROR("Invalid device (%d)\n", params->device);
		err = -EINVAL;
	}

	switch(params->format)
	{
		case SNDRV_PCM_FORMAT_S8:
		case SNDRV_PCM_FORMAT_S16_LE:
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S32_LE:
			break;
		default:
			BLTS_ERROR("Unsupported format (%d, %s)\n", params->format,
				format_to_str(params->format));
			err = -EINVAL;
			break;
	}

	if(params->channels < 1 || params->channels >= MAX_CHANNELS)
	{
		BLTS_ERROR("Invalid channel count (%d)\n", params->channels);
		err = -EINVAL;
	}

	switch(params->access)
	{
		case SNDRV_PCM_ACCESS_MMAP_INTERLEAVED:
		case SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED:
		case SNDRV_PCM_ACCESS_RW_INTERLEAVED:
		case SNDRV_PCM_ACCESS_RW_NONINTERLEAVED:
			break;
		/* MMAP_COMPLEX */
		default:
			BLTS_ERROR("Unsupported access method (%d, %s)\n", params->access,
				access_to_str(params->access));
			err = -EINVAL;
			break;
	}

	if(params->async < TEST_ASYNC_MODE_NONE ||
	 params->async > TEST_ASYNC_MODE_TIMER)
	{
		BLTS_ERROR("Invalid async mode (%d)\n", params->async);
		err = -EINVAL;
	}

	return err;
}

