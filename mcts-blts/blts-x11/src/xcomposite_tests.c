/* xcomposite_tests.c -- Tests for X Composite extension

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

#include <unistd.h>
#include <X11/extensions/Xcomposite.h>
#include "xcomposite_tests.h"
#include "blts_x11_util.h"

int xcomposite_get_widow_contents()
{
	int ret = 0;

	int ver, rev, eventB, baseB;
	int x, y;
	Pixmap pxm = 0;
	window_struct params;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if(!XCompositeQueryExtension(params.display, &eventB, &baseB))
	{
		BLTS_ERROR("XCompositeQueryExtension failed\n");
		ret = -1;
		goto cleanup;
	}

	if(!XCompositeQueryVersion(params.display, &ver, &rev))
	{
		BLTS_ERROR("XCompositeQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XComposite Extension version %i.%i\n", ver, rev);

	/* Redirect window to off-screen storage (the window might be already
	redirected when using composite window manager) */
	XCompositeRedirectWindow(params.display, params.window, 0);

	/* Draw rectangle pattern */
	for(x = 0; x < params.window_attributes.width; x+=20)
	{
		for(y = 0; y < params.window_attributes.height; y+=20)
		{
			XFillRectangle(params.display, params.window, params.gc, x, y,
				10, 10);
		}
	}
	XFlush(params.display);

	sleep(1);

	/* Get pixmap of window contents */
	pxm = XCompositeNameWindowPixmap(params.display, params.window);
	XCompositeUnredirectWindow(params.display, params.window, 0);

	/* Copy the pixmap over the window (you should see white/black stripes
	 * after this)
	 */
	for(y = 0; y < params.window_attributes.height; y++)
	{
		XCopyArea(params.display, pxm, params.window, params.gc, 0, 0,
			params.window_attributes.width, params.window_attributes.height,
			0, y );
	}
	XFlush(params.display);

	sleep(1);

cleanup:
	if(params.display && pxm)
	{
		XFreePixmap(params.display, pxm);
	}
	close_window(&params);

	return ret;
}

