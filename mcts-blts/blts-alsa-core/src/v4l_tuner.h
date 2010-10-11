/* v4l_tuner.h -- V4L tuner functionality

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


#ifndef V4L_TUNER_H
#define V4L_TUNER_H

#include <linux/videodev2.h>

#define MAX_TUNER_CHANNELS 256

typedef struct
{
	int fd;
	struct v4l2_tuner v4l2tuner;
	struct v4l2_frequency v4l2freq;
	int chan_count;
	unsigned int channels[MAX_TUNER_CHANNELS];
} tuner_device;

tuner_device* tuner_open(const char* name);
int tuner_close(tuner_device *dev);
unsigned int tuner_set_freq(tuner_device* dev, unsigned int freq);
unsigned int tuner_get_freq(tuner_device *dev);
int tuner_scan(tuner_device* dev, unsigned int timeout);

#endif /* V4L_TUNER_H */

