/* xdamage_tests.c -- Tests for X Damage extension

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

#include <X11/extensions/Xdamage.h>
#include "xdamage_tests.h"
#include "blts_x11_util.h"

int xdamage_monitor_region(double execution_time)
{
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	XEvent event;
	Damage damage;
	XserverRegion region;
	XRectangle *rectlist;
	int howmany;
	int event_received;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if(!XDamageQueryExtension(params.display, &event_base, &error_base))
	{
		BLTS_ERROR("XDamageQueryExtension failed\n");
		ret = -1;
		goto cleanup;
	}

	if(!XDamageQueryVersion(params.display, &ver, &rev))
	{
		BLTS_ERROR("XDamageQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XDamage Extension version %i.%i\n", ver, rev);

	/* Create damage region */
	damage = XDamageCreate(params.display, params.window,
		XDamageReportDeltaRectangles);
	region = XFixesCreateRegion(params.display, 0, 0);

	timing_start();

	XDrawString(params.display, params.window, params.gc, 75, 20, "XDamage", 7);
	XFlush(params.display);

	/* Wait for damage notify (should be received instantly) */
	event_received = 0;
	while(!event_received && timing_elapsed() < execution_time)
	{
		XNextEvent(params.display, &event);
		switch(event.type) {
		default:
			if(event.type == event_base + XDamageNotify)
			{
				XDamageNotifyEvent* dev = (XDamageNotifyEvent*) &event;
				BLTS_DEBUG("Damage Event received: x:%3d, y:%3d, width: %3d, height:%3d\n",
					(int) dev->area.x, (int) dev->area.y,
					(int) dev->area.width, (int) dev->area.height);
				XDamageSubtract(params.display, damage, None, region);
				rectlist = XFixesFetchRegion(params.display, region, &howmany);
				if(rectlist && howmany )
				{
					int i;
					BLTS_DEBUG("XDamageSubtract: %d rectangles\n", howmany);
					for(i=0; i<howmany; i++)
					{
						BLTS_DEBUG("rectangle %d: x: %d, y: %d, width: %d, height: %d\n",
							i+1, rectlist[i].x, rectlist[i].y,
							rectlist[i].width, rectlist[i].height);
					}
				}
				event_received = 1;
			}
			else
			{
				BLTS_DEBUG("Unknown event %x\n", event.type);
			}
			break;
		}
	}

	timing_stop();

	if(!event_received)
	{
		BLTS_ERROR("XDamageNotify event not received.\n");
		ret = -1;
	}


cleanup:
	close_window(&params);
	return ret;
}

