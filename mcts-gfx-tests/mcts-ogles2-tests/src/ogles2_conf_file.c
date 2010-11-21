/* ogles2_conf_file.c -- Configuration file parser

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
#include "ogles2_helper.h"
#include "ogles2_conf_file.h"

#undef LOGTRACE
#define LOGTRACE(...)

test_configuration_file_params* current_test_config;

/* Maximum line length in config file */
#define MAX_LINE_LEN 256

#define VAL_TYPE_INT 0
#define VAL_TYPE_FLOAT 1

static int cnfparser_remove_data(char* buf, unsigned int startpos,
	unsigned int len)
{
	unsigned int blen = strlen(buf);

	if(startpos >= blen || len > blen)
	{
		return -1;
	}

	memcpy(&buf[startpos], &buf[startpos + len], blen - len - startpos + 1);

	return 0;
}

static int cnfparser_readline(const char* buf, char* line)
{
	int len;
	char* ptr = strchr(buf, '\n');
	if(!ptr)
	{
		return strlen(buf);
	}

	/* Truncate long lines */
	len = GLESH_MIN((int)(ptr - buf), MAX_LINE_LEN);

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
	unsigned int len;
	int in_str;
	char* ptr;
	char* ptr_o;
	*out = malloc(strlen(in) + 1);
	ptr_o = *out;
	memcpy(ptr_o, in, strlen(in));
	ptr_o[strlen(in)] = 0;

	LOGTRACE("Cleaning out tabs, spaces etc.\n");

	/* Remove \r, \t, ' ' */
	in_str = 0;
	for(t = 0; t < strlen(ptr_o); t++)
	{
		if((ptr_o)[t] == '\"')
		{
			in_str = !in_str;
		}
		if(!in_str)
		{
			if(ptr_o[t] == '\r' || ptr_o[t] == '\t'  || ptr_o[t] == ' ')
			{
				cnfparser_remove_data(ptr_o, t, 1);
				t--;
			}
		}
	}

	LOGTRACE("Cleaning out comments and empty lines\n");

	/* Remove comments & empty lines */
	ptr = ptr_o;
	while(ptr < ptr_o + strlen(ptr_o))
	{
		char line[MAX_LINE_LEN];
		len = cnfparser_readline(ptr, line);
		if(len == 0)
		{
			cnfparser_remove_data(ptr, 0, 1);
		}
		else if(*ptr == '#')
		{
			cnfparser_remove_data(ptr, 0, len);
		}
		else
		{
			ptr += len + 1;
		}
	}

	ptr = ptr_o;
	while(ptr < ptr_o + strlen(ptr_o))
	{
		char line[MAX_LINE_LEN];
		len = cnfparser_readline(ptr, line);
		if(ptr[len-1] == '\\')
		{
			cnfparser_remove_data(&ptr[len-1], 0, 2);
		}
		else
		{
			ptr += len + 1;
		}
	}

	return 0;
}

/* return -1 in case of an error, 0 if success, 1 if 'name' was not found */
static int cnfparser_read_str(const char* start, const char* end,
	const char* name, char* str)
{
	int len;
	char line[MAX_LINE_LEN];
	char param_name[64];
	const char* ptr = start;
	char* lp;

	while(ptr < end)
	{
		len = cnfparser_readline(ptr, line);
		ptr += len + 1;
		lp = strchr(line, ':');
		if(lp)
		{
			memcpy(param_name, line, lp - line);
			param_name[lp - line] = 0;
			if(!strcmp(param_name, name))
			{
				char* start_ptr = strchr(line, '\"');
				if(!start_ptr)
				{
					BLTS_ERROR("Missing '\"' in line %s\n", line);
					return -1;
				}
				char* end_ptr = strchr(start_ptr + 1, '\"');
				if(!end_ptr)
				{
					BLTS_ERROR("Missing '\"' in line %s\n", line);
					return -1;
				}

				memcpy(str, start_ptr + 1, end_ptr - start_ptr - 1);
				str[end_ptr - start_ptr - 1] = 0;

				LOGTRACE("%s: %s\n", name, str);
				return 0;
			}
		}
	}

	return 1;
}

static int cnfparser_read_val(const char* start, const char* end,
	const char* name, void* val, int type)
{
	int len;
	char line[MAX_LINE_LEN];
	char param_name[64];
	const char* ptr = start;
	char* lp;

	if(!val || !name)
	{
		BLTS_ERROR("NULL pointer in cnfparser_read_int\n");
		return -1;
	}

	while(ptr < end)
	{
		len = cnfparser_readline(ptr, line);
		ptr += len + 1;
		lp = strchr(line, ':');
		if(lp)
		{
			memcpy(param_name, line, lp - line);
			param_name[lp - line] = 0;
			if(!strcmp(param_name, name))
			{
				char* p = line + strlen(line) - 1;
				while(isdigit(*p) && p > line) p--;
				p++;
				if(type == VAL_TYPE_INT)
				{
					*((int*)val) = atoi(p);
				}
				else if(type == VAL_TYPE_FLOAT)
				{
					*((float*)val) = atof(p);
				}
				else
				{
					BLTS_ERROR("Unknown value type %d\n", type);
				}
				LOGTRACE("%s: %s\n", name, p);
				return 0;
			}
		}
	}

	return 1;
}

static int cnfparser_parse_matrix(float* mat, const char* str)
{
	char tmp[8];
	unsigned int  t, p, i;

	i = 0; p = 0;
	for(t = 0; t < strlen(str); t++)
	{
		if(str[t] == ',')
		{
			mat[p++] = atof(tmp);
			i = 0;
		}
		else
		{
			if(i >= 8) return 0;
			tmp[i++] = str[t];
			tmp[i] = 0;
		}
	}

	if(i) mat[p++] = atoi(tmp);
	mat[p] = 0;
	return p;
}

static int cnfparser_parse_config(char* start, char* end,
	test_configuration_file_params* config)
{
	char line[MAX_LINE_LEN];

	if(cnfparser_read_val(start, end, "scale_images_to_window",
		(void*)&config->scale_images_to_window, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "video_widget_generation_freq",
		(void*)&config->video_widget_generation_freq, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "video_widget_texture_width",
		(void*)&config->video_widget_tex_width, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "video_widget_texture_height",
		(void*)&config->video_widget_tex_height, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "desktop_count",
		(void*)&config->desktop_count, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "layer_count",
		(void*)&config->layer_count, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "widget_count",
		(void*)&config->widget_count, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "particle_count",
		(void*)&config->particle_count, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_val(start, end, "scroll_speed",
		(void*)&config->scroll_speed, VAL_TYPE_INT) < 0) return -1;
	if(cnfparser_read_str(start, end, "compositor_cmd",
		config->compositor_cmd) < 0) return -1;
	if(cnfparser_read_str(start, end, "convolution_mat", line) < 0) return -1;
	config->convolution_mat_size = cnfparser_parse_matrix(
		config->convolution_mat, line);
	if(cnfparser_read_val(start, end, "convolution_mat_divisor",
		(void*)&config->convolution_mat_divisor, VAL_TYPE_FLOAT) < 0) return -1;

	return 0;
}

static int cnfparser_parse_sections(char* buf)
{
	char* section_start;
	char* section_end;

	section_start = buf;
	section_end = buf + strlen(buf);

	return cnfparser_parse_config(section_start, section_end,
		current_test_config);
}

int read_config(const char* filename,
	test_configuration_file_params* test_config)
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

	buf = malloc((len + 1) * sizeof(char));
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

	buf[len] = 0;

	LOGTRACE("Read %d bytes from file %s\n", len, filename);

	if(cnfparser_cleanup(buf, &clean_buf))
	{
		BLTS_ERROR("Config file parsing failed\n");
		free(buf);
		free(clean_buf);
		return 1;
	}
	free(buf);

	current_test_config = test_config;

	LOGTRACE("Parsing sections...\n");
	if(cnfparser_parse_sections(clean_buf))
	{
		BLTS_ERROR("Config file parsing failed\n");
		free(clean_buf);
		return 1;
	}

	free(clean_buf);

	return 0;
}

