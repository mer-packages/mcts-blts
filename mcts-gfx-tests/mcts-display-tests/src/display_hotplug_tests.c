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

int test_hotplug (double execution_time)
{
	UNUSED_PARAM (execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	int i;
	int j = 0;
	unsigned short origin_cnct = 0;
	unsigned short output_cnct = 0;
	unsigned short new_cnct[2] = { 0 };
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
//screen configure
	scr_conf = XRRGetScreenInfo (params.display, params.root_window);
	scr_res = XRRGetScreenResources (params.display, params.root_window);
	current_size = XRRConfigCurrentConfiguration (scr_conf, &current_rotation);
	current_rate = XRRConfigCurrentRate (scr_conf);
	sizes = XRRConfigSizes (scr_conf, &nsizes);
	BLTS_DEBUG ("\nCurrent:%dx%d, Rate:%d\n", (sizes + current_size)->width,
		(sizes + current_size)->height, current_rate);
//output
	do
	{
		output_cnct = 0;
		for (i = 0; i < scr_res->noutput; i++)
		{
			output_info =
				XRRGetOutputInfo (params.display, scr_res, *(scr_res->outputs + i));
			output_cnct ^= output_info->connection;
		}
		new_cnct[j % 2] = origin_cnct ^ output_cnct;
		if (j == 0)
			new_cnct[1] = new_cnct[0];
		j++;
		BLTS_DEBUG ("!! Please insert your connector !!\n");
		sleep (2);
	}
	while (!(new_cnct[0] ^ new_cnct[1]));

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
