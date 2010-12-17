/* display_enumerate_tests.c -- list informations of Screen/Crtc/Output/Modeline

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

int test_enumerate (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	int i, j, k;
//Screen
	XRRScreenConfiguration *scr_conf = NULL;
	XRRScreenResources *scr_res = NULL;
	int nsizes = 0;
	SizeID current_size = 0;
	XRRScreenSize *sizes = NULL;
	short current_rate = 0;
	Rotation current_rotation = 0;
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
		RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask |
		RROutputChangeNotifyMask | RROutputPropertyNotifyMask);
//screen configure
	scr_conf = XRRGetScreenInfo (params.display, params.root_window);
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	current_size = XRRConfigCurrentConfiguration (scr_conf, &current_rotation);
	current_rate = XRRConfigCurrentRate (scr_conf);
	sizes = XRRConfigSizes (scr_conf, &nsizes);
	LOG ("\nCurrent:%dx%d, Rate:%d\n", (sizes + current_size)->width,
		(sizes + current_size)->height, current_rate);
//output
	for (i = 0; i < scr_res->noutput; i++)
	{
		output_info =
			XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
		LOG ("Output Name=%s, ID=0x%lx, %s\n", output_info->name,
			*(scr_res->outputs + i),
			(output_info->connection) ? "disconnected" : "connected");
		LOG ("	Width(millimeters)=%u, Height(millimeters)=%u, Crtc=0x%lx\n",
			output_info->mm_width, output_info->mm_height, output_info->crtc);
		LOG ("	ncrtc:%d, RRCrtc ID: ", output_info->ncrtc);
		for (j = 0; j < output_info->ncrtc; j++)
			LOG ("0x%lx, ", *(output_info->crtcs + j));
		LOG ("\n");
		LOG ("	Preferred RRMode:\n");

		for (k = 0; k < output_info->nmode; k++)
			for (j = 0; j < scr_res->nmode; j++)
			{
				if (*(output_info->modes + k) == (scr_res->modes + j)->id)
				{
					LOG	("	-id:0x%lx, name:%s, size:%ux%u, refresh rate:%d, dotClock:%lu\n",
						(scr_res->modes + j)->id, (scr_res->modes + j)->name,
						(scr_res->modes + j)->width, (scr_res->modes + j)->height,
						REFRESH_RATE (((scr_res->modes + j)->dotClock),
							((scr_res->modes + j)->hTotal), ((scr_res->modes + j)->vTotal)),
						(scr_res->modes + j)->dotClock);
					LOG ("		   hSyncStart:%u, hSyncEnd:%u, hTotal:%u, hSkew:%u\n",
						(scr_res->modes + j)->hSyncStart, (scr_res->modes + j)->hSyncEnd,
						(scr_res->modes + j)->hTotal, (scr_res->modes + j)->hSkew);
					LOG ("		   vSyncStart:%u, vSyncEnd:%u, vTotal:%u, modeFlags:%lu\n",
						(scr_res->modes + j)->vSyncStart, (scr_res->modes + j)->vSyncEnd,
						(scr_res->modes + j)->vTotal, (scr_res->modes + j)->modeFlags);
				}
			}
	}

cleanup:
	if (scr_conf)
		XRRFreeScreenConfigInfo (scr_conf);
	if (scr_res)
		XRRFreeScreenResources (scr_res);
	if (output_info)
		XRRFreeOutputInfo (output_info);
	close_window (&params);
	return ret;
}
