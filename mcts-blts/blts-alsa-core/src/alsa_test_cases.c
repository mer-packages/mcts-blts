/* alsa_test_cases.c -- ALSA core test cases

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
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <blts_cli_frontend.h>
#include <blts_dep_check.h>

#include "alsa_test_cases.h"
#include "alsa_pcm.h"
#include "alsa_control.h"
#include "alsa_hwdep.h"
#include "alsa_timer.h"
#include "alsa_profiler.h"
#include "alsa_ioctl.h"

#include "bt_util.h"
#include "v4l_tuner.h"

/** Rules for dep check */
#define DEP_RULES "/usr/lib/tests/blts-alsa-core-tests/blts-alsa-core-req-files.cfg"

struct pcm_thread_params
{
	pthread_t tid;
	pcm_params* pcm;
	int ret;
};

static int setup_control_settings(alsa_control_settings* settings, int verbose)
{
	int t, err = 0;
	ctl_element* ctl = NULL;
	ctl_element* ctl2 = NULL;
	ctl_device* hw;

	if(!settings)
		return -EINVAL;

	/* [control] section is optional in conf file */
	if(!settings->num_values)
		return 0;

	hw = control_open(settings->card);
	if(!hw)
	{
		BLTS_ERROR("Failed to open control device\n");
		return -1;
	}

	for(t = 0; t < settings->num_values; t++)
	{
		if(strlen(settings->values[t].control_name) > 0)
		{
			ctl = control_read(hw, settings->values[t].control_name);
			if(!ctl)
			{
				BLTS_ERROR("Failed to read control element '%s'\n",
					settings->values[t].control_name);
				err = -1;
				goto cleanup;
			}
		}
		else
		{
			ctl = control_read_by_id(hw, settings->values[t].control_id);
			if(!ctl)
			{
				BLTS_ERROR("Failed to read control element %d\n",
					settings->values[t].control_id);
				err = -1;
				goto cleanup;
			}
		}

		if(verbose)
		{
			BLTS_DEBUG("Before:\n");
			control_print_element_value(hw, ctl);
		}

		if(strlen(settings->values[t].str_value))
		{
			if(contol_value_by_name(hw, ctl, settings->values[t].str_value,
				&settings->values[t].value))
			{
				BLTS_ERROR("Failed to write control element, value '%s' not found\n",
					settings->values[t].str_value);
				err = -1;
				goto cleanup;
			}
		}

		if(control_write(hw, ctl, settings->values[t].value))
		{
			BLTS_ERROR("Failed to write control element\n");
			err = -1;
			goto cleanup;
		}

		/* Read again and verify the values */
		if(strlen(settings->values[t].control_name) > 0)
		{
			ctl2 = control_read(hw, settings->values[t].control_name);
			if(!ctl2)
			{
				BLTS_ERROR("Failed to read control element '%s'\n",
					settings->values[t].control_name);
				err = -1;
				goto cleanup;
			}
		}
		else
		{
			ctl2 = control_read_by_id(hw, settings->values[t].control_id);
			if(!ctl2)
			{
				BLTS_ERROR("Failed to read control element %d\n",
					settings->values[t].control_id);
				err = -1;
				goto cleanup;
			}
		}

		if(verbose)
		{
			BLTS_DEBUG("After:\n");
			control_print_element_value(hw, ctl);
		}

		free(ctl);
		free(ctl2);
		ctl = ctl2 = NULL;
	}

cleanup:
	if(ctl)
		free(ctl);
	if(ctl2)
		free(ctl2);

	control_close(hw);

	return err;
}

static int play_sine_simple(pcm_params* params)
{
	int err = 0;

	pcm_device* hw = pcm_open(params->card, params->device, STREAM_DIR_OUT);
	if(!hw)
	{
		err = -1;
		goto cleanup;
	}

	pcm_device_fd_store(params->card, params->device, hw->fd);

	memcpy(&hw->params, params, sizeof(pcm_params));

	err = pcm_init(hw);
	if(err)
		goto cleanup;

	err = pcm_play_sine(hw);
	if(err)
		BLTS_ERROR("Playback failed (%d).\n", err)
	else
		BLTS_DEBUG("Playback ok.\n")

cleanup:

	if(hw)
		pcm_close(hw);

	return err;
}

static int record_audio(pcm_params* params)
{
	int err = 0;

	pcm_device* hw = pcm_open(params->card, params->device, STREAM_DIR_IN);
	if(!hw)
	{
		err = -1;
		goto cleanup;
	}

	pcm_device_fd_store(params->card, params->device, hw->fd);

	memcpy(&hw->params, params, sizeof(pcm_params));

	err = pcm_init(hw);
	if(err)
		goto cleanup;

	err = pcm_record(hw);
	if(err)
		BLTS_ERROR("Recording failed (%d).\n", err)
	else
		BLTS_DEBUG("Recording ok.\n")

cleanup:

	if(hw)
		pcm_close(hw);

	return err;
}

int alsa_presence_check(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(user_ptr)
	BLTS_UNUSED_PARAM(test_num)
	BLTS_DEBUG("Checking required components...\n");
	return depcheck(DEP_RULES, 1);
}

static int check_pcm_device(int card, int dev, int dir)
{
	int err = 0;
	char filename[MAX_DEV_NAME_LEN];

	sprintf(filename, "/dev/snd/pcmC%dD%d%s", card, dev,
		(dir == STREAM_DIR_OUT)?"p":"c");

	if(!access(filename, R_OK))
	{
		BLTS_DEBUG("Checking device %s\n", filename);
		pcm_device* hw = pcm_open(card, dev, dir);
		if(!hw)
		{
			BLTS_ERROR("Failed to open device\n");
			err = -1;
		}
		else
		{
			if(pcm_print_info(hw))
			{
				BLTS_ERROR("Failed to read pcm device information\n");
				err = -1;
			}
			pcm_close(hw);
		}
	}
	else
		err = -ENOENT;

	return err;
}

int alsa_open_close_pcm(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;
	int ret, err = 0;
	int card, dev, count = 0;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		for(dev = 0; dev < MAX_PCM_DEVICES; dev++)
		{
			/* Playback devices */
			ret = check_pcm_device(card, dev, STREAM_DIR_OUT);
			if(ret < 0 && ret != -ENOENT)
				err = -1;
			else if(ret != -ENOENT)
				count++;

			/* Capture devices */
			ret = check_pcm_device(card, dev, STREAM_DIR_IN);
			if(ret < 0 && ret != -ENOENT)
				err = -1;
			else if(ret != -ENOENT)
				count++;
		}
	}
	if(!count)
	{
		BLTS_ERROR("No (working) PCM devices found\n");
		err = -1;
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_open_close_timer(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;
	int err = 0;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	BLTS_DEBUG("Checking device /dev/snd/timer\n");
	timer_device* hw = timer_open();
	if(!hw)
	{
		BLTS_ERROR("Failed to open device\n");
		err = -1;
	}
	else
	{
		if(timer_print_info(hw))
		{
			BLTS_ERROR("Failed to read timer information\n");
			err = -1;
		}
		timer_close(hw);
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_open_close_controls(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int err = 0;
	int card, count = 0;
	char filename[MAX_DEV_NAME_LEN];
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		sprintf(filename, "/dev/snd/controlC%d", card);

		if(!access(filename, R_OK))
		{
			BLTS_DEBUG("Checking device %s\n", filename);
			ctl_device* hw = control_open(card);
			if(!hw)
			{
				BLTS_ERROR("Failed to open device\n");
				err = -1;
			}
			else
			{
				if(control_print_card_info(hw))
				{
					BLTS_ERROR("Failed to read card information\n");
					err = -1;
				}
				else
					count++;
				control_close(hw);
			}
		}
	}

	if(!count)
	{
		BLTS_ERROR("No (working) control devices found\n");
		err = -1;
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_enumerate_control_elements(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int err = 0;
	int card;
	char filename[MAX_DEV_NAME_LEN];
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		sprintf(filename, "/dev/snd/controlC%d", card);

		if(!access(filename, R_OK))
		{
			BLTS_DEBUG("Using device %s\n", filename);
			ctl_device* hw = control_open(card);
			if(!hw)
			{
				BLTS_ERROR("Failed to open device\n");
				err = -1;
			}
			else
			{
				if(control_print_element_list(hw))
				{
					BLTS_ERROR("Failed to read control elements\n");
					err = -1;
				}
				control_close(hw);
			}
		}
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_open_close_hwdeps(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int err = 0;
	int card, dev;
	char filename[MAX_DEV_NAME_LEN];
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		for(dev = 0; dev < MAX_PCM_DEVICES; dev++)
		{
			sprintf(filename, "/dev/snd/hwC%dD%d", card, dev);
			if(!access(filename, R_OK))
			{
				BLTS_DEBUG("Checking device %s\n", filename);
				hwdep_device* hw = hwdep_open(card, dev);
				if(!hw)
				{
					BLTS_ERROR("Failed to open device\n");
					err = -1;
				}
				else
				{
					if(hwdep_print_info(hw))
					{
						BLTS_ERROR("Failed to read hwdep info\n");
						err = -1;
					}
					hwdep_close(hw);
				}
			}
		}
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

static void *pcm_thread_function(void *ptr)
{
	struct pcm_thread_params *params = (struct pcm_thread_params*)ptr;

	if(params->pcm->dir == STREAM_DIR_OUT)
		params->ret = play_sine_simple(params->pcm);
	else if(params->pcm->dir == STREAM_DIR_IN)
		params->ret = record_audio(params->pcm);
	else
		sleep(params->pcm->duration);

	pthread_exit(ptr);

	return NULL;
}

static int setup_ctl_settings(alsa_control_settings **settings, int verbose)
{
	int err;
	while(*settings)
	{
		err = setup_control_settings(*settings, verbose);
		if(err)
		{
			BLTS_ERROR("Failed to setup control settings\n");
			return err;
		}
		settings++;
	}
	return 0;
}

static int setup_tuner(tuner_device *tun, testcase_params *testcase)
{
	int freq = testcase->tuner.freq;
	int new_freq;
	int err;

	if(testcase->tuner.scan > 0)
	{
		/* Assume one minute scan is enough */
		err = tuner_scan(tun, 60);
		if(err < 0)
		{
			BLTS_ERROR("Scan failed\n");
			return err;
		}
		if(tun->chan_count <= 0)
		{
			BLTS_ERROR("No channels found\n");
			return -1;
		}
		if(tun->chan_count < testcase->tuner.scan)
		{
			BLTS_ERROR("Channel %d not found\n", testcase->tuner.scan);
			return -1;
		}
		freq = tun->channels[testcase->tuner.scan - 1];
	}
	new_freq = tuner_set_freq(tun, freq);
	if(!new_freq)
	{
		BLTS_ERROR("Failed to set frequency\n");
		return -1;
	}

	BLTS_DEBUG("Tuned to frequency %d\n", new_freq);

	return 0;
}

int alsa_play_rec_pcm(void* user_ptr, int test_num)
{
	int err = 0, t;
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;
	struct pcm_thread_params *threads = NULL;
	testcase_params* testcase = &params->testcases[test_num - 1];
	tuner_device* tun = NULL;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	pcm_stored_devices_init();

	/* Setup tuner, bt and control settings for the test case */

	if(strlen(testcase->tuner.device) > 0)
	{
		tun = tuner_open(testcase->tuner.device);
		if(!tun)
		{
			err = -1;
			goto cleanup;
		}
	}

	if(strlen(testcase->btaudio.remote_mac) > 0)
	{
		err = bt_util_connect_btaudio(testcase->btaudio.remote_mac,
			testcase->btaudio.pin);
		if(err)
			goto cleanup;
	}

	/* apply default settings... */
	err = setup_ctl_settings(testcase->def_control_ref,
		(blts_log_get_level() == LEVEL_TRACE));
	if(err)
		goto cleanup;

	/* ...and testcase specific settings */
	err = setup_ctl_settings(testcase->control_ref,
		(blts_log_get_level() == LEVEL_TRACE));
	if(err)
		goto cleanup;

	if(tun)
	{
		err = setup_tuner(tun, testcase);
		if(err)
			goto cleanup;
	}

	/* Setup done, start the threads */

	pcm_thread_init(testcase->num_pcm_params);

	threads = calloc(testcase->num_pcm_params,
		sizeof(struct pcm_thread_params));
	if(!threads)
	{
		BLTS_LOGGED_PERROR("calloc failed");
		goto cleanup;
	}

	/* Every thread uses the same common timer, must start and stop timer here */
	timing_start();

	for(t = 0; t < testcase->num_pcm_params; t++)
	{
		threads[t].pcm = testcase->params[t];

		if(threads[t].pcm->dir != STREAM_DIR_NONE)
		{
			BLTS_DEBUG("\nUsed %s parameters for stream %d:\n",
				(threads[t].pcm->dir == STREAM_DIR_OUT)?"playback":"recording", t + 1);
			BLTS_DEBUG("Card: %d\n", threads[t].pcm->card);
			BLTS_DEBUG("Device: %d\n", threads[t].pcm->device);
			BLTS_DEBUG("Rate: %d\n", threads[t].pcm->rate);
			BLTS_DEBUG("Channels: %d\n", threads[t].pcm->channels);
			BLTS_DEBUG("Access: %s\n", access_to_str(threads[t].pcm->access));
			BLTS_DEBUG("Format: %s\n", format_to_str(threads[t].pcm->format));
			BLTS_DEBUG("HW resampling: %s\n", threads[t].pcm->hw_resampling?"on":"off");
			BLTS_DEBUG("Duration: %d seconds\n", threads[t].pcm->duration);
			if(threads[t].pcm->period_size > 0)
			{
				BLTS_DEBUG("Period size: %d frames\n",
					threads[t].pcm->period_size);
			}
			if(threads[t].pcm->dir == STREAM_DIR_OUT)
			{
				BLTS_DEBUG("Output frequency: %d\n", threads[t].pcm->freq);
				if (threads[t].pcm->async != TEST_ASYNC_MODE_NONE)
					BLTS_DEBUG("Async mode: %s\n",async_to_str(threads[t].pcm->async));
			}
			if(threads[t].pcm->link_card >= 0 && threads[t].pcm->link_device >= 0)
			{
				BLTS_DEBUG("Linked with card %d device %d\n",
					threads[t].pcm->link_card, threads[t].pcm->link_device);
			}

			err = check_pcm_params(threads[t].pcm);
			if(err)
			{
				BLTS_ERROR("Configuration file contains invalid values\n");
				goto cleanup;
			}
		}
		else
		{
			BLTS_DEBUG("\nWaiting for %d seconds...\n", threads[t].pcm->duration);
		}

		err = pthread_create(&threads[t].tid, NULL, pcm_thread_function,
			&threads[t]);
		if(err)
		{
			BLTS_ERROR("Failed to start thread (%d)\n", err);
			goto cleanup;
		}
	}

	/* wait until all threads are done */
	for(t = 0; t < testcase->num_pcm_params; t++)
	{
		pthread_join(threads[t].tid, NULL);
		if(threads[t].ret)
			err = threads[t].ret;
		BLTS_TRACE("Thread %u exited with %d\n", threads[t].tid, threads[t].ret);
		threads[t].tid = 0;
	}

cleanup:
	timing_stop();

	if(threads)
	{
		for(t = 0; t < testcase->num_pcm_params; t++)
		{
			if(threads[t].tid)
				pthread_cancel(threads[t].tid);
		}
		free(threads);
	}

	pcm_stored_devices_free();

	/* (try to) apply default settings */
	if(setup_ctl_settings(testcase->def_control_ref,
		(blts_log_get_level() == LEVEL_TRACE)))
		err = -1;

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	if(strlen(testcase->btaudio.remote_mac) > 0)
	{
		bt_util_disconnect_btaudio();
	}

	if(tun)
		tuner_close(tun);

	return err;
}

static int set_ctl_power_states(ctl_device* hw)
{
	int state = 0;
	int err = 0;

	err = ioctl_ctl_power_state(hw, &state);
	if(err)
	{
		BLTS_ERROR("Failed to read power state\n");
		return err;
	}

	BLTS_DEBUG("Current state: %s\n", power_state_to_string(state));

	state = SNDRV_CTL_POWER_D0;
	err = ioctl_ctl_power(hw, &state);
	if(err == -ENOPROTOOPT)
	{
		BLTS_DEBUG("Setting power state not (yet) supported by ALSA core\n");
		err = 0;
	}
	else if(err)
	{
		BLTS_ERROR("Failed to set power state\n");
		return err;
	}
	else
	{
		BLTS_DEBUG("Current state: %s\n", power_state_to_string(state));
	}

	return err;
}

int alsa_power_management(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int card, ret;
	int err = 0;
	char filename[MAX_DEV_NAME_LEN];
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		sprintf(filename, "/dev/snd/controlC%d", card);

		if(!access(filename, R_OK))
		{
			BLTS_DEBUG("Checking device %s\n", filename);
			ctl_device* hw = control_open(card);
			if(!hw)
			{
				BLTS_ERROR("Failed to open device\n");
				err = -1;
			}
			else
			{
				ret = set_ctl_power_states(hw);
				if(ret)
					err = -1;
				control_close(hw);
			}
		}
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_enumerate_ctl_devices(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int err = 0;
	int card, count = 0;
	char filename[MAX_DEV_NAME_LEN];
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	for(card = 0; card < MAX_CARDS; card++)
	{
		sprintf(filename, "/dev/snd/controlC%d", card);

		if(!access(filename, R_OK))
		{
			BLTS_DEBUG("Checking device %s\n", filename);
			ctl_device* hw = control_open(card);
			if(!hw)
			{
				BLTS_ERROR("Failed to open device\n");
				err = -1;
			}
			else
			{
				if(control_enumerate_devices(hw))
				{
					BLTS_ERROR("Failed to read card information\n");
					err = -1;
				}
				else
					count++;
				control_close(hw);
			}
		}
	}

	if(!count)
	{
		BLTS_ERROR("No (working) control devices found\n");
		err = -1;
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	return err;
}

int alsa_add_remove_ctl_element(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int err = 0, ret;
	int used_device = 0;
	alsa_cli_params* params = (alsa_cli_params*) user_ptr;
	struct snd_ctl_elem_info info;
	ctl_device* hw = NULL;
	ctl_element* elem = NULL;
	ctl_device* hw2 = NULL;
	ctl_element* elem2 = NULL;
	const char *test_ctl_name = "BLTS_TEST_CONTROL";

	if(params->flags&CLI_FLAG_PROFILING)
		ioctl_start_profiling();

	hw = control_open(used_device);
	if(!hw)
	{
		BLTS_ERROR("Failed to open device\n");
		err = -1;
		goto cleanup;
	}

	memset(&info, 0, sizeof(info));
	info.id.numid = UINT_MAX;
	info.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	info.id.device = used_device;
	info.id.subdevice = 0;
	memcpy(info.id.name, test_ctl_name, strlen(test_ctl_name));
	info.id.index = 0;
	info.type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	info.access = SNDRV_CTL_ELEM_ACCESS_USER |
		SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_LOCK;
	info.count = 1;
	info.value.integer.min = 10;
	info.value.integer.max = 100;
	info.value.integer.step = 2;

	BLTS_DEBUG("Adding new element\n");

	err = ioctl_ctl_elem_add(hw, &info);
	if(err)
	{
		BLTS_ERROR("Failed to add control element (%d)\n", err);
		goto cleanup;
	}

	elem = control_read(hw, test_ctl_name);
	if(!elem)
	{
		BLTS_ERROR("Could not find control element by name\n");
		err = -1;
		goto cleanup;
	}

	if(elem->info.value.integer.min != 10 ||
		elem->info.value.integer.max != 100 ||
		elem->info.value.integer.step != 2)
	{
		BLTS_ERROR("Control element values do not match\n");
		err = -1;
	}

	ret = control_write(hw, elem, 50);
	if(ret)
	{
		BLTS_ERROR("Failed to write control element value (%d)\n", ret);
		err = ret;
	}

	BLTS_DEBUG("Replacing control element\n");
	memcpy(&info, &elem->info, sizeof(info));
	info.value.integer.min = 20;
	info.value.integer.max = 50;
	info.value.integer.step = 0;
	ret = ioctl_ctl_elem_replace(hw, &info);
	if(ret)
	{
		BLTS_ERROR("Failed to replace control element (%d)\n", ret);
		err = ret;
	}

	free(elem);

	/* Test lock/unlock. Use two fds, lock the control from fd1 and try to
	   write with fd2, which should fail */
	control_close(hw);
	hw = control_open(used_device);
	if(!hw)
	{
		BLTS_ERROR("Failed to open device\n");
		err = -1;
		goto cleanup;
	}

	hw2 = control_open(used_device);
	if(!hw)
	{
		BLTS_ERROR("Failed to open device\n");
		err = -1;
		goto cleanup;
	}

	elem = control_read(hw, test_ctl_name);
	if(!elem)
	{
		BLTS_ERROR("Could not find control element by name\n");
		err = -1;
		goto cleanup;
	}

	if(elem->info.value.integer.min != 20 ||
		elem->info.value.integer.max != 50 ||
		elem->info.value.integer.step != 0)
	{
		BLTS_ERROR("Control element values do not match\n");
		err = -1;
	}

	BLTS_DEBUG("Lock the element\n");

	ret = ioctl_ctl_elem_lock(hw, &elem->info.id);
	if(ret)
	{
		BLTS_ERROR("Failed to lock the element\n");
		err = -1;
		goto cleanup;
	}

	elem2 = control_read(hw2, test_ctl_name);
	if(!elem2)
	{
		BLTS_ERROR("Could not find control element by name\n");
		err = -1;
		goto cleanup;
	}

	BLTS_DEBUG("Try to write, this should fail\n");
	ret = control_write(hw2, elem2, 50);
	if(!ret)
	{
		BLTS_DEBUG("Expected error, got none\n");
		err = -1;
		goto cleanup;
	}

	ret = ioctl_ctl_elem_unlock(hw, &elem->info.id);
	if(ret)
	{
		BLTS_ERROR("Failed to unlock the element\n");
		err = -1;
		goto cleanup;
	}

cleanup:
	if(elem)
	{
		BLTS_DEBUG("Remove created element\n");
		ret = ioctl_ctl_elem_remove(hw, &elem->info.id);
		if(ret)
		{
			BLTS_ERROR("Failed to remove control element (%d)\n", ret);
			err = ret;
		}
	}

	ioctl_print_profiling_data();
	ioctl_stop_profiling();

	if(hw)
		control_close(hw);
	if(hw2)
		control_close(hw2);
	if(elem)
		free(elem);
	if(elem2)
		free(elem2);

	return err;
}

