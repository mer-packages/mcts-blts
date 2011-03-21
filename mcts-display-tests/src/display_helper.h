/* display_helper.h -- Various helper functions and definitions

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

#include <X11/Xlib.h>
#include <blts_log.h>
#include <blts_timing.h>
#include <blts_dep_check.h>

#define X_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define X_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define UNUSED_PARAM(a) ((void)a)
#define REFRESH_RATE(DotClock, HTotal, VTotal)	\
	((DotClock) / ((HTotal) * (VTotal)))

// X window 
typedef struct
{
	Display *display;
	Window root_window;
	Window window;
	XWindowAttributes root_window_attributes;
	XWindowAttributes window_attributes;
	long screen;
	GC gc;
	Visual *visual;
	int width;
	int height;
} window_struct;

int create_window (window_struct * params, const char *window_name);
int create_window_ex (window_struct * params, const char *window_name,
	int width, int height);
int close_window (window_struct * params);
Display *open_display ();

/*
* Basic X test. XCreateSimpleWindow, draw rectangle around window and
* "Hello World" -text.
*/
int create_simple_window (double execution_time);

// Crtc/Output/Modeline
