/* display_rotate_tests.c -- Rotate screen with 0/90/180/270 and reflect with X/Y

   Copyright (C) 2000-2010, Intel Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Zheng Kui <kui.zheng@intel.com>

*/

#include <unistd.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>

#include <X11/extensions/Xrandr.h>
#include "display_helper.h"

int test_backlight (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
//Screen
	XRRScreenResources *scr_res = NULL;
	int i;
//CRTC
	XRRCrtcInfo *crtc_info = NULL;
//output
	XRROutputInfo *output_info = NULL;
//Property
	static Atom backlight;
	XRRPropertyInfo *prop_info = NULL;
	long value = 0;

	if (create_window (&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if (!XRRQueryExtension (params.display, &event_base, &error_base))
	{
		BLTS_ERROR ("XRRQueryExtension failed\n");
		ret = -1;
		goto cleanup;
	}

	if (!XRRQueryVersion (params.display, &ver, &rev))
	{
		BLTS_ERROR ("XRRQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG ("XRandR Extension version %i.%i\n", ver, rev);
	XRRSelectInput (params.display, params.root_window,
		RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask
		| RROutputPropertyNotifyMask);
	backlight = XInternAtom (params.display, "Backlight", True);
//Set Backlight
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	for (i = 0; i < scr_res->noutput; i++)
	{
		BLTS_DEBUG ("RROutput ID: 0x%lx\n", *(scr_res->outputs + i));
		output_info =
			XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
		BLTS_DEBUG ("	Crtc=0x%lx, name=%s, mm_width=%u, mm_height=%u, connection=%d,\n",
			output_info->crtc, output_info->name, output_info->mm_width,
			output_info->mm_height, output_info->connection);
		if (!output_info->connection && output_info->crtc)
		{
			prop_info =
				XRRQueryOutputProperty (params.display, *(scr_res->outputs + i),
				backlight);
			if (prop_info->range)
				BLTS_DEBUG ("	BACKLIGHT range: (%d, %d)\n", *(prop_info->values),
					*(prop_info->values + 1));
			for (value = 0; value <= *(prop_info->values + 1); value++)
			{
				BLTS_DEBUG ("		Setting backlight value: %d\n", value);
				XRRChangeOutputProperty (params.display, *(scr_res->outputs + i),
					backlight, XA_INTEGER, 32, PropModeReplace, (unsigned char *) &value,
					1);
				output_info =
					XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
				sleep (1);
			}
		}
	}

cleanup:
	if (scr_res)
		XRRFreeScreenResources (scr_res);
	if (crtc_info)
		XRRFreeCrtcInfo (crtc_info);
	if (output_info)
		XRRFreeOutputInfo (output_info);
	close_window (&params);
	return ret;
}
