/* openvg_helper.h -- Various helper functions and definitions for GLES2

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

#ifndef OPENVG_HELPER_H
#define OPENVG_HELPER_H

#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <VG/openvg.h>
#include <blts_log.h>
#include <blts_timing.h>
#include <errno.h>
#include <math.h>

#define UNUSED_PARAM(a) (void)(a);
#define VGH_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define VGH_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))
#define COUNTER_TO_COLOR(cur, counter, step) \
	(((cur + step * counter) > 1.0f)?((cur + step * counter) - floor(cur + step * counter) ): \
	(cur + step * counter))         

#define FEA_NUM 8
#define DASH_PATTERN_COUNT 2

typedef struct
{
	unsigned int frames_rendered;
	double total_time_elapsed;
	double fps;
	double cpu_usage;
	double total_load;
} vgh_perf_data;

typedef struct
{
	VGint width; /* Render window width in pixels */
	VGint height; /* Render window height in pixels */
	VGint depth; /* Render window depth in bpp */

	EGLDisplay egl_display;
	EGLContext egl_context;
	EGLSurface egl_surface;
	Display* x11_display;
	Window x11_window;
	Colormap x11_colormap;
	vgh_perf_data perf_data;

	VGPath vgPath;
	VGPaint vgPaint;
} vgh_context;


typedef struct
{
	VGuint counter;
	struct{
		VGfloat tx_step;
		VGfloat ty_step;
		VGuint sx_step;
		VGuint sy_step;
		VGfloat shx_step;
		VGfloat shy_step;
		VGfloat angle_step;
	} transformation;
	struct{
		VGfloat width_step;
		VGfloat dpattern_step[DASH_PATTERN_COUNT];
	} stroke;
	VGfloat color_step[4];	   	
	VGfloat rampStop_step[4][5];
	void (*redraw[FEA_NUM])(void *);	//redraw[0]: pointer to transformation callback function.
						//redraw[1]: pointer to color callback function.
						//redraw[2]: pointer to rampStop callback function.
						//redraw[3]: pointer to stroke callback function.
} redraw_context;

#define DRAW_FUNCTION int (*drawFunc)(vgh_context* context, redraw_context* user_ptr)

/* Init/uninit */
int vgh_create_context(vgh_context* context, const EGLint attribList[],
	int window_width, int window_height, int depth);
int vgh_execute_main_loop(vgh_context* context, DRAW_FUNCTION,
	redraw_context* user_ptr, double runtime);
int vgh_destroy_context(vgh_context* context);
/* Misc */
void vgh_report_eglerror(const char* location);
const char* vgh_egl_error_to_string(EGLint err);
/* for debug */
void event_loop(vgh_context* context, DRAW_FUNCTION);

#endif // OPENVG_HELPER

