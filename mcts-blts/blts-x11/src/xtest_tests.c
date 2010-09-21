/* xtest_tests.c -- Tests for X Test extension

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

#include <X11/extensions/XTest.h>
#include "xtest_tests.h"
#include "blts_x11_util.h"

int xtest_move_cursor(double execution_time)
{
	int ret = 0;
	int ver, rev, eventB, errorB;
	int x, y;
	window_struct params;
	Cursor my_cursor = 0;
	Pixmap my_cursor_pxm = 0;
	XColor black;
	black.red = black.green = black.blue = 0;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if(!XTestQueryExtension(params.display, &eventB, &errorB, &ver, &rev))
	{
		LOGERR("X-Test extension not found\n");
		ret = -1;
		goto cleanup;
	}

	LOG("XTest Extension version %i.%i\n", ver, rev);

	/* Create our own cursor to move around */
	my_cursor_pxm = XCreatePixmap(params.display, params.window, 8, 8,
		params.root_window_attributes.depth);
	if(!my_cursor_pxm)
	{
		LOGERR("XCreateBitmapFromData failed\n");
		ret = -1;
		goto cleanup;
	}
 	my_cursor = XCreatePixmapCursor(params.display, my_cursor_pxm, None,
 		&black, &black, 0, 0);
	if(!my_cursor)
	{
		LOGERR("XCreatePixmapCursor failed\n");
		ret = -1;
		goto cleanup;
	}
 	XDefineCursor(params.display, params.window, my_cursor);

	timing_start();

	/* Create fake motion events to move the cursor from left to right edge
	 * of the window
	 */
	y = params.root_window_attributes.height / 2;
	for(x = 0; x < params.root_window_attributes.width; x++)
	{
		if(timing_elapsed() > execution_time)
		{
			break;
		}
		if(!XTestFakeMotionEvent(params.display, params.screen, x, y, 2))
		{
			LOGERR("XTestFakeMotionEvent failed\n");
			ret = -1;
			break;
		}
	}

	timing_stop();

cleanup:
	if(params.display)
	{
		if(my_cursor)
		{
			XFreeCursor(params.display, my_cursor);
			XUndefineCursor(params.display, params.window);
		}

		if(my_cursor_pxm)
		{
			XFreePixmap(params.display, my_cursor_pxm);
		}
	}

	close_window(&params);

	return ret;
}

