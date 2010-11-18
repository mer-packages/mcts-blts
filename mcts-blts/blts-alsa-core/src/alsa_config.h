/* alsa_config.h -- ALSA configuration interface and definitions

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

#ifndef ALSA_CONFIG_FILE_H
#define ALSA_CONFIG_FILE_H

#include <stdio.h>
#include <limits.h>

/* Maximum line length in config file */
#define MAX_LINE_LEN 256

#define MAGIC_NUMBER 256

#define MAX_STREAMS 10

enum {
	STREAM_DIR_NONE = 0,
	STREAM_DIR_IN,
	STREAM_DIR_OUT
};

typedef struct
{
	char control_name[MAGIC_NUMBER];
	int control_id;
	long long value;
	/* str_value is usable only with enumerated values,
	   other types do not have value name */
	char str_value[MAX_LINE_LEN];
} alsa_control_value;

typedef struct
{
	int card;
	int num_values;
	alsa_control_value values[MAGIC_NUMBER];
} alsa_control_settings;

typedef struct
{
	int card;
	int device;
	int dir;
	int num_rates;
	int rates[MAGIC_NUMBER];
	int num_formats;
	int formats[MAGIC_NUMBER];
	int num_channels;
	int channels[MAGIC_NUMBER];
	int num_accesses;
	int access[MAGIC_NUMBER];
	int hw_resampling;
	int num_asyncs;
	int async[MAGIC_NUMBER];
	int duration;
	int freq;
	int link_card;
	int link_device;
	int period_size;
	int volume;
} alsa_pcm_settings;

typedef struct
{
	char device[64];
	int freq;
	int scan;
} v4l_tuner_settings;

typedef struct
{
	char remote_mac[64];
	char pin[64];
} bluetooth_audio_settings;

typedef struct
{
	char description[MAX_LINE_LEN];
	int num_pcms;
	alsa_pcm_settings* pcms[MAX_STREAMS];
	int num_ctls;
	alsa_control_settings* ctls[MAX_STREAMS];
	v4l_tuner_settings tuner;
	bluetooth_audio_settings btaudio;
} alsa_testset;

typedef struct
{
	int num_testsets;
	alsa_testset* testsets[MAGIC_NUMBER];
	int num_def_ctls;
	alsa_control_settings* def_ctls[MAX_STREAMS];
} alsa_configuration;

int alsa_read_config(const char* filename, alsa_configuration* config);
int alsa_free_config(alsa_configuration* config);

#endif /* ALSA_CONFIG_FILE_H */

