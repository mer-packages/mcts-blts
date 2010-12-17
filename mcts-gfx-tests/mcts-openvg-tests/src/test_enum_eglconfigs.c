/* test_enum_eglconfigs.c -- Enumerate EGL configurations

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
#include "openvg_helper.h"
#include "test_common.h"

typedef struct {
	EGLint value;
	char name[64];
} egl_config_attribs;

const egl_config_attribs conf_attribs[] = {
	{EGL_BUFFER_SIZE, "EGL_BUFFER_SIZE"},
	{EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE"},
	{EGL_BLUE_SIZE, "EGL_BLUE_SIZE"},
	{EGL_GREEN_SIZE, "EGL_GREEN_SIZE"},
	{EGL_RED_SIZE, "EGL_RED_SIZE"},
	{EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE"},
	{EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE"},
	{EGL_CONFIG_CAVEAT, "EGL_CONFIG_CAVEAT"},
	{EGL_CONFIG_ID, "EGL_CONFIG_ID"},
	{EGL_LEVEL, "EGL_LEVEL"},
	{EGL_MAX_PBUFFER_HEIGHT, "EGL_MAX_PBUFFER_HEIGHT"},
	{EGL_MAX_PBUFFER_PIXELS, "EGL_MAX_PBUFFER_PIXELS"},
	{EGL_MAX_PBUFFER_WIDTH, "EGL_MAX_PBUFFER_WIDTH"},
	{EGL_NATIVE_RENDERABLE, "EGL_NATIVE_RENDERABLE"},
	{EGL_NATIVE_VISUAL_ID, "EGL_NATIVE_VISUAL_ID"},
	{EGL_NATIVE_VISUAL_TYPE, "EGL_NATIVE_VISUAL_TYPE"},
#ifdef EGL_PRESERVED_RESOURCES
	{EGL_PRESERVED_RESOURCES, "EGL_PRESERVED_RESOURCES"},
#endif /* EGL_PRESERVED_RESOURCES */
	{EGL_SAMPLES, "EGL_SAMPLES"},
	{EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS"},
	{EGL_SURFACE_TYPE, "EGL_SURFACE_TYPE"},
	{EGL_TRANSPARENT_TYPE, "EGL_TRANSPARENT_TYPE"},
	{EGL_TRANSPARENT_BLUE_VALUE, "EGL_TRANSPARENT_BLUE_VALUE"},
	{EGL_TRANSPARENT_GREEN_VALUE, "EGL_TRANSPARENT_GREEN_VALUE"},
	{EGL_TRANSPARENT_RED_VALUE, "EGL_TRANSPARENT_RED_VALUE"},
	{EGL_BIND_TO_TEXTURE_RGB, "EGL_BIND_TO_TEXTURE_RGB"},
	{EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA"},
	{EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL"},
	{EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA"},
	{EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL"},
	{EGL_MAX_SWAP_INTERVAL, "EGL_MAX_SWAP_INTERVAL"},
	{EGL_LUMINANCE_SIZE, "EGL_LUMINANCE_SIZE"},
	{EGL_ALPHA_MASK_SIZE, "EGL_ALPHA_MASK_SIZE"},
	{EGL_COLOR_BUFFER_TYPE, "EGL_COLOR_BUFFER_TYPE"},
	{EGL_RENDERABLE_TYPE, "EGL_RENDERABLE_TYPE"},
	{EGL_CONFORMANT, "EGL_CONFORMANT"}
};

void config_contents(EGLDisplay display, EGLConfig * config)
{
	EGLint value;
	unsigned t;

	for (t = 0; t < ARRAY_SIZE(conf_attribs); t++) {
		if (!eglGetConfigAttrib
		    (display, *config, conf_attribs[t].value, &value)) {
			LOGERR("Failed to get config attribute '%s'\n",
			       conf_attribs[t].name);
		} else {
			LOG("%s: %d\n", conf_attribs[t].name, value);
		}
	}
}

int test_enum_eglconfigs(test_execution_params * params)
{
	vgh_context context;
	EGLConfig configs[256];
	EGLint config_size = sizeof(configs);
	EGLint num_configs;
	int t;
	int ret = 0;

	if (!vgh_create_context
	    (&context, NULL, params->w, params->h, params->d)) {
		LOGERR("vgh_create_context failed!\n");
		return -1;
	}

	if (!eglGetConfigs
	    (context.egl_display, configs, config_size, &num_configs)) {
		LOGERR("Failed to EGL configurations\n");
		vgh_report_eglerror("eglGetConfigs");
		ret = -1;
	} else {
		LOG("Found %d EGL configurations\n", num_configs);
		for (t = 0; t < num_configs; t++) {
			LOG("\nContents of configuration %d:\n", t);
			config_contents(context.egl_display, &configs[t]);
		}
	}

	sleep(1);

	vgh_destroy_context(&context);

	return ret;
}
