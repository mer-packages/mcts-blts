/* xinput_tests.c -- Tests for X Input extension

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

#include <X11/extensions/XInput.h>
#include <X11/extensions/XI.h>
#include "xinput_tests.h"
#include "blts_x11_util.h"

int xinput_list_devices()
{
	int ret = 0;
	int t, i, j;
	Display *display = NULL;
	XDeviceInfo* devices = NULL;
	int ndevices = 0;
	XExtensionVersion* version;
	XAnyClassPtr any_class;
	XAxisInfoPtr axis_info;

	/* Open default display */
	display = open_display();
	if(!display)
	{
		LOGERR("XOpenDisplay failed\n");
		ret = -1;
		goto cleanup;
	}

	/* TODO: XGetExtensionVersion is deprected */
	version = XGetExtensionVersion(display, INAME);
	if(!version || !version->present)
	{
		LOGERR("XGetExtensionVersion failed\n");
		ret = -1;
		goto cleanup;
	}
	LOG("XInput Extension version %i.%i\n", version->major_version, 
		version->minor_version);

	devices = XListInputDevices(display, &ndevices);
	if(!devices)
	{
		LOGERR("XListInputDevices failed\n");
		ret = -1;
		goto cleanup;
	}
	
	if(ndevices == 0)
	{
		LOGERR("No input devices found\n");
		ret = -1;
		goto cleanup;
	}
	
	LOG("Found %d input devices\n", ndevices);
	for(t = 0; t < ndevices; t++)
	{
		LOG("id: %d\n", devices[t].id);
		LOG("\tname: %s\n", devices[t].name);
		if(devices[t].use == IsXKeyboard)
		{
			LOG("\tuse: keyboard\n");
		}
		else if(devices[t].use == IsXPointer)
		{
			LOG("\tuse: pointer\n");
		}
		else if(devices[t].use == IsXExtensionKeyboard)
		{
			LOG("\tuse: extension keyboard\n");
		}
		else if(devices[t].use == IsXExtensionPointer)
		{
			LOG("\tuse: extension pointer\n");
		}
		else
		{
			LOG("\tuse: extension device\n");
		}

		any_class = (XAnyClassPtr)(devices[t].inputclassinfo);
		for(i = 0; i < devices[t].num_classes; i++)
		{
			LOG("\tInput class %d:\n", i);
			switch (any_class->class)
			{
			case KeyClass:
				LOG("\t num_keys: %d\n", 
					((XKeyInfoPtr)devices[t].inputclassinfo)->num_keys);
				LOG("\t min_keycode: %d\n",
					((XKeyInfoPtr)devices[t].inputclassinfo)->min_keycode);
				LOG("\t max_keycode: %d\n",
					((XKeyInfoPtr)devices[t].inputclassinfo)->max_keycode);
				break;

			case ButtonClass:
				LOG("\t num_buttons: %d\n",
					((XButtonInfoPtr)any_class)->num_buttons);
				break;

			case ValuatorClass:
				LOG("\t num_axes: %d\n", 
					((XValuatorInfoPtr)any_class)->num_axes);
				LOG("\t mode: %s\n", 
					(((XValuatorInfoPtr)any_class)->mode == Absolute) ?
						"absolute" : "relative");
				LOG("\t motion_buffer: %ld\n",
					((XValuatorInfoPtr)any_class)->motion_buffer);

				axis_info = (XAxisInfoPtr)((char *)((XValuatorInfoPtr)any_class)
					+ sizeof(XValuatorInfo));
				
				for(j = 0; j < ((XValuatorInfoPtr)any_class)->num_axes; j++)
				{
					LOG("\t axis %d:\n", j);
					LOG("\t min_value: %d\n", axis_info->min_value);
					LOG("\t max_value: %d\n", axis_info->max_value);
					LOG("\t resolution: %d\n", axis_info->resolution);
					axis_info++;
				}
				break;

			default:
				LOG("\t unknown class\n");
				break;
			}
			any_class = (XAnyClassPtr)((char*)any_class + any_class->length);
		}
	}

cleanup:
	if(devices)
	{
		XFreeDeviceList(devices);
	}
	if(display)
	{
		XCloseDisplay(display);
	}
	return ret;
}

