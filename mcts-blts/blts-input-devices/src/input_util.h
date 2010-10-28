/* input_util.h -- input device utility functions

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

#ifndef INPUT_UTIL_H
#define INPUT_UTIL_H

#include <stdio.h>
#include <X11/Xutil.h>

struct window_struct
{
	Display *display;
	Window root_window;
	Window window;
	XWindowAttributes root_window_attributes;
	XWindowAttributes window_attributes;
	long screen;
	GC gc;
	Visual *visual;
	unsigned width;
	unsigned height;
};

Display* input_open_display();
int input_close_window(struct window_struct* params);
int input_create_window(struct window_struct* params,
	const char* window_name, int width, int height, int screen);
int input_write_bmp(struct window_struct *ws, const char* filename, XImage *img);

#endif /* INPUT_UTIL_H */

