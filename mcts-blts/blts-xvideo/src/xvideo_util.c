/* xvideo_util.c -- XVideo utility functions

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
#include <linux/videodev2.h>

#include <blts_log.h>

#include "xvideo_util.h"

static struct xv_pix_fmt fmt_table[] =
{
	{ "YUY2", V4L2_PIX_FMT_YUYV, 16, 32 },
	{ "YUYV", V4L2_PIX_FMT_YUYV, 16, 32 },
	{ "YUNV", V4L2_PIX_FMT_YUYV, 16, 32 },
	{ "V422", V4L2_PIX_FMT_YUYV, 16, 32 },
	{ "YV12", V4L2_PIX_FMT_YVU420, 12, 0 },
	{ "UYVY", V4L2_PIX_FMT_UYVY, 16, 32 },
	{ "I420", V4L2_PIX_FMT_YUV420, 12, 0 },
	{ "IYUV", V4L2_PIX_FMT_YUV420, 12, 0 },
	{ "", 0, 0, 0 },
};

#define WHITE_COL 0xFFFFFFFF
#define RGB_TO_COL(r, g, b) ((r << 16) | (g << 8) | b)

Display *open_display()
{
	Display *display = XOpenDisplay(NULL);
	if (!display) {
		BLTS_DEBUG("XOpenDisplay(NULL) failed, DISPLAY environment variable "\
			"missing? Trying display :0.\n");
		display = XOpenDisplay(":0");
	}

	return display;
}

int xv_create_window(struct window_struct *params, const char *window_name,
	int width, int height, int screen)
{
	unsigned int mask;
	XSetWindowAttributes window_attr;

	memset(params, 0, sizeof(*params));

	params->display = open_display();
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

	mask = CWBackPixel | CWBorderPixel | CWEventMask;
	if (!width && !height) {
		mask = mask | CWOverrideRedirect | CWSaveUnder | CWBackingStore;
		window_attr.override_redirect = True;
		window_attr.backing_store = NotUseful;
		window_attr.save_under = False;
	}

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.event_mask = StructureNotifyMask | ExposureMask;

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

	params->window = XCreateWindow(params->display,
		RootWindow(params->display, params->screen), 0, 0,
		params->width, params->height, 0, CopyFromParent, InputOutput,
		CopyFromParent, mask, &window_attr);

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
	xv_close_window(params);
	return -1;
}

int xv_close_window(struct window_struct *params)
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

void xv_setpixel(struct xv_rgb32_image *img, uint32_t x, uint32_t y,
	uint32_t color)
{
	if (x >= img->w || y >= img->h)
		return;
	img->buf[x + y * img->w] = color;
}

void xv_draw_line(struct xv_rgb32_image *img, uint32_t x0, uint32_t y0,
	uint32_t x1, uint32_t y1, uint32_t color)
{
	int dy = y1 - y0;
	int dx = x1 - x0;
	float t = (float) 0.5;

	xv_setpixel(img, x0, y0, color);
	if (abs(dx) > abs(dy)) {
		float m = (float) dy / (float) dx;
		t += y0;
		dx = (dx < 0) ? -1 : 1;
		m *= dx;
		while (x0 != x1) {
			x0 += dx;
			t += m;
			xv_setpixel(img, x0, (int) t, color);
		}
	} else {
		float m = (float) dx / (float) dy;
		t += x0;
		dy = (dy < 0) ? -1 : 1;
		m *= dy;
		while (y0 != y1) {
			y0 += dy;
			t += m;
			xv_setpixel(img, (int) t, y0, color);
		}
	}
}

void xv_draw_ellipse(struct xv_rgb32_image *img, float x, float y,
	float w, float h, float angle, uint32_t color)
{
	int i;
	float beta = -angle * M_PI / 180.0;
	float sinbeta = sin(beta);
	float cosbeta = cos(beta);
	float xp, yp, xl, yl;
	float alpha = 0;
	float sinalpha;
	float cosalpha;

	w /= 2;
	h /= 2;
	xl = x + w * cosbeta;
	yl = y + w * sinbeta;

	for (i = 0; i < 361; i++) {
		alpha = i * (M_PI / 180.0);
		sinalpha = sin(alpha);
		cosalpha = cos(alpha);
		xp = x + (w * cosalpha * cosbeta - h * sinalpha * sinbeta);
		yp = y + (w * cosalpha * sinbeta + h * sinalpha * cosbeta);
		xv_draw_line(img, xp, yp, xl, yl, color);
		xl = xp;
		yl = yp;
	}
}

void xv_draw_filled_rectangle(struct xv_rgb32_image *img,
	unsigned x, unsigned y, unsigned w, unsigned h, uint32_t color)
{
	unsigned x1, y1;

	for (y1 = y; y1 < y + h; y1++)
		for (x1 = x; x1 < x + w; x1++)
			xv_setpixel(img, x1, y1, color);
}

void xv_draw_filled_triangle(struct xv_rgb32_image *img, unsigned x, unsigned y,
	unsigned w, unsigned h, uint32_t color)
{
	unsigned x1, y1, i = 0;
	unsigned x_off = w;
	float ar = (float)w / (float)h;
	x += w / 2;

	for (y1 = y; y1 < y + h; y1++) {
		x_off = ar * ++i / 2;
		for (x1 = x - x_off; x1 < x + x_off; x1++)
			xv_setpixel(img, x1, y1, color);
	}
}

void xv_clear_rgb_img(struct xv_rgb32_image *img)
{
	if (img && img->buf)
		memset(img->buf, 0, img->w * img->h * sizeof(uint32_t));
}

struct xv_rgb32_image *xv_alloc_rgb_img(uint32_t w, uint32_t h)
{
	struct xv_rgb32_image *img;

	img = malloc(sizeof(*img));
	if (!img)
		return NULL;

	img->buf = malloc(w * h * sizeof(uint32_t));
	if (!img->buf) {
		free(img);
		return NULL;
	}

	img->w = w;
	img->h = h;

	return img;
}

void xv_free_rgb_img(struct xv_rgb32_image *img)
{
	if (img) {
		if(img->buf)
			free(img->buf);
		free(img);
	}
}

void xv_generate_vh_lines(struct xv_rgb32_image *img, unsigned frame)
{
	unsigned i;

	xv_clear_rgb_img(img);

	if (!frame) {
		for (i = 0; i < img->h; i += 2)
			xv_draw_line(img, 0, i, img->w, i, WHITE_COL);
	} else {
		for (i = 0; i < img->w; i += 2)
			xv_draw_line(img, i, 0, i, img->h, WHITE_COL);
	}
}

void xv_generate_checkerboard(struct xv_rgb32_image *img, unsigned frame,
	unsigned w, unsigned h)
{
	unsigned x, y, sx = 0;

	if (frame)
		sx = w;

	xv_clear_rgb_img(img);
	for (y = 0; y < img->w; y += h) {
		sx = sx ? 0 : w;
		for (x = sx; x < img->w; x += w * 2)
			xv_draw_filled_rectangle(img, x, y, w, h, WHITE_COL);
	}
}

void xv_generate_arrow(struct xv_rgb32_image *img, unsigned pos)
{
	unsigned w, h;

	w = img->w / 10;
	h = img->h / 10;

	xv_clear_rgb_img(img);
	xv_draw_filled_triangle(img, pos, 0, w, h, WHITE_COL);
	xv_draw_filled_rectangle(img, pos + w / 3, h, w / 3,
		img->h - h, WHITE_COL);
}

void xv_generate_fps_blocks(struct xv_rgb32_image *img, unsigned frame)
{
	unsigned x, y;

	x = img->w / 4;
	y = img->h / 3;

	xv_clear_rgb_img(img);
	if (frame)
		xv_draw_filled_rectangle(img, x + x, y, x, y, WHITE_COL);
	else
		xv_draw_filled_rectangle(img, x, y, x, y, WHITE_COL);
}

void xv_scroll_buf(uint8_t *orig, uint8_t *img, unsigned size,
	unsigned pitch, unsigned x_off)
{
	unsigned i;

	for (i = 0; i < size; i += pitch) {
		memcpy(&img[i], &orig[i + pitch - x_off], x_off);
		memcpy(&img[i + x_off], &orig[i], pitch - x_off);
	}
}

void xv_generate_fs_graygradient(struct xv_rgb32_image *img)
{
	unsigned x;
	float c = 0.0;
	float c_step = 256.0 / img->w;

	for (x = 0; x < img->w; x++) {
		xv_draw_line(img, x, 0, x, img->h,
			RGB_TO_COL((uint8_t)c, (uint8_t)c, (uint8_t)c));
		c += c_step;
	}
}

void xv_generate_fs_colorgradient(struct xv_rgb32_image *img)
{
	unsigned x, h;
	float c = 0.0;
	float c_step = 256.0 / img->w;

	h = img->h / 3;
	for (x = 0; x < img->w; x++) {
		xv_draw_line(img, x, 0, x, h, RGB_TO_COL((uint8_t)c, 0, 0));
		xv_draw_line(img, x, h, x, h * 2, RGB_TO_COL(0, (uint8_t)c, 0));
		xv_draw_line(img, x, h * 2, x, img->h, RGB_TO_COL(0, 0, (uint8_t)c));
		c += c_step;
	}
}

enum xv_test_images string_to_test_images(const char *str)
{
	if (!str)
		return XV_TI_UNKNOWN;
	if (!strcmp(str, "random"))
		return XV_TI_RANDOM;
	if (!strcmp(str, "vh_lines"))
		return XV_TI_VH_LINES;
	if (!strcmp(str, "checkerboard"))
		return XV_TI_CHECKERBOARD;
	if (!strcmp(str, "moving_arrow"))
		return XV_TI_MOVING_ARROW;
	if (!strcmp(str, "fps_check"))
		return XV_TI_FPS_CHECK;
	if (!strcmp(str, "graygradient"))
		return XV_TI_GRAYGRADIENT;
	if (!strcmp(str, "colorgradient"))
		return XV_TI_COLORGRADIENT;
	if (!strcmp(str, "graygradient_scroll"))
		return XV_TI_GRAYGRADIENT_SCROLL;
	if (!strcmp(str, "colorgradient_scroll"))
		return XV_TI_COLORGRADIENT_SCROLL;
	return XV_TI_UNKNOWN;
}

struct xv_pix_fmt *xv_get_format(const char *fourcc)
{
	int i = 0;
	while (fmt_table[i].fmt) {
		if (!strcmp(fmt_table[i].fourcc, fourcc))
			return &fmt_table[i];
		i++;
	}

	return NULL;
}

