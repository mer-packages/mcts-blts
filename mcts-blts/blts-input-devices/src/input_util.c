/* input_util.c -- input device utility functions

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>

#include <blts_log.h>
#include "input_util.h"

typedef struct
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} BITMAPINFOHEADER;

Display *input_open_display()
{
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		BLTS_DEBUG("XOpenDisplay(NULL) failed, DISPLAY environment variable "\
			"missing? Trying display :0.\n");
		display = XOpenDisplay(":0");
	}

	return display;
}

int input_create_window(struct window_struct *params, const char *window_name,
	int width, int height, int screen)
{
	int fs = 0;
	unsigned int mask;
	XSetWindowAttributes window_attr;

	memset(params, 0, sizeof(*params));

	params->display = input_open_display();
	if (!params->display) {
		BLTS_ERROR("XOpenDisplay failed\n");
		goto error_exit;
	}

	params->screen = screen;
	params->root_window = RootWindow(params->display, params->screen);
	if (!params->root_window) {
		BLTS_ERROR("No root window\n");
		goto error_exit;
	}

	params->visual = DefaultVisual(params->display, params->screen);

	if (!XGetWindowAttributes(params->display, params->root_window,
		&params->root_window_attributes)) {
		BLTS_ERROR("XGetWindowAttributes failed\n");
		goto error_exit;
	}

	if (!width || !height)
		fs = 1;

	if (width)
		params->width = width;
	else
		params->width = params->root_window_attributes.width;

	if (height)
		params->height = height;
	else
		params->height = params->root_window_attributes.height;

	if (!XGetWindowAttributes(params->display, params->root_window,
		&params->window_attributes)) {
		BLTS_ERROR("XGetWindowAttributes failed\n");
		goto error_exit;
	}

	mask = CWBackPixel | CWBorderPixel | CWEventMask;
	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.event_mask = 0;
	if(!width && !height) {
		mask = mask | CWOverrideRedirect | CWSaveUnder | CWBackingStore;
		window_attr.override_redirect = True;
		window_attr.save_under = False;
		window_attr.backing_store = NotUseful;
	}

	params->window = XCreateWindow(params->display,
		RootWindow(params->display, screen), 0, 0,
		params->width, params->height, 0, CopyFromParent,
		InputOutput, CopyFromParent, mask, &window_attr);

	if (!XMapWindow(params->display, params->window)) {
		BLTS_ERROR("XMapWindow failed\n");
		goto error_exit;
	}

	if (!XRaiseWindow(params->display, params->window)) {
		BLTS_ERROR("XRaiseWindow failed\n");
		goto error_exit;
	}

	if (!XStoreName(params->display, params->window, window_name)) {
		BLTS_ERROR("XStoreName failed\n");
		goto error_exit;
	}

	params->gc = XCreateGC(params->display, params->window, 0, 0);
	if (!params->gc) {
		BLTS_ERROR("XCreateGC failed\n");
		goto error_exit;
	}

	XFlush(params->display);

	return 0;

error_exit:
	input_close_window(params);
	return -1;
}

int input_close_window(struct window_struct *params)
{
	if (params->display) {
		if (params->window) {
			XUnmapWindow(params->display, params->window);
			XDestroyWindow(params->display, params->window);
		}

		if (params->gc)
			XFreeGC(params->display, params->gc);

		XFlush(params->display);
		XCloseDisplay(params->display);
	}

	memset(params, 0, sizeof(*params));

	return 0;
}

int input_write_bmp(struct window_struct *ws, const char* filename, XImage *img)
{
	int x,y, k = 0;
	XColor col, old_col;
	const int bitmapdatasize = img->width * img->height * 3;
	FILE *fp;

	BLTS_DEBUG("Writing bitmap...\n");

	unsigned char *pTemp = (unsigned char *) malloc( bitmapdatasize );
	if (!pTemp)
		return -errno;

	old_col.pixel = 0;
	for (y = 0; y < img->height; y++) {
		for (x = 0; x < img->width; x++) {
			col.pixel = XGetPixel(img, x, y);
			if (old_col.pixel != col.pixel)
				XQueryColor(ws->display,
					ws->window_attributes.colormap, &col);
			old_col = col;
			pTemp[k++] = old_col.blue;
			pTemp[k++] = old_col.green;
			pTemp[k++] = old_col.red;
		}
	}

	fp = fopen( filename, "wb" );
	if (!fp)
	{
		free( pTemp );
		return -1;
	}

	short type = 19778;
	fwrite(&type, 1, 2, fp);
	int size = 54 + bitmapdatasize;
	fwrite(&size, 1, 4, fp);
	short dummy[2] = {0, 0};
	fwrite(dummy, 1, 4, fp);
	int offset = 54;
	fwrite(&offset, 1, 4, fp);

	BITMAPINFOHEADER info;
	memset(&info, 0, sizeof(BITMAPINFOHEADER));
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = img->width;
	info.biHeight = img->height;
	info.biPlanes = 1;
	info.biBitCount = 24;
	fwrite(&info, 1, sizeof(BITMAPINFOHEADER), fp);

	fwrite(pTemp, 1, bitmapdatasize, fp);

	free(pTemp);
	fclose(fp);

	BLTS_DEBUG("done.\n");

	return 0;
}

