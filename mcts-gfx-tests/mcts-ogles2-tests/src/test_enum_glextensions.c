/* test_enum_glextensions.c -- Enumerate GL extensions

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
#include <string.h>
#include <unistd.h>
#include "ogles2_helper.h"
#include "test_common.h"

static const char* gl_ext_list[] =
{
	"GL_OES_depth24",
	"GL_OES_rgb8_rgba8",
	"GL_OES_texture_float",
	"GL_OES_mapbuffer",
	"GL_OES_fragment_precision_high",
	"GL_OES_EGL_image",
	NULL
};

static int check_extensions(char* all_extensions, const char *ext_name)
{
	char *end;
	int ext_name_len;

	ext_name_len = strlen(ext_name);
	end = all_extensions + strlen(all_extensions);

	while(all_extensions < end)
	{
		int n = strcspn(all_extensions, " ");
		if((ext_name_len == n) && (strncmp(ext_name, all_extensions, n) == 0))
		{
			return 0;
		}

		all_extensions += (n + 1);
	}

	return -1;
}

int test_enum_glextensions(test_execution_params* params)
{
	glesh_context context;
	char* query_str;
	char* end;
	char* p;
	char ext_name[256];
	int t;
	int missing_extensions = 0;
	int ret = -1;

	if(!glesh_create_context(&context, NULL, params->w, params->h, params->d))
	{
		BLTS_ERROR("glesh_create_context failed!\n");
		return -1;
	}

	query_str = (char*)glGetString(GL_VERSION);
	if(!query_str)
	{
		BLTS_ERROR("Failed to get GL_VERSION\n");
		return -1;
	}
	BLTS_DEBUG("GL_VERSION: %s\n", query_str);

	query_str = (char*)glGetString(GL_RENDERER);
	if(!query_str)
	{
		BLTS_ERROR("Failed to get GL_RENDERER\n");
		return -1;
	}
	BLTS_DEBUG("GL_RENDERER: %s\n", query_str);

	query_str = (char*)glGetString(GL_EXTENSIONS);
	if(!query_str)
	{
		BLTS_ERROR("Failed to get list of GL extensions\n");
		return -1;
	}

	BLTS_DEBUG("List of found extensions:\n");

	end = query_str + strlen(query_str);
	p = query_str;
	while(p < end)
	{
		int n = strcspn(p, " ");
		if(n > 0)
		{
			memcpy(ext_name, p, n);
			ext_name[n] = 0;
			BLTS_DEBUG("%s\n", ext_name);
		}

		p += (n + 1);
	}

	BLTS_DEBUG("end of list\n");

	t = 0;
	while(gl_ext_list[t])
	{
		if(check_extensions(query_str, gl_ext_list[t]))
		{
			BLTS_ERROR("Required extension '%s' missing.\n", gl_ext_list[t]);
			missing_extensions++;
		}
		t++;
	}
	if(!missing_extensions)
	{
		BLTS_DEBUG("All required extensions found.\n");
		ret = 0;
	}

	sleep(1);

	glesh_destroy_context(&context);

	return ret;
}

