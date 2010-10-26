/* v4l_tuner.c -- V4L tuner functionality

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
#include <sys/ioctl.h>
#include <asm/types.h>

#include <blts_log.h>
#include <blts_timing.h>
#include "v4l_tuner.h"

/* 1000 / 62.5 (kHz) */
#define FREQ_UNIT 16

static int update_tuner(tuner_device* dev)
{
	memset(&dev->v4l2tuner, 0, sizeof(struct v4l2_tuner));

	if(ioctl(dev->fd, VIDIOC_G_TUNER, &dev->v4l2tuner) == -1)
	{
		BLTS_ERROR("Failed to get tuner (%d)\n", errno);
		return -errno;
	}

	if(dev->v4l2tuner.capability == V4L2_TUNER_CAP_LOW)
	{
		dev->v4l2tuner.rangelow = dev->v4l2tuner.rangelow / FREQ_UNIT;
		dev->v4l2tuner.rangehigh = dev->v4l2tuner.rangehigh / FREQ_UNIT;
	}
	else
	{
		dev->v4l2tuner.rangelow = dev->v4l2tuner.rangelow * 1000 / FREQ_UNIT;
		dev->v4l2tuner.rangehigh = dev->v4l2tuner.rangehigh * 1000 / FREQ_UNIT;
	}

	return 0;
}

tuner_device* tuner_open(const char* name)
{
	tuner_device* dev = calloc(1, sizeof(tuner_device));
	if(!dev)
	{
		BLTS_LOGGED_PERROR("Failed to allocate tuner_device structure");
		return NULL;
	}

	dev->fd = open(name, O_RDWR);
	if(dev->fd == -1)
	{
		BLTS_ERROR("Failed to open device %s (%d)\n", name, errno);
		return NULL;
	}

	update_tuner(dev);

	return dev;
}

int tuner_close(tuner_device *dev)
{
	if(!dev)
		return -EINVAL;

	if(dev->fd)
		close(dev->fd);

	free(dev);

	return 0;
}

unsigned int tuner_get_freq(tuner_device *dev)
{
	unsigned int freq = 0;

	if(ioctl(dev->fd, VIDIOC_G_FREQUENCY, &dev->v4l2freq) == -1)
	{
		BLTS_ERROR("Failed to get frequency (%d)\n", errno);
		return 0;
	}

	if(dev->v4l2tuner.capability == V4L2_TUNER_CAP_LOW)
		freq = dev->v4l2freq.frequency / FREQ_UNIT;
	else
		freq = dev->v4l2freq.frequency * 1000 / FREQ_UNIT;

	return freq;
}

unsigned int tuner_set_freq(tuner_device* dev, unsigned int freq)
{
	dev->v4l2freq.tuner = 0;
	dev->v4l2freq.type = V4L2_TUNER_RADIO;
	dev->v4l2freq.frequency = (unsigned long)(freq * FREQ_UNIT);

	update_tuner(dev);

	if(!(dev->v4l2tuner.capability & V4L2_TUNER_CAP_LOW))
		dev->v4l2freq.frequency /= 1000;

	if(ioctl(dev->fd, VIDIOC_S_FREQUENCY, &dev->v4l2freq) == -1)
	{
		BLTS_ERROR("Failed to set frequency (%d)\n", errno);
		return 0;
	}

	update_tuner(dev);

	return tuner_get_freq(dev);
}

/* performs hw scan upward from lowest frequency, returns count of found
   channels, or negetive in case of an error */
int tuner_scan(tuner_device* dev, unsigned int timeout)
{
	unsigned int limit = dev->v4l2tuner.rangehigh;
	unsigned int prev_freq = 0;
	unsigned int freq = 0;
	int err;
	struct v4l2_hw_freq_seek v4l2_seek;

	BLTS_DEBUG("Starting scan from %u kHz to %u kHz...\n",
		dev->v4l2tuner.rangelow, dev->v4l2tuner.rangehigh);
	dev->chan_count = 0;

	memset(&v4l2_seek, 0, sizeof(v4l2_seek));
	v4l2_seek.tuner = 0;
	v4l2_seek.type = V4L2_TUNER_RADIO;
	v4l2_seek.wrap_around = 1;
	v4l2_seek.seek_upward = 1;

	freq = tuner_set_freq(dev, dev->v4l2tuner.rangelow);
	if(!freq)
		return -1;

	timing_start();

	while(timing_elapsed() < timeout &&
		freq < limit &&
		dev->chan_count < MAX_TUNER_CHANNELS)
	{
		freq = tuner_get_freq(dev);

		err = ioctl(dev->fd, VIDIOC_S_HW_FREQ_SEEK, &v4l2_seek);
		if(err == -1)
		{
			if(errno == EAGAIN)
				continue;

			BLTS_ERROR("Failed to start scan (%d)\n", errno);
			timing_stop();
			return -errno;
		}

		prev_freq = 0;
		while(timing_elapsed() < timeout && freq < limit)
		{
			freq = tuner_get_freq(dev);
			if(!freq)
			{
				timing_stop();
				return -errno;
			}

			BLTS_TRACE("Current frequency: %d kHz\n", freq);

			if(prev_freq == freq)
			{
				if(dev->chan_count && dev->channels[dev->chan_count - 1] == freq)
				{
					BLTS_TRACE("Skipping channel, same as previous\n");
					break;
				}
				BLTS_DEBUG("Found channel: %u kHz\n", freq);
				dev->channels[dev->chan_count++] = freq;
				/* tuner_set_freq(dev, freq); */
				break;
			}
			prev_freq = freq;
			sleep(1);
		}
	}
	BLTS_DEBUG("Found %d channels\n", dev->chan_count);

	timing_stop();

	return dev->chan_count;
}

