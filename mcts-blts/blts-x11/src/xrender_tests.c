/* xrender_tests.c -- Tests for X Render extension

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

#include <X11/extensions/Xrender.h>
#include "xrender_tests.h"
#include "blts_x11_util.h"

int xrender_draw_rectangle(double execution_time)
{
	int ret = 0;
	int t, ver, rev, event_base, error_base;
	window_struct params;
	XRenderPictFormat* fmt = 0;
	XRenderPictureAttributes pict_attr;
	Picture drawingarea = 0;
	Pixmap background = 0;
	XRenderColor white = { 0xffff, 0xffff, 0xffff, 0xffff };

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if(!XRenderQueryExtension(params.display, &event_base, &error_base))
	{
		BLTS_ERROR("XRecordQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	if(!XRenderQueryVersion(params.display, &ver, &rev))
	{
		BLTS_ERROR("XRenderQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XRender Extension version %i.%i\n", ver, rev);

	background = XCreatePixmap(params.display, params.window,
		params.window_attributes.width, params.window_attributes.height,
		params.window_attributes.depth);
	if(!background)
	{
		BLTS_ERROR("XCreatePixmap failed\n");
		ret = -1;
		goto cleanup;
	}

	fmt = XRenderFindVisualFormat(params.display, params.visual);
	if(!fmt)
	{
		BLTS_ERROR("XRenderFindVisualFormat failed\n");
		ret = -1;
		goto cleanup;
	}

	pict_attr.poly_edge = PolyEdgeSmooth;
	pict_attr.poly_mode = PolyModeImprecise;
	drawingarea = XRenderCreatePicture(params.display, background, fmt,
		CPPolyEdge|CPPolyMode, &pict_attr);

	if(!drawingarea)
	{
		BLTS_ERROR("XRenderCreatePicture failed\n");
		ret = -1;
		goto cleanup;
	}

	XSetWindowBackgroundPixmap(params.display, params.window, background);
	XFlush(params.display);

	timing_start();
	for(t = 0; t < X_MIN(params.window_attributes.width / 2,
		params.window_attributes.height / 2); t++)
	{
		XRenderFillRectangle(params.display, PictOpOver, drawingarea, &white,
			params.window_attributes.width / 2 - t,
			params.window_attributes.height / 2 - t,
			t * 2, t * 2);

		XClearArea(params.display, params.window, 0, 0,
			params.window_attributes.width,
			params.window_attributes.height, 0);
		XFlush(params.display);
		if(timing_elapsed() > execution_time)
		{
			break;
		}
	}
	timing_stop();

cleanup:

	if(drawingarea)
	{
		XRenderFreePicture(params.display, drawingarea);
	}

	close_window(&params);
	return ret;
}

