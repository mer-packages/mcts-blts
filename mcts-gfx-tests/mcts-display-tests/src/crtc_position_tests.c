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

int test_position (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	Status err;
	int i, j;
//Screen
	XRRScreenResources *scr_res = NULL;
//CRTC
	XRRCrtcInfo *crtc_info = NULL;
	unsigned short crtc_position[2] = { 0, 0 };
//output
	XRROutputInfo *output_info = NULL;

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
//Posite Crtc
	BLTS_DEBUG ("Setting Crtc Position ......\n");
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	for (i = 0; i < scr_res->noutput; i++)
	{
		BLTS_DEBUG ("RROutput ID: 0x%lx, ", *(scr_res->outputs + i));
		output_info =
			XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
		BLTS_DEBUG ("name=%s, mm_width=%u, mm_height=%u, connection=%d,\n",
			output_info->name, output_info->mm_width, output_info->mm_height,
			output_info->connection);
		if (!output_info->connection && output_info->crtc)
		{
			for (j = 0; j < 5; j++)
			{
				crtc_info = XRRGetCrtcInfo (params.display, scr_res, output_info->crtc);
				BLTS_DEBUG ("	Crtc ID:0x%lx, position:(%d, %d), size:%ux%u, mode:0x%lx\n",
					output_info->crtc, crtc_info->x, crtc_info->y, crtc_info->width,
					crtc_info->height, crtc_info->mode);
				crtc_position[0] = crtc_info->x - 50;
				crtc_position[1] = crtc_info->y - 20;
				err =
					XRRSetCrtcConfig (params.display, scr_res, output_info->crtc,
					CurrentTime, crtc_position[0], crtc_position[1], crtc_info->mode,
					RR_Rotate_0, crtc_info->outputs, crtc_info->noutput);
				sleep (2);
			}
			crtc_info = XRRGetCrtcInfo (params.display, scr_res, output_info->crtc);
			BLTS_DEBUG ("	Crtc ID:0x%lx, position:(%d, %d), size:%ux%u, mode:0x%lx\n",
				output_info->crtc, crtc_info->x, crtc_info->y, crtc_info->width,
				crtc_info->height, crtc_info->mode);
			BLTS_DEBUG ("Restoring original position ......\n");
			err =
				XRRSetCrtcConfig (params.display, scr_res, output_info->crtc,
				CurrentTime, 0, 0, crtc_info->mode, RR_Rotate_0, crtc_info->outputs,
				crtc_info->noutput);
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
