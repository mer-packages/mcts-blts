/*
 Copyright (C) 2010 Intel Corporation
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 test_enum_eglextensions.c -- Enumerate EGL extensions
 Author: 
     Zheng Kui (kui.zheng@intel.com)
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "openvg_helper.h"
#include "test_common.h"

/* TODO: This list is incomplete. Required extensions are not known yet. */
static const char *egl_ext_list[] = {
	"texture_from_pixmap",
	NULL
};

static int check_extensions(char *all_extensions, const char *ext_name)
{
	if (strstr(all_extensions, ext_name)) {
		return 0;
	}

	return -1;
}

int test_enum_eglextensions(test_execution_params * params)
{
	vgh_context context;
	char *query_str;
	char *end;
	char *p;
	char ext_name[256];
	int t;
	int missing_extensions = 0;
	int ret = -1;

	if (!vgh_create_context
	    (&context, NULL, params->w, params->h, params->d)) {
		LOGERR("vgh_create_context failed!\n");
		return -1;
	}

	query_str = (char *)eglQueryString(context.egl_display, EGL_VERSION);
	if (!query_str) {
		LOGERR("Failed to get EGL_VERSION)\n");
		return -1;
	}
	LOG("EGL_VERSION: %s\n", query_str);

	query_str =
	    (char *)eglQueryString(context.egl_display, EGL_CLIENT_APIS);
	if (!query_str) {
		LOGERR("Failed to get EGL_CLIENT_APIS\n");
		return -1;
	}
	LOG("EGL_CLIENT_APIS: %s\n", query_str);

	query_str = (char *)eglQueryString(context.egl_display, EGL_EXTENSIONS);
	if (!query_str) {
		LOGERR("Failed to get list of EGL extensions\n");
		return -1;
	}

	LOG("List of found extensions:\n");

	end = query_str + strlen(query_str);
	p = query_str;
	while (p < end) {
		int n = strcspn(p, " ");
		if (n > 0) {
			memcpy(ext_name, p, n);
			ext_name[n] = 0;
			LOG("%s\n", ext_name);
		}

		p += (n + 1);
	}

	LOG("end of list\n");

	t = 0;
	while (egl_ext_list[t]) {
		if (check_extensions(query_str, egl_ext_list[t])) {
			LOGERR("Required extension '%s' missing.\n",
			       egl_ext_list[t]);
			missing_extensions++;
		}
		t++;
	}
	if (!missing_extensions) {
		LOG("All required extensions found.\n");
		ret = 0;
	}
	sleep(1);

	vgh_destroy_context(&context);

	return ret;
}
