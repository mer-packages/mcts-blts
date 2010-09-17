/* xrandr_tests.c -- Tests for X Randr extension

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
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>

#include <X11/extensions/Xrandr.h> 
#include "xrandr_tests.h"
#include "blts_x11_util.h"


int xrandr_rotate_screen(double execution_time)
{
	UNUSED_PARAM(execution_time);
	int ret = 0;
	int ver, rev, event_base, error_base;
	window_struct params;
	XRRScreenConfiguration* conf = NULL;
	short original_rate;
	Rotation original_rotation;
	SizeID original_size_id;
	Rotation rotations = 0;
	Status err;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}
	
	if(!XRRQueryExtension(params.display, &event_base, &error_base))
	{
		LOGERR("XRRQueryExtension failed\n");
		ret = -1;
		goto cleanup;
	}

	if(!XRRQueryVersion(params.display, &ver, &rev))
	{
		LOGERR("XRRQueryVersion failed\n");
		ret = -1;
		goto cleanup;
	}
	
	LOG("XRandR Extension version %i.%i\n", ver, rev);

	conf = XRRGetScreenInfo(params.display, params.root_window);
	original_rate = XRRConfigCurrentRate(conf);
	original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);
	rotations = XRRConfigRotations(conf, &original_rotation);
	
	if(rotations & RR_Rotate_0)
	{
		LOG("Rotation 0 degrees\n");
		err = XRRSetScreenConfigAndRate(params.display, conf, params.root_window,
			original_size_id, RR_Rotate_0, original_rate, CurrentTime);
		if(err != Success)
		{
			LOGERR("XRRSetScreenConfigAndRate failed (%d)\n", err);
			ret = -err;
		}
		sleep(2);
	}
	else
	{
		LOG("RR_Rotate_0 not supported\n");
	}

	if(rotations & RR_Rotate_90)
	{
		LOG("Rotation 90 degrees\n");
		err = XRRSetScreenConfigAndRate(params.display, conf, params.root_window,
			original_size_id, RR_Rotate_90, original_rate, CurrentTime);
		if(err != Success)
		{
			LOGERR("XRRSetScreenConfigAndRate failed (%d)\n", err);
			ret = -err;
		}
		sleep(2);
	}
	else
	{
		LOG("RR_Rotate_90 not supported\n");
	}

	if(rotations & RR_Rotate_180)
	{
		LOG("Rotation 180 degrees\n");
		err = XRRSetScreenConfigAndRate(params.display, conf, params.root_window,
			original_size_id, RR_Rotate_180, original_rate, CurrentTime);
		if(err != Success)
		{
			LOGERR("XRRSetScreenConfigAndRate failed (%d)\n", err);
			ret = -err;
		}
		sleep(2);
	}
	else
	{
		LOG("RR_Rotate_180 not supported\n");
	}

	if(rotations & RR_Rotate_270)
	{
		LOG("Rotation 270 degrees\n");
		err = XRRSetScreenConfigAndRate(params.display, conf, params.root_window,
			original_size_id, RR_Rotate_270, original_rate, CurrentTime);
		if(err != Success)
		{
			LOGERR("XRRSetScreenConfigAndRate failed (%d)\n", err);
			ret = -err;
		}
		sleep(2);
	}
	else
	{
		LOG("RR_Rotate_270 not supported\n");
	}

	LOG("Restore original configuration\n");
	err = XRRSetScreenConfigAndRate(params.display, conf, params.root_window,
		original_size_id, original_rotation, original_rate, CurrentTime);
	if(err != Success)
	{
		LOGERR("XRRSetScreenConfigAndRate failed (%d)\n", err);
		ret = -err;
	}

cleanup:
	if(conf)
	{
		XRRFreeScreenConfigInfo(conf);
	}
	close_window(&params);	
	return ret;
}

