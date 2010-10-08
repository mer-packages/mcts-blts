/* blts_alsa_core_test.c -- Command line interface for ALSA tests

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
#include <blts_cli_frontend.h>
#include <blts_log.h>
#include "alsa_test_cases.h"
#include "alsa_config.h"
#include "alsa_util.h"

static alsa_cli_params* cli_params = NULL;
const char* default_config = BLTS_CONFIG_DIR "/blts-alsa-core-tests.cnf";

static void blts_alsa_help(const char* help_msg_base)
{
	fprintf(stdout, help_msg_base,
		"[-v] [-p]",
		"  -v: Verbose logging\n"
		"  -p: Enables profiling of ioctl calls\n"
		);
}

static void* blts_alsa_argument_processor(int argc, char **argv)
{
	BLTS_UNUSED_PARAM(argc)
	BLTS_UNUSED_PARAM(argv)
	return cli_params;
}

static blts_cli_testcase blts_alsa_cases[] =
{
	{ "ALSA presence check", alsa_presence_check, 10000 },
	{ "Open and close PCM devices", alsa_open_close_pcm, 20000 },
	{ "Open and close timer", alsa_open_close_timer, 20000 },
	{ "Open and close controls", alsa_open_close_controls, 20000 },
	{ "Open and close hwdeps", alsa_open_close_hwdeps, 20000 },
	{ "Enumerate control elements", alsa_enumerate_control_elements, 20000 },
	{ "Power management", alsa_power_management, 20000 },
	{ "Enumerate devices with control interface", alsa_enumerate_ctl_devices, 20000 },
	{ "Add and remove control element", alsa_add_remove_ctl_element, 20000 },
	BLTS_CLI_END_OF_LIST
};

static blts_cli alsa_cli =
{
	.test_cases = blts_alsa_cases,
	.log_file = "blts_alsa_core.txt",
	.blts_cli_help = blts_alsa_help,
	.blts_cli_process_arguments = blts_alsa_argument_processor,
	.blts_cli_teardown = NULL
};

static int generate_pcm_params(alsa_pcm_settings* pcms, pcm_params* params)
{
	int rates, formats, channels, accesses, asyncs;
	int cnt = 0;

	for(rates = 0; rates < pcms->num_rates; rates++)
	{
		for(formats = 0; formats < pcms->num_formats; formats++)
		{
			for(channels = 0; channels < pcms->num_channels; channels++)
			{
				for(accesses = 0; accesses < pcms->num_accesses; accesses++)
				{
					for(asyncs = 0; asyncs < pcms->num_asyncs; asyncs++)
					{
						if(params)
						{
							params[cnt].format = pcms->formats[formats];
							params[cnt].rate = pcms->rates[rates];
							params[cnt].channels = pcms->channels[channels];
							params[cnt].format = pcms->formats[formats];
							params[cnt].access = pcms->access[accesses];
							params[cnt].hw_resampling = pcms->hw_resampling;
							params[cnt].card = pcms->card;
							params[cnt].device = pcms->device;
							params[cnt].async = pcms->async[asyncs];
							params[cnt].dir = pcms->dir;
							params[cnt].freq = pcms->freq;
							params[cnt].duration = pcms->duration;
							params[cnt].link_card = pcms->link_card;
							params[cnt].link_device = pcms->link_device;
							params[cnt].period_size = pcms->period_size;
						}
						cnt++;
					}
				}
			}
		}
	}

	return cnt;
}

static testcase_params* generate_pcm_testcase_list(alsa_configuration* conf)
{
	int t, i, p, o, count, timeout;
	int ncases = 0;
	char *case_name;
	pcm_params *params[MAX_STREAMS];
	int param_count[MAX_STREAMS];
	int px[MAX_STREAMS];
	testcase_params* testcases = calloc(1, sizeof(testcase_params));
	blts_cli_testcase* cases = calloc(1, sizeof(blts_alsa_cases));

	memset(params, 0, sizeof(*params));

	/* Add static test cases to case list */
	while(blts_alsa_cases[ncases].case_name)
	{
		memcpy(&cases[ncases], &blts_alsa_cases[ncases], sizeof(blts_cli_testcase));
		ncases++;
	}

	/* go through every [testset] section */
	for(t = 0; t < conf->num_testsets; t++)
	{
		/* get parameter variations within single [playback]/[recording] sections */
		for(p = 0; p < conf->testsets[t]->num_pcms; p++)
		{
			param_count[p] = generate_pcm_params(conf->testsets[t]->pcms[p], NULL);
			if(!param_count[p])
			{

				if(conf->testsets[t]->pcms[p]->dir == STREAM_DIR_NONE)
				{
					param_count[p] = 1;
					params[p] = calloc(param_count[p], sizeof(pcm_params));
					params[p][0].duration = conf->testsets[t]->pcms[p]->duration;
					continue;
				}

				fprintf(stderr, "No test cases to execute, check configuration file.\n");
				goto error_exit;
			}
			params[p] = calloc(param_count[p], sizeof(pcm_params));
			param_count[p] = generate_pcm_params(conf->testsets[t]->pcms[p], params[p]);
		}

		/* figure out the number of test cases */
		count = param_count[0];
		for(p = 1; p < conf->testsets[t]->num_pcms; p++)
			count *= param_count[p];

		testcases = realloc(testcases, (ncases + count) * sizeof(testcase_params));
		cases = realloc(cases, (ncases + count) * sizeof(blts_cli_testcase));

		/* figure out parameter variations with multiple [playback]/[recording] sections */
		o = ncases;
		memset(px, 0, sizeof(px));
		for(i = 0; i < count; i++)
		{
			for(p = 0; p < conf->testsets[t]->num_pcms; p++)
			{
				testcases[o].params[p] = calloc(1, sizeof(pcm_params));
				memcpy(testcases[o].params[p], &params[p][px[p]], sizeof(pcm_params));
			}

			for(p = 0; p < conf->testsets[t]->num_pcms; p++)
			{
				if(++px[p] >= param_count[p])
					px[p] = 0;
				else
					break;
			}
			testcases[o++].num_pcm_params = conf->testsets[t]->num_pcms;
		}

		for(p = 0; p < conf->testsets[t]->num_pcms; p++)
			free(params[p]);

		/* generate test cases for cli */
		for(i = 0; i < count; i++)
		{
			case_name = malloc(256 * conf->testsets[t]->num_pcms);
			if(!case_name)
			{
				logged_perror("malloc failed");
				goto error_exit;
			}

			if(conf->testsets[t]->num_pcms == 1)
			{
				/* simple test case name when there is only one stream */
				if(testcases[ncases].params[0]->dir == STREAM_DIR_NONE)
				{
					sprintf(case_name, "PCM route %s",
						conf->testsets[t]->description);
				}
				else
				{
					sprintf(case_name, "PCM %s %s (%d:%d %d %d %s %s %s)",
						(testcases[ncases].params[0]->dir == STREAM_DIR_IN)?"recording":"playback",
						conf->testsets[t]->description,
						testcases[ncases].params[0]->card,
						testcases[ncases].params[0]->device,
						testcases[ncases].params[0]->rate,
						testcases[ncases].params[0]->channels,
						access_to_str(testcases[ncases].params[0]->access),
						format_to_str(testcases[ncases].params[0]->format),
						async_to_str(testcases[ncases].params[0]->async)
						);
				}
			}
			else
			{
				/* create test case name for tests with multiple streams */
				int play = 0, rec = 0;
				case_name[0] = 0;
				for(p = 0; p < conf->testsets[t]->num_pcms; p++)
				{
					if(testcases[ncases].params[p]->dir == STREAM_DIR_IN)
						rec = 1;
					if(testcases[ncases].params[p]->dir == STREAM_DIR_OUT)
						play = 1;
				}

				if(rec && play)
					sprintf(&case_name[strlen(case_name)], "PCM playback and recording");
				else if(rec)
					sprintf(&case_name[strlen(case_name)], "PCM recording");
				else
					sprintf(&case_name[strlen(case_name)], "PCM playback");

				sprintf(&case_name[strlen(case_name)], " %s", conf->testsets[t]->description);

				for(p = 0; p < conf->testsets[t]->num_pcms; p++)
				{
					sprintf(&case_name[strlen(case_name)], " (%s %d:%d %d %d %s %s %s)",
						(testcases[ncases].params[p]->dir == STREAM_DIR_IN)?"r":"p",
						testcases[ncases].params[p]->card,
						testcases[ncases].params[p]->device,
						testcases[ncases].params[p]->rate,
						testcases[ncases].params[p]->channels,
						access_to_str(testcases[ncases].params[p]->access),
						format_to_str(testcases[ncases].params[p]->format),
						async_to_str(testcases[ncases].params[p]->async)
						);
				}
			}

			/* Calculate timeout for the test */
			timeout = 0;
			for(p = 0; p < conf->testsets[t]->num_pcms; p++)
				if(testcases[ncases].params[p]->duration > timeout)
					timeout = testcases[ncases].params[p]->duration;
			timeout = (timeout * 2 + 5) * 1000;
			/* Add one minute for tuner */
			if(strlen(conf->testsets[t]->tuner.device) > 0)
				timeout += 60 * 1000;
			/* Add one minute for bluetooth (if only BT) */
			else if(strlen(conf->testsets[t]->btaudio.remote_mac) > 0)
				timeout += 60 * 1000;

			testcases[ncases].def_control_ref = conf->def_ctls;
			testcases[ncases].control_ref = conf->testsets[t]->ctls;
			memcpy(&testcases[ncases].tuner, &conf->testsets[t]->tuner,
				sizeof(v4l_tuner_settings));
			memcpy(&testcases[ncases].btaudio, &conf->testsets[t]->btaudio,
				sizeof(bluetooth_audio_settings));

			cases[ncases].case_name = case_name;
			cases[ncases].case_func = alsa_play_rec_pcm;
			cases[ncases].timeout = timeout;
			ncases++;
		}
	}

	/* Last case must be null */
	cases = realloc(cases, (ncases + 1) * sizeof(blts_cli_testcase));
	memset(&cases[ncases], 0, sizeof(blts_cli_testcase));

	alsa_cli.test_cases = cases;

	return testcases;

error_exit:
	if(testcases)
		free(testcases);

	return NULL;
}

int main(int argc, char **argv)
{
	int err, t, i;
	const char* config_file = default_config;

	cli_params = calloc(1, sizeof(alsa_cli_params));
	if(!cli_params)
	{
		fprintf(stderr, "Failed to allocate memory\n");
		return -1;
	}

	/* blts_cli_main opens log file, but we need to read conf file before
	 * calling it. Open the log file temporarily here to get output from
	 * conf file parser. */
	log_open(alsa_cli.log_file, 1);
	log_set_level(LEVEL_DEBUG);

	for(t = 1; t < argc; t++)
	{
		if(!strcmp(argv[t], "-C"))
		{
			if(++t < argc)
				config_file = argv[t];
		}
		else if(!strcmp(argv[t], "-v"))
		{
			log_set_level(LEVEL_TRACE);
			cli_params->flags |= CLI_FLAG_VERBOSE_LOG;
		}
		else if(!strcmp(argv[t], "-p"))
		{
			cli_params->flags |= CLI_FLAG_PROFILING;
		}
	}

        BLTS_DEBUG ("Using config file %s\n", config_file);

	if(alsa_read_config(config_file, &cli_params->config))
	{
		fprintf(stderr, "Failed to read config file %s\n", config_file);
		free(cli_params);
		return -1;
	}

	cli_params->testcases = generate_pcm_testcase_list(&cli_params->config);

	log_close();

	err =  blts_cli_main(&alsa_cli, argc, argv);

	/* free test case list if it was created dynamically */
	if(blts_alsa_cases != alsa_cli.test_cases)
	{
		t = ARRAY_SIZE(blts_alsa_cases) - 1;
		while(alsa_cli.test_cases[t].case_name)
		{
			free(alsa_cli.test_cases[t].case_name);
			for(i = 0; i < cli_params->testcases[t].num_pcm_params; i++)
				free(cli_params->testcases[t].params[i]);
			t++;
		}
		free(cli_params->testcases);
		free(alsa_cli.test_cases);
	}

	alsa_free_config(&cli_params->config);
	free(cli_params);

	return err;
}

