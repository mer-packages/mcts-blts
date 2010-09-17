/* xvideo_util.h -- XVideo utility functions

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

#ifndef XVIDEO_UTIL_H
#define XVIDEO_UTIL_H

#include <stdio.h>
#include <inttypes.h>
#include <X11/Xutil.h>

struct window_struct
{
	Display *display;
	Window root_window;
	Window window;
	XWindowAttributes root_window_attributes;
	XWindowAttributes window_attributes;
	long screen;
	GC gc;
	Visual *visual;
	unsigned width;
	unsigned height;
};

struct xv_rgb32_image
{
	uint32_t w;
	uint32_t h;
	uint32_t *buf;
};

struct xv_pix_fmt {
	char fourcc[5];
	unsigned fmt;
	unsigned eff_bpp;
	unsigned mpix_bpp;
};

enum xv_test_images {
	XV_TI_UNKNOWN = 0,
	XV_TI_RANDOM,
	XV_TI_VH_LINES,
	XV_TI_CHECKERBOARD,
	XV_TI_MOVING_ARROW,
	XV_TI_GRAYGRADIENT,
	XV_TI_COLORGRADIENT,
	XV_TI_GRAYGRADIENT_SCROLL,
	XV_TI_COLORGRADIENT_SCROLL,
	XV_TI_FPS_CHECK
};

typedef struct
{
	char name[64];
	int value;
} port_attributes;

typedef struct
{
	int src_width;
	int src_height;
	int dst_width;
	int dst_height;
	int screen;
	unsigned adaptor;
	char adaptor_name[256];
	char fourcc[5];
	int duration;
	unsigned num_images;
	enum xv_test_images test_images;
	port_attributes attrs[32];
	int num_attrs;

	int port;
	unsigned int id;
	unsigned int bpp;
} xvideo_data;

Display* open_display();
int xv_close_window(struct window_struct* params);
int xv_create_window(struct window_struct* params,
	const char* window_name, int width, int height, int screen);

enum xv_test_images string_to_test_images(const char *str);
struct xv_pix_fmt *xv_get_format(const char *fourcc);

struct xv_rgb32_image *xv_alloc_rgb_img(uint32_t w, uint32_t h);
void xv_free_rgb_img(struct xv_rgb32_image *img);
void xv_clear_rgb_img(struct xv_rgb32_image *img);
void xv_draw_ellipse(struct xv_rgb32_image *img, float x, float y,
	float a, float b, float angle, uint32_t color);
void xv_draw_line(struct xv_rgb32_image *img, uint32_t x0, uint32_t y0,
	uint32_t x1, uint32_t y1, uint32_t color);

void xv_generate_vh_lines(struct xv_rgb32_image *img, unsigned frame);
void xv_generate_checkerboard(struct xv_rgb32_image *img, unsigned frame,
	unsigned w, unsigned h);
void xv_generate_arrow(struct xv_rgb32_image *img, unsigned pos);
void xv_generate_fps_blocks(struct xv_rgb32_image *img, unsigned frame);
void xv_scroll_buf(uint8_t *orig, uint8_t *img, unsigned size,
	unsigned pitch, unsigned x_off);
void xv_generate_fs_graygradient(struct xv_rgb32_image *img);
void xv_generate_fs_colorgradient(struct xv_rgb32_image *img);

#endif /* XVIDEO_UTIL_H */

