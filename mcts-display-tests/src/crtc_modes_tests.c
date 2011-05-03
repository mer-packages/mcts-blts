/*
 Copyright (C) 2010 Intel Corporation
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 Author: 
     Zheng Kui (kui.zheng@intel.com)
*/


#include <unistd.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>

#include <X11/extensions/Xrandr.h>
#include "display_helper.h"

int test_modes (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	Status err;
	int i, j, k;
//Screen
	XRRScreenResources *scr_res = NULL;
//CRTC
	XRRCrtcInfo *crtc_info = NULL;
//output
	XRROutputInfo *output_info = NULL;
	RRMode origin_mode = 0;

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
//Setting Crtc Modeline
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	for (i = 0; i < scr_res->noutput; i++)
	{
		BLTS_DEBUG ("RROutput ID: 0x%lx, ", *(scr_res->outputs + i));
		output_info =
			XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
		BLTS_DEBUG ("name=%s, mm_width=%u, mm_height=%u, connection=%d\n",
			output_info->name, output_info->mm_width, output_info->mm_height,
			output_info->connection);
		if (!output_info->connection && output_info->crtc)
		{
			crtc_info = XRRGetCrtcInfo (params.display, scr_res, output_info->crtc);
			origin_mode = crtc_info->mode;
			for (k = 0; k < output_info->nmode; k++)
			{
				for (j = 0; j < scr_res->nmode; j++)
				{
					if (*(output_info->modes + k) == (scr_res->modes + j)->id)
					{
						BLTS_DEBUG ("	Setting Crtc mode ......\n");
						crtc_info =
							XRRGetCrtcInfo (params.display, scr_res, output_info->crtc);
						err =
							XRRSetCrtcConfig (params.display, scr_res, output_info->crtc,
							CurrentTime, crtc_info->x, crtc_info->y,
							*(output_info->modes + k), RR_Rotate_0, crtc_info->outputs,
							crtc_info->noutput);
						BLTS_DEBUG
							("   -id:0x%lx, name:%s, size:%ux%u, refresh rate:%d, dotClock:%lu\n",
							(scr_res->modes + j)->id, (scr_res->modes + j)->name,
							(scr_res->modes + j)->width, (scr_res->modes + j)->height,
							REFRESH_RATE (((scr_res->modes + j)->dotClock),
								((scr_res->modes + j)->hTotal), ((scr_res->modes + j)->vTotal)),
							(scr_res->modes + j)->dotClock);
						BLTS_DEBUG
							("              hSyncStart:%u, hSyncEnd:%u, hTotal:%u, hSkew:%u\n",
							(scr_res->modes + j)->hSyncStart, (scr_res->modes + j)->hSyncEnd,
							(scr_res->modes + j)->hTotal, (scr_res->modes + j)->hSkew);
						BLTS_DEBUG
							("              vSyncStart:%u, vSyncEnd:%u, vTotal:%u, modeFlags:%lu\n",
							(scr_res->modes + j)->vSyncStart, (scr_res->modes + j)->vSyncEnd,
							(scr_res->modes + j)->vTotal, (scr_res->modes + j)->modeFlags);
						BLTS_DEBUG ("\n");
						sleep (2);

					}
				}
			}

			BLTS_DEBUG ("Restoring original mode ......\n");
			err =
				XRRSetCrtcConfig (params.display, scr_res, output_info->crtc,
				CurrentTime, 0, 0, origin_mode, RR_Rotate_0, crtc_info->outputs,
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
