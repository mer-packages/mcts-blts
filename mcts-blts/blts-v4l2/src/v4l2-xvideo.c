/* v4l2-xvideo.c -- XVideo utils for tests

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

/* XVideo headers */ 
#include <ctype.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>			 /* getopt_long() */

#include <fcntl.h>			  /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/shm.h>

#include "v4l2-xvideo.h"

#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */

static window_struct params;
static XvImage* yuv_image = NULL;
static int xv_port = -1;
static unsigned int w_ret, h_ret, bw_ret, depth_ret;

static XvAdaptorInfo *ainfo = NULL;
static XShmSegmentInfo	yuv_shminfo;

Display* open_display()
{
	Display *display = NULL;

	display = XOpenDisplay(NULL);
	if(!display)
	{
		/* DISPLAY environment variable may not exist on non-posix-conformant systems */
		printf("XOpenDisplay(NULL) failed, DISPLAY environment variable missing? Trying display :0.\n");
		display = XOpenDisplay(":0");
		if(!display)
		{
			return NULL;
		}
	}
	
	return display;
}


int close_window(window_struct* params)
{
	if(params->display)
	{
		if(params->window)
		{
			XUnmapWindow(params->display, params->window);
			XDestroyWindow(params->display, params->window);
		}
		
		if(params->gc)
		{
			XFreeGC(params->display, params->gc);
		}
		
		XFlush(params->display);
		XCloseDisplay(params->display);
	}

	memset(params, 0, sizeof(window_struct));
	
	return 0;
}

int create_window(window_struct* params, const char* window_name)
{
	return create_window_ex(params, window_name, 0, 0);
}

int create_window_ex(window_struct* params, const char* window_name, int width, int height)
{
	XGCValues values;

	memset(params, 0, sizeof(window_struct));

	/* Open default display */
	params->display = open_display();
	if(!params->display)
	{
		printf("XOpenDisplay failed\n");
		goto cleanup;
	}

	params->screen = XDefaultScreen(params->display);
	params->root_window = RootWindow(params->display, params->screen);
	if(!params->root_window)
	{
		printf("No root window\n");
		goto cleanup;
	}
	
	params->visual = DefaultVisual(params->display, params->screen);
	
	if(!XGetWindowAttributes(params->display, params->root_window, 
		&params->root_window_attributes))
	{
		printf("XGetWindowAttributes failed\n");
		goto cleanup;
	}
	
	if(width)
	{
		params->width = width;
	}
	else
	{
		params->width = params->root_window_attributes.width;
	}

	if(height)
	{
		params->height = height;
	}
	else
	{
		params->height = params->root_window_attributes.height;
	}

	params->window = XCreateSimpleWindow(
		params->display,
		params->root_window,
		0, 0, params->width,
		params->height,
		0,0x0,0x00FFFFFF
		);
	if(!params->window)
	{
		printf("XCreateSimpleWindow failed\n");
		goto cleanup;
	}

	if(!XGetWindowAttributes(params->display, params->root_window,
		&params->window_attributes))
	{
		printf("XGetWindowAttributes failed\n");
		goto cleanup;
	}

	if(!XMapWindow(params->display, params->window))
	{
		printf("XMapWindow failed\n");
		goto cleanup;
	}
	
	if(!XRaiseWindow(params->display, params->window))
	{
		printf("XRaiseWindow failed\n");
		goto cleanup;
	}
	
	if(!XStoreName(params->display, params->window, 
		window_name))
	{
		printf("XStoreName failed\n");
		goto cleanup;
	}
	
	values.foreground = BlackPixel(params->display, params->screen);
	params->gc = XCreateGC(params->display, params->window, GCForeground, &values);
	if(!params->gc)
	{
		printf("XCreateGC failed\n");
		goto cleanup;
	}
	
	XFlush(params->display);
	
	return 0;
	
cleanup:
	close_window(params);
	return -1;
}


void xvideo_init(int cam_width, int cam_height, int img_format)

{
	int guid;	
	unsigned int nadaptors;
	Window root_ret;
	int x_ret, y_ret;

	if(create_window(&params, "Test Window"))
	{
		printf("Create window failed\n");
		exit (EXIT_FAILURE);
	}

	if(XvQueryAdaptors(params.display, RootWindow(params.display, 0), &nadaptors, &ainfo) != Success) 
	{
		printf("Unable to query for Xvideo adaptors\n");
		exit (EXIT_FAILURE);
	}

	if(nadaptors == 0) 
	{
		printf("No Xvideo adaptors\n");
		exit (EXIT_FAILURE);
	}

	xv_port = ainfo[0].base_id;

	if(!img_format)
		guid = FOURCC_YUV2;
	else
		guid = img_format;
	
	yuv_image = (XvImage*)XvShmCreateImage(params.display, xv_port, 
		guid, 0, cam_width, cam_height, &yuv_shminfo);

	yuv_shminfo.shmid = shmget(IPC_PRIVATE, yuv_image->data_size, 
		IPC_CREAT | 0777);
	yuv_shminfo.shmaddr = yuv_image->data = shmat(yuv_shminfo.shmid, 0, 0);
	yuv_shminfo.readOnly = False;

	if(!XShmAttach(params.display, &yuv_shminfo))
	{
		printf("XShmAttach failed!\n");
		exit (EXIT_FAILURE);
	}

	XGetGeometry(params.display, params.window, &root_ret, &x_ret, 
		&y_ret, &w_ret, &h_ret, &bw_ret, &depth_ret);
}

void xvideo_deinit()
{
	if(ainfo)
	{
		XvFreeAdaptorInfo(ainfo);
	}
	
	if(yuv_image)
	{
		XFree(yuv_image);
	}

	close_window(&params);
}

void xvideo_process_image(const void *p)
{
	if(!p)
		return;

	memcpy(yuv_image->data, p, yuv_image->data_size);

	XvShmPutImage(params.display, xv_port, params.window, params.gc, 
				yuv_image,
				0, 0, yuv_image->width, yuv_image->height,
				0, 0, w_ret, h_ret, True);

	XFlush(params.display);
	XSync(params.display, False);
}
