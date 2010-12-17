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

int test_rotate (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	Status err;
//Screen
	XRRScreenConfiguration *scr_conf = NULL;
	XRRScreenResources *scr_res = NULL;
	short current_rate = 0;
	Rotation current_rotation = 0;
	Rotation rotations = 0;
	int current_size_id = 0;
	int i, j;
	unsigned short RotationOp = 0x0001;
//CRTC
	XRRCrtcInfo *crtc_info = NULL;
//output
	XRROutputInfo *output_info = NULL;

	if (create_window (&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if (!XRRQueryExtension (params.display, &event_base, &error_base))
	{
		LOGERR ("XRRQueryExtension failed\n");
		ret = -1;
		goto cleanup;
	}

	if (!XRRQueryVersion (params.display, &ver, &rev))
	{
		LOGERR ("XRRQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}

	LOG ("XRandR Extension version %i.%i\n", ver, rev);
	XRRSelectInput (params.display, params.root_window,
		RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask
		| RROutputPropertyNotifyMask);
//screen configure
	scr_conf = XRRGetScreenInfo (params.display, params.root_window);
	rotations = XRRConfigRotations (scr_conf, &current_rotation);
	current_rate = XRRConfigCurrentRate (scr_conf);
	current_size_id = XRRConfigCurrentConfiguration (scr_conf, &current_rotation);
//Rotate Screen
	LOG ("Rotating Screen .......\n");
	for (i = 0; i < 5; i++)
	{
		err =
			XRRSetScreenConfig (params.display, scr_conf, params.root_window,
			current_size_id, RotationOp, CurrentTime);
		if (RotationOp == RR_Rotate_270)
			RotationOp = RR_Rotate_0;
		else
			RotationOp = RotationOp << 1;
		current_size_id =
			XRRConfigCurrentConfiguration (scr_conf, &current_rotation);
		LOG ("	Current Rotation = 0x%x\n", current_rotation);
		sleep (1);
	}

//Rotate Crtc
	LOG ("Rotating Crtc .......");
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	for (i = 0; i < scr_res->noutput; i++)
	{
		LOG ("RROutput ID: 0x%lx\n", *(scr_res->outputs + i));
		output_info =
			XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
		LOG ("   Crtc=0x%lx, name=%s, mm_width=%u, mm_height=%u, connection=%d,\n",
			output_info->crtc, output_info->name, output_info->mm_width,
			output_info->mm_height, output_info->connection);
		RotationOp = RR_Rotate_0;
		if (!output_info->connection && output_info->crtc)
		{
			crtc_info = XRRGetCrtcInfo (params.display, scr_res, output_info->crtc);
			for (j = 0; j < 5; j++)
			{
				err =
					XRRSetCrtcConfig (params.display, scr_res, output_info->crtc,
					CurrentTime, crtc_info->x, crtc_info->y, crtc_info->mode, RotationOp,
					crtc_info->outputs, crtc_info->noutput);
				sleep (2);
				LOG ("	Current RotationOp: 0x%x\n", RotationOp);
				RotationOp = RotationOp << 1;
				if (RotationOp > RR_Rotate_270)
					RotationOp = RR_Rotate_0;
			}
		}
	}

cleanup:
	if (scr_conf)
		XRRFreeScreenConfigInfo (scr_conf);
	if (scr_res)
		XRRFreeScreenResources (scr_res);
	if (crtc_info)
		XRRFreeCrtcInfo (crtc_info);
	if (output_info)
		XRRFreeOutputInfo (output_info);
	close_window (&params);
	return ret;
}
