/* openvg_helper.c -- Various helper functions for OpenVG

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

/*  
 *  kui.zheng@intel.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <X11/Xutil.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <errno.h>
#include "openvg_helper.h"

static const EGLint default_config_attr[] = {
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
	EGL_NONE
};

static double time_step;

const char *vgh_egl_error_to_string(EGLint err)
{
	switch (err) {
	case EGL_SUCCESS:
		return "EGL_SUCCESS";
	case EGL_BAD_DISPLAY:
		return "EGL_BAD_DISPLAY";
	case EGL_NOT_INITIALIZED:
		return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONFIG:
		return "EGL_BAD_CONFIG";
	case EGL_BAD_CONTEXT:
		return "EGL_BAD_CONTEXT";
	case EGL_BAD_CURRENT_SURFACE:
		return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_MATCH:
		return "EGL_BAD_MATCH";
	case EGL_BAD_NATIVE_PIXMAP:
		return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return "EGL_BAD_NATIVE_WINDOW";
	case EGL_BAD_PARAMETER:
		return "EGL_BAD_PARAMETER";
	case EGL_BAD_SURFACE:
		return "EGL_BAD_SURFACE";
	default:
		return "unknown";
	}
}

void vgh_report_eglerror(const char *location)
{
	EGLint err = eglGetError();
	LOGERR("%s failed (%d, %s).\n", location, err,
	       vgh_egl_error_to_string(err));
}

int vgh_create_context(vgh_context * context,
		       const EGLint attribList[],
		       int window_width, int window_height, int depth)
{
	EGLint num_configs;
	EGLint major_version;
	EGLint minor_version;
	EGLConfig config;
	EGLint *attribs;
	long x11_screen = 0;
	Window root_window;
	XVisualInfo *x11_visual, visTemplate;
	XSetWindowAttributes window_attr;
	unsigned int mask;
	XWindowAttributes root_window_attributes;
	int num_visuals;
	char *s;
	EGLint vid;

	memset(context, 0, sizeof(vgh_context));

	if (!attribList) {
		attribs = (EGLint *) default_config_attr;
	}

	context->x11_display = XOpenDisplay(NULL);
	if (!context->x11_display) {
		LOGERR("Error: Unable to open X display\n");
		vgh_destroy_context(context);
		return 0;
	}
	context->egl_display =
	    eglGetDisplay((NativeDisplayType) context->x11_display);

	if (!eglInitialize
	    (context->egl_display, &major_version, &minor_version)) {
		vgh_report_eglerror("eglInitialize");
		vgh_destroy_context(context);
		return 0;
	}
	s = (char *)eglQueryString(context->egl_display, EGL_VERSION);
	printf("EGL_VERSION = %s\n", s);
	x11_screen = XDefaultScreen(context->x11_display);
	root_window = RootWindow(context->x11_display, x11_screen);
	if ((!eglChooseConfig
	     (context->egl_display, attribs, &config, 1, &num_configs))
	    || !num_configs) {
		vgh_report_eglerror("eglChooseConfig");
		vgh_destroy_context(context);
		return 0;
	}
	if (!eglGetConfigAttrib
	    (context->egl_display, config, EGL_NATIVE_VISUAL_ID, &vid)) {
		vgh_report_eglerror("eglGetConfigAttrib");
		vgh_destroy_context(context);
		return 0;
	}

	/* The X window visual must match the EGL config */
	visTemplate.visualid = vid;
	x11_visual =
	    XGetVisualInfo(context->x11_display, VisualIDMask, &visTemplate,
			   &num_visuals);
	if (!x11_visual) {
		vgh_report_eglerror("XGetVisualInfo");
		vgh_destroy_context(context);
		return 0;
	}

	/* window attributes */
	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap =
	    XCreateColormap(context->x11_display, root_window,
			    x11_visual->visual, AllocNone);
	window_attr.event_mask =
	    StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	XGetWindowAttributes(context->x11_display, root_window,
			     &root_window_attributes);
	if (window_width == 0)
		context->width = root_window_attributes.width;
	else
		context->width = window_width;

	if (window_height == 0)
		context->height = root_window_attributes.height;
	else
		context->height = window_height;

	context->x11_window =
	    XCreateWindow(context->x11_display, root_window, 0, 0,
			  context->width, context->height, 0, x11_visual->depth,
			  InputOutput, x11_visual->visual, mask, &window_attr);
	eglBindAPI(EGL_OPENVG_API);
	context->egl_context =
	    eglCreateContext(context->egl_display, config, EGL_NO_CONTEXT,
			     NULL);
	if (!(context->egl_context)) {
		vgh_report_eglerror("eglCreateContext");
		vgh_destroy_context(context);
		return 0;
	}
	context->egl_surface =
	    eglCreateWindowSurface(context->egl_display, config,
				   context->x11_window, NULL);
	XFree(x11_visual);
	return 1;
}

int vgh_destroy_context(vgh_context * context)
{
	if (context->egl_display) {
		eglMakeCurrent(context->egl_display, EGL_NO_SURFACE,
			       EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglTerminate(context->egl_display);
		context->egl_display = NULL;
	}

	if (context->x11_window) {
		XDestroyWindow(context->x11_display, context->x11_window);
		context->x11_window = 0;
	}

	if (context->x11_colormap) {
		XFreeColormap(context->x11_display, context->x11_colormap);
		context->x11_colormap = 0;
	}

	if (context->x11_display) {
		/* TODO: Uncomment when bug #123095 is fixed */
/*		XCloseDisplay(context->x11_display);*/
		context->x11_display = NULL;
	}

	memset(context, 0, sizeof(vgh_context));

	return 1;
}

double vgh_time_step()
{
	return time_step;
}

//TODO: 
static int vgh_prepare_redraw(vgh_context * context, redraw_context * rectx)
{

	return 1;
}

int vgh_execute_main_loop(vgh_context * context,
			  DRAW_FUNCTION, redraw_context * rectx, double runtime)
{
	int running = 1;
	int t;
	struct rusage usage_start;
	struct rusage usage_end;
	double used_time = 0.0;
	FILE *fp;
	long unsigned int start_user_load, start_nice_load, start_sys_load,
	    start_idle;
	double prev_time = 0;
	double cur_time;

	fp = fopen("/proc/stat", "r");
	if (fp) {
		char tmp[128];
		if (fscanf(fp, "%s %lu %lu %lu %lu", tmp, &start_user_load,
			   &start_nice_load, &start_sys_load, &start_idle) != 5)
		{
			LOGERR("Failed to read /proc/stat\n");
		}
		fclose(fp);
	}

	context->perf_data.frames_rendered = 0;
	getrusage(RUSAGE_SELF, &usage_start);
	timing_start();

	while (running) {
		cur_time = timing_elapsed();
		time_step = cur_time - prev_time;
		prev_time = cur_time;
		if (!vgh_prepare_redraw(context, rectx)) {
			LOGERR("Failed to prepare next frame %d\n",
			       context->perf_data.frames_rendered);
			return 0;
		}
		if (!drawFunc(context, rectx)) {
			LOGERR("Failed to draw frame %d\n",
			       context->perf_data.frames_rendered);
			return 0;
		}
		eglSwapBuffers(context->egl_display, context->egl_surface);
		context->perf_data.frames_rendered++;
		rectx->counter++;
		if (runtime == 0.0f) {
			int i32NumMessages = XPending(context->x11_display);
			for (t = 0; t < i32NumMessages; t++) {
				XEvent event;
				XNextEvent(context->x11_display, &event);
				if (event.type == ButtonPress)
					running = 0;
			}
		} else {
			if (cur_time >= runtime) {
				running = 0;
			}
		}
	}

	timing_stop();

	context->perf_data.total_time_elapsed = timing_elapsed();

	getrusage(RUSAGE_SELF, &usage_end);
	used_time = usage_end.ru_utime.tv_sec;
	used_time += usage_end.ru_utime.tv_usec * 1E-6;
	used_time += usage_end.ru_stime.tv_sec;
	used_time += usage_end.ru_stime.tv_usec * 1E-6;
	used_time -= usage_start.ru_utime.tv_sec;
	used_time -= usage_start.ru_utime.tv_usec * 1E-6;
	used_time -= usage_start.ru_stime.tv_sec;
	used_time -= usage_start.ru_stime.tv_usec * 1E-6;

	context->perf_data.total_load = 0.0f;

	fp = fopen("/proc/stat", "r");
	if (fp) {
		char tmp[128];
		long unsigned int user_load, nice_load, sys_load, idle;
		if (fscanf
		    (fp, "%s %lu %lu %lu %lu", tmp, &user_load, &nice_load,
		     &sys_load, &idle) == 5) {
			user_load -= start_user_load;
			nice_load -= start_nice_load;
			sys_load -= start_sys_load;
			idle -= start_idle;
			context->perf_data.total_load =
			    (double)(user_load + nice_load + sys_load);
			context->perf_data.total_load =
			    context->perf_data.total_load * 100.0f /
			    (context->perf_data.total_load + idle);
		} else {
			LOGERR("Failed to read /proc/stat\n");
		}

		fclose(fp);
	}

	context->perf_data.fps = 1.0f / (context->perf_data.total_time_elapsed /
					 context->perf_data.frames_rendered);
	context->perf_data.cpu_usage = 100.0 * used_time /
	    context->perf_data.total_time_elapsed;

	LOG("Frames rendered: %d\n", context->perf_data.frames_rendered);
	LOG("Total render time: %lf\n", context->perf_data.total_time_elapsed);
	LOG("Frames per second: %lf\n", context->perf_data.fps);
	LOG("CPU usage (this process): %lf %%\n", context->perf_data.cpu_usage);
	if (context->perf_data.total_load > 0.0f) {
		LOG("CPU usage (all processes, all CPUs): %lf %%\n",
		    context->perf_data.total_load);
	} else {
		LOG("CPU usage (all processes, all CPUs): N/A\n");
	}

	return 1;
}
