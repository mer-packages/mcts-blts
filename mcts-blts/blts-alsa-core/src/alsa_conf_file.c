/* alsa_conf_file.c -- configuration file parser

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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "alsa_util.h"
#include "alsa_config.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define CNF_TOKEN_STR 0
#define CNF_TOKEN_INT 1

typedef struct {
	int type;
	int value;
	char str[MAX_LINE_LEN];
} cnf_token;

static alsa_configuration* g_config;

static int cnfparser_remove_data(char* buf, unsigned int startpos, unsigned int len)
{
	unsigned int blen = strlen(buf);
	char *tmp;
	if(startpos >= blen || len > blen)
		return -1;

	tmp = calloc(1, blen + 1);
	if(!tmp)
	{
		BLTS_LOGGED_PERROR("calloc");
		return -ENOMEM;
	}
	memcpy(tmp, buf, blen);
	memcpy(&buf[startpos], &tmp[startpos + len], blen - len - startpos + 1);
	free(tmp);
	return 0;
}

static int cnfparser_readline(const char* buf, char* line)
{
	int len;
	char* ptr = strchr(buf, '\n');
	if(!ptr)
		return strlen(buf);

	/* Truncate long lines */
	len = MIN((int)(ptr - buf), MAX_LINE_LEN);

	if(line)
	{
		memcpy(line, buf, len);
		line[len] = 0;
	}

	return len;
}

static int cnfparser_cleanup(const char* in, char** out)
{
	unsigned int t;
	unsigned int len = strlen(in) + 1;
	int in_str;
	char* ptr;
	char* ptr_o;

	*out = malloc(len);
	if(!*out)
	{
		BLTS_LOGGED_PERROR("malloc");
		return -1;
	}

	ptr_o = *out;
	memcpy(ptr_o, in, len);

	BLTS_TRACE("Cleaning out tabs, spaces etc.\n");

	/* Remove \r, \t */
	in_str = 0;
	for(t = 0; t < strlen(ptr_o); t++)
	{
		if((ptr_o)[t] == '\"')
			in_str = !in_str;
		if(!in_str)
		{
			if(ptr_o[t] == '\r')
			{
				cnfparser_remove_data(ptr_o, t, 1);
				t--;
			}
			else if(ptr_o[t] == '\t')
				ptr_o[t] = ' ';
		}
	}

	/* Remove leading spaces */
	ptr = ptr_o;
	while(ptr < ptr_o + strlen(ptr_o))
	{
		if(*ptr == ' ')
		{
			cnfparser_remove_data(ptr--, 0, 1);
		}
		else
		{
			ptr = strchr(ptr, '\n');
			if(!ptr)
				break;
		}
		ptr++;
	}

	BLTS_TRACE("Cleaning out comments and empty lines\n");

	/* Remove comments & empty lines */
	ptr = ptr_o;
	while(ptr < ptr_o + strlen(ptr_o))
	{
		char line[MAX_LINE_LEN];
		len = cnfparser_readline(ptr, line);
		if(len == 0)
			cnfparser_remove_data(ptr, 0, 1);
		else if(*ptr == '#')
			cnfparser_remove_data(ptr, 0, len);
		else
			ptr += len + 1;
	}

	return 0;
}

static char* cnfparser_seek_section_end(char* ptr, const char* end_ptr,
	const char* tag)
{
	char line[MAX_LINE_LEN];
	while(ptr < end_ptr)
	{
		int len = cnfparser_readline(ptr, line);
		char* section_end = ptr;
		if(!strcmp(line, tag))
			return section_end;

		ptr += len + 1;
	}

	return NULL;
}

static char* next_token(char* str, cnf_token* token)
{
	int len = strlen(str);
	char* ptr = str;
	int start = 0;
	int pos = 0;
	int tlen = 0;
	int in_str = 0;

	token->type = CNF_TOKEN_INT;
	memset(token->str, 0, MAX_LINE_LEN);
	while(pos++ < len)
	{
		if(*ptr == '\"')
		{
			if(in_str)
				in_str = 0;
			else
				in_str = 1;
			start = 1;
		}

		if(!in_str)
		{
			if((*ptr == ' ' || *ptr == '\n') && start)
			{
				if(token->type == CNF_TOKEN_INT)
					token->value = atoi(token->str);
				return ptr;
			}
			else if(*ptr != ' ')
				start = 1;
		}

		if(start)
		{
			if(!isdigit(*ptr))
				token->type = CNF_TOKEN_STR;
			if(*ptr != '\"')
				token->str[tlen++] = *ptr;
		}
		ptr++;
	}

	if(start)
	{
		if(token->type == CNF_TOKEN_INT)
			token->value = atoi(token->str);
		return ptr;
	}

	return NULL;
}

static int cnfparser_parse_control(char* start, char* end,
	alsa_control_settings** params)
{
	char line[MAX_LINE_LEN];
	cnf_token token;
	int len;
	char* ptr;
	alsa_control_settings* current_config;

	current_config = calloc(1, sizeof(alsa_control_settings));
	if(!current_config)
	{
		BLTS_LOGGED_PERROR("calloc");
		return -1;
	}

	*params = current_config;

	while(start < end)
	{
		len = cnfparser_readline(start, line);
		ptr = next_token(line, &token);
		if(!ptr || token.type != CNF_TOKEN_STR)
			goto error_exit;

		if(!strncmp(token.str, "card", 4))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			current_config->card = token.value;
		}
		else if(!strncmp(token.str, "set", 3))
		{
			alsa_control_value* ctl =
				&current_config->values[current_config->num_values++];
			ptr = next_token(ptr, &token);
			if(token.type == CNF_TOKEN_STR)
				strcpy(ctl->control_name, token.str);
			else if(token.type == CNF_TOKEN_INT)
				ctl->control_id = token.value;

			ptr = next_token(ptr, &token);
			if(token.type == CNF_TOKEN_STR)
				strcpy(ctl->str_value, token.str);
			else if(token.type == CNF_TOKEN_INT)
				ctl->value = token.value;
		}
		else
			goto error_exit;

		start += len + 1;
	}

	return 0;

error_exit:
	BLTS_ERROR("Syntax error in configuration line '%s'\n", line);
	return -1;
}

static int cnfparser_parse_pcm(char* start, char* end, alsa_pcm_settings* params)
{
	char line[MAX_LINE_LEN];
	cnf_token token;
	int len;
	char* ptr;

	params->link_card = -1;
	params->link_device = -1;
	params->period_size = -1;
	params->volume = 100;

	while(start < end)
	{
		len = cnfparser_readline(start, line);
		ptr = next_token(line, &token);
		if(!ptr || token.type != CNF_TOKEN_STR)
			goto error_exit;

		if(!strncmp(token.str, "card", 4))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->card = token.value;
		}
		else if(!strncmp(token.str, "device", 6))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->device = token.value;
		}
		else if(!strncmp(token.str, "hw_resampling", 13))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->hw_resampling = token.value;
		}
		else if(!strncmp(token.str, "channels", 8))
		{
			while((ptr = next_token(ptr, &token)))
			{
				if(token.type != CNF_TOKEN_INT)
					goto error_exit;
				params->channels[params->num_channels++] = token.value;
			}
		}
		else if(!strncmp(token.str, "rates", 5))
		{
			while((ptr = next_token(ptr, &token)))
			{
				if(token.type != CNF_TOKEN_INT)
					goto error_exit;
				params->rates[params->num_rates++] = token.value;
			}
		}
		else if(!strncmp(token.str, "formats", 7))
		{
			while((ptr = next_token(ptr, &token)))
			{
				if(token.type == CNF_TOKEN_INT)
				{
					params->formats[params->num_formats++] = token.value;
				}
				else if(token.type == CNF_TOKEN_STR)
				{
					params->formats[params->num_formats] = str_to_format(token.str);
					if(params->formats[params->num_formats] < 0)
						goto error_exit;
					params->num_formats++;
				}
			}
		}
		else if(!strncmp(token.str, "access", 6))
		{
			while((ptr = next_token(ptr, &token)))
			{
				if(token.type == CNF_TOKEN_INT)
				{
					params->access[params->num_accesses++] = token.value;
				}
				else if(token.type == CNF_TOKEN_STR)
				{
					params->access[params->num_accesses] = str_to_access(token.str);
					if(params->access[params->num_accesses] < 0)
						goto error_exit;
					params->num_accesses++;
				}
			}
		}
		else if(!strncmp(token.str, "async", 5))
		{
			while((ptr = next_token(ptr, &token)))
			{
				if(token.type == CNF_TOKEN_INT)
				{
					params->async[params->num_asyncs++] = token.value;
				}
				else if(token.type == CNF_TOKEN_STR)
				{
					params->async[params->num_asyncs] = str_to_async(token.str);
					if(params->async[params->num_asyncs] < 0)
						goto error_exit;
					params->num_asyncs++;
				}
			}
		}
		else if(!strncmp(token.str, "duration", 8))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->duration = token.value;
		}
		else if(!strncmp(token.str, "freq", 4))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->freq = token.value;
		}
		else if(!strncmp(token.str, "link", 4))
		{
			ptr = next_token(ptr, &token);
			if(token.type != CNF_TOKEN_INT)
				goto error_exit;
			params->link_card = token.value;
			ptr = next_token(ptr, &token);
			if(token.type != CNF_TOKEN_INT)
				goto error_exit;
			params->link_device = token.value;
		}
		else if(!strncmp(token.str, "period_size", 4))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->period_size = token.value;
		}
		else if(!strncmp(token.str, "volume", 6))
		{
			ptr = next_token(ptr, &token);
			if(!ptr)
				goto error_exit;
			params->volume = token.value;
		}
		else
			goto error_exit;

		start += len + 1;
	}

	return 0;

error_exit:
	BLTS_ERROR("Syntax error in configuration line '%s'\n", line);
	return -1;
}

static int cnfparser_parse_tuner(char* start, char* end, alsa_testset* params)
{
	char line[MAX_LINE_LEN];
	cnf_token token;
	int len;
	char* ptr;

	params->tuner.scan = -1;

	while(start < end)
	{
		len = cnfparser_readline(start, line);
		ptr = next_token(line, &token);
		if(!ptr || token.type != CNF_TOKEN_STR)
			goto error_exit;

		if(!strcmp(token.str, "device"))
		{
			ptr = next_token(ptr, &token);
			if(!ptr || token.type != CNF_TOKEN_STR)
				goto error_exit;
			strcpy(params->tuner.device, token.str);
		}
		else if(!strcmp(token.str, "freq"))
		{
			ptr = next_token(ptr, &token);
			if(token.type != CNF_TOKEN_INT)
				goto error_exit;
			params->tuner.freq = token.value;
		}
		else if(!strcmp(token.str, "scan_select"))
		{
			ptr = next_token(ptr, &token);
			if(token.type != CNF_TOKEN_INT)
				goto error_exit;
			params->tuner.scan = token.value;
		}
		else
			goto error_exit;

		start += len + 1;
	}

	return 0;

error_exit:
	BLTS_ERROR("Syntax error in configuration line '%s'\n", line);
	return -1;
}

static int cnfparser_parse_bluetooth(char* start, char* end,
	alsa_testset* params)
{
	char line[MAX_LINE_LEN];
	cnf_token token;
	int len;
	char* ptr;

	while(start < end)
	{
		len = cnfparser_readline(start, line);
		ptr = next_token(line, &token);
		if(!ptr || token.type != CNF_TOKEN_STR)
			goto error_exit;

		if(!strcmp(token.str, "connect"))
		{
			ptr = next_token(ptr, &token);
			if(!ptr || token.type != CNF_TOKEN_STR)
				goto error_exit;
			strcpy(params->btaudio.remote_mac, token.str);
		}
		else if(!strcmp(token.str, "pin"))
		{
			ptr = next_token(ptr, &token);
			if(!ptr || token.type != CNF_TOKEN_STR)
				goto error_exit;
			strcpy(params->btaudio.pin, token.str);
		}
		else
			goto error_exit;

		start += len + 1;
	}

	return 0;

error_exit:
	BLTS_ERROR("Syntax error in configuration line '%s'\n", line);
	return -1;
}

static int cnfparser_parse_config_section(alsa_testset* current_testset,
	char* start, char* end)
{
	alsa_pcm_settings* current_config;

	if(current_testset->num_pcms >= MAX_STREAMS)
		return -1;

	current_config = calloc(1, sizeof(alsa_pcm_settings));
	if(!current_config)
	{
		BLTS_LOGGED_PERROR("calloc");
		return -1;
	}

	current_testset->pcms[current_testset->num_pcms++] = current_config;

	if(cnfparser_parse_pcm(start, end, current_config))
		return -1;

	return 0;
}

static int cnfparser_parse_testset_section(char* start, char* end)
{
	int len;
	char line[MAX_LINE_LEN];
	char* section_start = start;
	char* section_end;
	char* tmp;
	cnf_token token;

	alsa_testset* current_testset = calloc(1, sizeof(alsa_testset));
	if(!current_testset)
	{
		BLTS_LOGGED_PERROR("calloc");
		return -1;
	}

	g_config->testsets[g_config->num_testsets++] = current_testset;

	while(section_start < end)
	{
		len = cnfparser_readline(section_start, line);
		section_start += len + 1;
		if(!strcmp(line, "[playback]"))
		{
			BLTS_TRACE("Found [playback] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end,
				"[end_playback]");
			if(section_end)
			{
				if(cnfparser_parse_config_section(current_testset, section_start,
					section_end))
				{
					BLTS_ERROR("Failed to parse playback section\n");
					return -1;
				}
				current_testset->pcms[current_testset->num_pcms - 1]->dir = STREAM_DIR_OUT;
				BLTS_TRACE("[playback] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [playback] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[recording]"))
		{
			BLTS_TRACE("Found [recording] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end,
				"[end_recording]");
			if(section_end)
			{
				if(cnfparser_parse_config_section(current_testset, section_start,
					section_end))
				{
					BLTS_ERROR("Failed to parse recording section\n");
					return -1;
				}
				current_testset->pcms[current_testset->num_pcms - 1]->dir = STREAM_DIR_IN;
				BLTS_TRACE("[recording] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [recording] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[control]"))
		{
			BLTS_TRACE("Found [control] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end, "[end_control]");
			if(section_end)
			{
				if(current_testset->num_ctls >= MAX_STREAMS)
				{
					BLTS_ERROR("Too many control sections\n");
					return -1;
				}

				if(cnfparser_parse_control(section_start, section_end,
					&current_testset->ctls[current_testset->num_ctls++]))
				{
					BLTS_ERROR("Failed to parse control section\n");
					return -1;
				}
				BLTS_TRACE("[control] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [control] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[wait]"))
		{
			BLTS_TRACE("Found [wait] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end, "[end_wait]");
			if(section_end)
			{
				if(cnfparser_parse_config_section(current_testset, section_start,
					section_end))
				{
					BLTS_ERROR("Failed to parse wait section\n");
					return -1;
				}
				current_testset->pcms[current_testset->num_pcms - 1]->dir = STREAM_DIR_NONE;
				BLTS_TRACE("[wait] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [wait] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[tuner]"))
		{
			BLTS_TRACE("Found [tuner] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end, "[end_tuner]");
			if(section_end)
			{
				if(cnfparser_parse_tuner(section_start, section_end, current_testset))
				{
					BLTS_ERROR("Failed to parse tuner section\n");
					return -1;
				}
				BLTS_TRACE("[tuner] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [control] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[bluetooth]"))
		{
			BLTS_TRACE("Found [bluetooth] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end,
				"[end_bluetooth]");
			if(section_end)
			{
				if(cnfparser_parse_bluetooth(section_start, section_end,
					current_testset))
				{
					BLTS_ERROR("Failed to parse bluetooth section\n");
					return -1;
				}
				BLTS_TRACE("[bluetooth] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [bluetooth] found\n");
			}
			section_start = section_end;
		}

		else
		{
			tmp = next_token(line, &token);
			if(tmp && token.type == CNF_TOKEN_STR)
			{
				if(!strncmp(token.str, "description", 11))
				{
					tmp = next_token(tmp, &token);
					if(!tmp || token.type != CNF_TOKEN_STR)
						return -1;
					strcpy(current_testset->description, token.str);
				}
			}
		}
	}

	return 0;
}

static int cnfparser_parse_defaults_section(char* start, char* end)
{
	int len;
	char line[MAX_LINE_LEN];
	char* section_start = start;
	char* section_end;

	while(section_start < end)
	{
		len = cnfparser_readline(section_start, line);
		section_start += len + 1;

		if(!strcmp(line, "[control]"))
		{
			BLTS_TRACE("Found [control] section...\n");
			section_end = cnfparser_seek_section_end(section_start, end,
				"[end_control]");
			if(section_end)
			{
				if(g_config->num_def_ctls >= MAX_STREAMS)
				{
					BLTS_ERROR("Too many control sections\n");
					return -1;
				}

				if(cnfparser_parse_control(section_start, section_end,
					&g_config->def_ctls[g_config->num_def_ctls++]))
				{
					BLTS_ERROR("Failed to parse control section\n");
					return -1;
				}
				BLTS_TRACE("[control] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [control] found\n");
			}
			section_start = section_end;
		}
	}

	return 0;
}

static int cnfparser_parse_sections(char* buf)
{
	int len;
	char line[MAX_LINE_LEN];
	char* section_start;
	char* section_end;

	section_start = buf;

	while(section_start < buf + strlen(buf))
	{
		len = cnfparser_readline(section_start, line);
		section_start += len + 1;
		if(!strcmp(line, "[testset]"))
		{
			BLTS_TRACE("Found [testset] section...\n");
			section_end = cnfparser_seek_section_end(section_start,
				buf + strlen(buf), "[end_testset]");
			if(section_end)
			{
				if(cnfparser_parse_testset_section(section_start, section_end))
				{
					BLTS_ERROR("Failed to parse testset section\n");
					return -1;
				}
				BLTS_TRACE("[testset] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [testset] found\n");
			}
			section_start = section_end;
		}
		else if(!strcmp(line, "[defaults]"))
		{
			BLTS_TRACE("Found [defaults] section...\n");
			section_end = cnfparser_seek_section_end(section_start,
				buf + strlen(buf), "[end_defaults]");
			if(section_end)
			{
				if(cnfparser_parse_defaults_section(section_start, section_end))
				{
					BLTS_ERROR("Failed to parse defaults section\n");
					return -1;
				}
				BLTS_TRACE("[defaults] section parsed ok\n");
			}
			else
			{
				BLTS_ERROR("No end tag for [defaults] found\n");
			}
			section_start = section_end;
		}
	}

	return 0;
}

int alsa_read_config(const char* filename, alsa_configuration* config)
{
	FILE* fp;
	unsigned int len;
	char* buf;
	char* clean_buf = NULL;

	if(!(fp = fopen(filename, "rb")))
	{
		BLTS_ERROR("Failed to open config file\n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = calloc(1, (len + 1));
	if(!buf)
	{
		BLTS_ERROR("Failed to allocate buffer\n");
		fclose(fp);
		return -1;
	}

	if(fread(buf, 1, len, fp) != len)
	{
		BLTS_ERROR("Failed to read config file\n");
		fclose(fp);
		free(buf);
		return -1;
	}
	fclose(fp);

	BLTS_TRACE("Read %d bytes from file %s\n", len, filename);

	if(cnfparser_cleanup(buf, &clean_buf))
	{
		BLTS_ERROR("Config file parsing failed\n");
		free(buf);
		free(clean_buf);
		return 1;
	}
	free(buf);

	memset(config, 0, sizeof(alsa_configuration));
	g_config = config;

	BLTS_TRACE("Parsing sections...\n");
	if(cnfparser_parse_sections(clean_buf))
	{
		BLTS_ERROR("Config file parsing failed\n");
		free(clean_buf);
		return 1;
	}

	free(clean_buf);

	return 0;
}

int alsa_free_config(alsa_configuration* config)
{
	int t, i;
	if(!config)
		return -EINVAL;

	for(t = 0; t < config->num_testsets; t++)
	{
		for(i = 0; i < config->testsets[t]->num_pcms; i++)
			free(config->testsets[t]->pcms[i]);
		for(i = 0; i < config->testsets[t]->num_ctls; i++)
			free(config->testsets[t]->ctls[i]);
		free(config->testsets[t]);
		config->testsets[t] = NULL;
	}

	for(i = 0; i < config->num_def_ctls; i++)
		free(config->def_ctls[i]);

	config->num_testsets = 0;

	return 0;
}

