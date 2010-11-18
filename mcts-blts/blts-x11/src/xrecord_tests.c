/* xrecord_tests.c -- Tests for X Record extension

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

#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>
#include "xrecord_tests.h"
#include "blts_x11_util.h"

int cur_x, cur_y;
int stop;

typedef union {
	unsigned char type;
	xEvent event;
	xResourceReq req;
	xGenericReply reply;
	xError error;
	xConnSetupPrefix setup;
} XRecordDatum;

/* Callback for xrecord events */
static void xrec_callback(XPointer p, XRecordInterceptData* hook)
{
	UNUSED_PARAM(p);
	if(hook->category == XRecordFromServer)
	{
		XRecordDatum* data = (XRecordDatum *) hook->data;
		switch(data->type)
		{
			case ButtonPress:
				BLTS_DEBUG("ButtonPress %d, X=%d, Y=%d",
					data->event.u.u.detail, cur_x, cur_y);
				break;
			case ButtonRelease:
				BLTS_DEBUG("ButtonRelease %d, X=%d, Y=%d",
					data->event.u.u.detail, cur_x, cur_y);
				break;
			case MotionNotify:
				BLTS_DEBUG("MouseMove X=%d, Y=%d",
					data->event.u.keyButtonPointer.rootX,
					data->event.u.keyButtonPointer.rootY);
				cur_x = data->event.u.keyButtonPointer.rootX;
				cur_y = data->event.u.keyButtonPointer.rootY;
				stop = 1;
				break;
			default:
				break;
		}

		BLTS_DEBUG(", time=%d\n", (int)hook->server_time);
	}

   XRecordFreeData(hook);

}

int xrecord_capture_mouse_movement(double execution_time)
{
	int ret = 0;
	int ver, rev;
	window_struct params;
	XRecordContext context = 0;
	XRecordClientSpec clients = 0;
	XRecordRange* range[1];
	Display* ctl_dpy = NULL;

	range[0] = NULL;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	ctl_dpy = open_display();
	if(!ctl_dpy)
	{
		BLTS_ERROR("XOpenDisplay failed\n");
		ret = -1;
		goto cleanup;
	}

	XSynchronize(ctl_dpy, True);

	if(!XRecordQueryVersion(params.display, &ver, &rev))
	{
		BLTS_ERROR("XRecordQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XRecord Extension version %i.%i\n", ver, rev);

	clients = XRecordAllClients;
	range[0] = XRecordAllocRange();
	if (range[0] == 0)
	{
		BLTS_ERROR("Failed to allocate XRecordRange\n");
		ret = -1;
		goto cleanup;
	}

	range[0]->device_events.first = KeyPress;
	range[0]->device_events.last = MotionNotify;
	context = XRecordCreateContext(ctl_dpy, 0, &clients, 1, range, 1);
	if (context == 0)
	{
		BLTS_ERROR("Failed to create XRecordContext\n");
		ret = -1;
		goto cleanup;
	}

	/* Revisit: XRecord is broken (http://bugs.freedesktop.org/show_bug.cgi?id=21971).
	 * Somekind of workaround, except you still won't get any events
	 * (http://bugs.freedesktop.org/show_bug.cgi?id=20500). */
	XSync(ctl_dpy, False);

	stop = 0;
	XRecordEnableContextAsync(params.display, context, xrec_callback,
		(XPointer)&params);

	BLTS_DEBUG("Waiting for mouse movement...\n");

	timing_start();
    while(!stop && timing_elapsed() < execution_time)
    {
		XRecordProcessReplies(params.display);
	}
	timing_stop();

cleanup:
	if(range[0])
	{
		XFree(range[0]);
	}

	if(context)
	{
		XRecordDisableContext(ctl_dpy, context);
		XRecordFreeContext(ctl_dpy, context);
	}

	if(ctl_dpy)
	{
		XCloseDisplay(ctl_dpy);
	}

	close_window(&params);

	return ret;
}

