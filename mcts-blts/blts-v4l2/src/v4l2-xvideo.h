/* v4l2-xvideo.h -- XVideo utility functions for v4l2 test cases

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

#ifndef V4L2_XVIDEO_H
#define V4L2_XVIDEO_H

#include <ctype.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>

typedef struct
{
	Display* display;
	Window root_window;
	Window window;
	XWindowAttributes root_window_attributes;
	XWindowAttributes window_attributes;
	long screen;
	GC gc;
	Visual* visual;
	int width;
	int height;
} window_struct;


int create_window(window_struct* params, const char* window_name);
int create_window_ex(window_struct* params, const char* window_name, int width, int height);
int close_window(window_struct* params);
Display* open_display();

void xvideo_init(int cam_width, int cam_height, int img_format);
void xvideo_deinit();

void xvideo_process_image(const void * p);

#endif /* V4L2_XVIDEO_H */
