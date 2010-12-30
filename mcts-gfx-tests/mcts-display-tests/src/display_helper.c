/* display_helper.c -- For help display testing

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
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include "display_helper.h"

Display *open_display ()
{
	Display *display = NULL;

	display = XOpenDisplay (NULL);
	if (!display)
	{
		/* DISPLAY environment variable may not exist on non-posix-conformant systems */
		BLTS_DEBUG ("XOpenDisplay(NULL) failed, DISPLAY environment variable missing? Trying display :0.\n");
		display = XOpenDisplay (":0");
		if (!display)
		{
			return NULL;
		}
	}

	return display;
}

int close_window (window_struct * params)
{
	if (params->display)
	{
		if (params->window)
		{
			XUnmapWindow (params->display, params->window);
			XDestroyWindow (params->display, params->window);
		}

		if (params->gc)
		{
			XFreeGC (params->display, params->gc);
		}

		XFlush (params->display);
		XCloseDisplay (params->display);
	}

	memset (params, 0, sizeof (window_struct));

	return 0;
}

int create_window (window_struct * params, const char *window_name)
{
	return create_window_ex (params, window_name, 0, 0);
}

int create_window_ex (window_struct * params, const char *window_name,
	int width, int height)
{
	XGCValues values;

	memset (params, 0, sizeof (window_struct));
	/* Open default display */
	params->display = open_display ();
	if (!params->display)
	{
		BLTS_ERROR ("XOpenDisplaRORailed\n");
		goto cleanup;
	}

	params->screen = XDefaultScreen (params->display);
	params->root_window = RootWindow (params->display, params->screen);
	if (!params->root_window)
	{
		BLTS_ERROR ("No root window\n");
		goto cleanup;
	}

	params->visual = DefaultVisual (params->display, params->screen);

	if (!XGetWindowAttributes (params->display, params->root_window,
			&params->root_window_attributes))
	{
		BLTS_ERROR ("XGetWindowAttributes failed\n");
		goto cleanup;
	}

	if (width)
	{
		params->width = width;
	}
	else
	{
		params->width = params->root_window_attributes.width / 4;
	}
	if (height)
	{
		params->height = height;
	}
	else
	{
		params->height = params->root_window_attributes.height / 4;
	}
	params->window =
		XCreateSimpleWindow (params->display, params->root_window, 300, 200,
		params->width, params->height, 0, 0x0, 0x00FFFFFF);
	if (!params->window)
	{
		BLTS_ERROR ("XCreateSimpleWindow failed\n");
		goto cleanup;
	}

	if (!XGetWindowAttributes (params->display, params->root_window,
			&params->window_attributes))
	{
		BLTS_ERROR ("XGetWindowAttributes failed\n");
		goto cleanup;
	}

	if (!XMapWindow (params->display, params->window))
	{
		BLTS_ERROR ("XMapWindow failed\n");
		goto cleanup;
	}

	if (!XRaiseWindow (params->display, params->window))
	{
		BLTS_ERROR ("XRaiseWindow failed\n");
		goto cleanup;
	}

	if (!XStoreName (params->display, params->window, window_name))
	{
		BLTS_ERROR ("XStoreName failed\n");
		goto cleanup;
	}

	values.foreground = BlackPixel (params->display, params->screen);
	params->gc =
		XCreateGC (params->display, params->window, GCForeground, &values);
	if (!params->gc)
	{
		BLTS_ERROR ("XCreateGC failed\n");
		goto cleanup;
	}

	return 0;

cleanup:
	close_window (params);
	return -1;
}

int create_simple_window (double execution_time)
{
	int ret = 0;
	int x, y, speedx, speedy;
	const char *text = "Hello World";
	window_struct params;

	if (create_window (&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG ("Display width: %d height: %d depth: %d\n", DisplayWidth (params.display,
			params.screen), DisplayHeight (params.display, params.screen),
		DefaultDepth (params.display, params.screen));

	timing_start ();

	x = 1;
	y = 1;
	speedx = 3;
	speedy = 2;
	while (timing_elapsed () < execution_time)
	{
		XClearWindow (params.display, params.window);
		XDrawString (params.display, params.window, params.gc, x, y, text,
			strlen (text));
		XDrawRectangle (params.display, params.window, params.gc, 5, 5,
			params.window_attributes.width - 10,
			params.window_attributes.height - 10);
		XFlush (params.display);
		usleep (10000);

		if (x <= 0 || x >= params.window_attributes.width)
		{
			speedx = -speedx;
		}

		if (y <= 0 || y >= params.window_attributes.height)
		{
			speedy = -speedy;
		}

		x += speedx;
		y += speedy;
	}

	timing_stop ();

cleanup:
	close_window (&params);
	return ret;
}
