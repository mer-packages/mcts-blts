/* blts-bayer.c -- pixel format changes

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


#include "blts-bayer.h"

/**
 * getpixel 16bits
 */

static inline unsigned char get_pixel(const uint16_t *p, const unsigned short w, const unsigned short h, const unsigned short width)
{
	const unsigned int pos = h * width + w;

	return p[pos];
}


/**
 * getpixel for 32bits
 */

static inline unsigned char get_pixel_32(const uint32_t *p, const unsigned short w, const unsigned short h, const unsigned short width)
{
	const unsigned int pos = h * width + w ;
	return p[pos];
}



/**
 * Put pixel
 * Calculates position of pixel in bayer and inserts data to it
 * @param unsigned short pix pixel value
 * @param short* p bayer layer
 * @param short w x position of pixel
 * @param short h y position of pixel
 * @param short width width of layer
 */

static inline void put_pixel(unsigned short pix, uint16_t *p, const unsigned short w, const unsigned short h, const unsigned short width)
{
	if(h>0 && h<DEFAULT_FB_HEIGHT && w>0 && w<DEFAULT_FB_WIDTH)
		p[h*width + w] = pix;
}

/**
 * putpixel for 32bits
 */

static inline void put_pixel_32(unsigned int pix, uint32_t *p, const unsigned short w, const unsigned short h, const unsigned short width)
{
	if(h>0 && h<DEFAULT_FB_HEIGHT && w>0 && w<DEFAULT_FB_WIDTH)
		p[h*width + w] = pix;
}


/**
 * changes YUYV surface to rbg 16bit
 */

void bayer_to_565(const uint32_t *in, int width, int height, uint16_t* out_data, int o_w, int o_h, int clamp_width, int clamp_height, int line_w)
{
	unsigned short x,y;
	for(y = 0; y < DEFAULT_FB_HEIGHT; y++)
	{
		for(x = 0; x < DEFAULT_FB_WIDTH; x++)
		{
			int pixel = get_pixel_32(in, x,y, o_w);

			put_pixel(pixel, out_data ,x,y,line_w);
		}
	}
}


// --------------------------8<--------------------------------

void YCbCr2RGB(int Y, int Cb, int Cr, int* r, int* g, int*b)
{
	*b = 1.164 * (Y - 16) + 2.018 * (Cb - 128);
	*g = 1.164 * (Y - 16) - 0.813 * (Cr - 128) - 0.391 * (Cb - 128);
	*r = 1.164 * (Y - 16) + 1.596 * (Cr - 128);
	*r = *r > 255 ? 255 : *r;
	*g = *g > 255 ? 255 : *g;
	*b = *b > 255 ? 255 : *b;
	*r = *r < 0 ? 0 : *r;
	*g = *g < 0 ? 0 : *g;
	*b = *b < 0 ? 0 : *b;
}

/**
 * YUYV surface to 24bit rgb colour surface
 */

void yuyv_to_888(const uint8_t *in, int width, int height, uint8_t* out_data, int fb_width, int fb_height, int line_w)
{
	int x,y, xstep, ystep, ys;
	int r, g, b;
	unsigned char y0, cb0, y1, cr0;

	xstep = (width << 16) / fb_width;
	ystep = (height << 16) / fb_height;

	for(y = 0; y < fb_height; y++)
	{
		ys = (y * ystep >> 16) * width << 1;
		for(x = 0; x < fb_width; x+=2)
		{
			int pos = (x * xstep >> 15) + ys;
			pos >>= 2; pos <<= 2;
			y0 = in[pos];
			cb0 = in[pos + 1];
			y1 = in[pos + 2];
			cr0 = in[pos + 3];

			YCbCr2RGB(y0, cb0, cr0, &r, &g, &b);
			pos = (x + y * line_w) * 3;
			out_data[pos++] = r;
			out_data[pos++] = g;
			out_data[pos++] = b;

			YCbCr2RGB(y1, cb0, cr0, &r, &g, &b);
			out_data[pos++] = r;
			out_data[pos++] = g;
			out_data[pos] = b;
		}
	}
}

/**
 * YUYV surface to 32bit rgb colour surface
 */

void yuyv_to_8880(const uint8_t *in, int width, int height, uint8_t* out_data, int fb_width, int fb_height, int line_w)
{
	int x,y, xstep, ystep, ys;
	int r, g, b;
	unsigned char y0, cb0, y1, cr0;

	xstep = (width << 16) / fb_width;
	ystep = (height << 16) / fb_height;

	for(y = 0; y < fb_height; y++)
	{
		ys = (y * ystep >> 16) * width << 1;
		for(x = 0; x < fb_width; x+=2)
		{
			int pos = (x * xstep >> 15) + ys;
			pos >>= 2; pos <<= 2;
			y0 = in[pos];
			cb0 = in[pos + 1];
			y1 = in[pos + 2];
			cr0 = in[pos + 3];

			YCbCr2RGB(y0, cb0, cr0, &r, &g, &b);
			pos = (x + y * line_w) * 4;
			out_data[pos++] = r;
			out_data[pos++] = g;
			out_data[pos++] = b;
			pos++;

			YCbCr2RGB(y1, cb0, cr0, &r, &g, &b);
			out_data[pos++] = r;
			out_data[pos++] = g;
			out_data[pos] = b;
		}
	}
}

/**
 * YUYV surface to 16bit rgb colour surface
 */

void yuyv_to_565(const uint8_t *in, int width, int height, uint16_t* out_data, int fb_width, int fb_height, int line_w)
{
	int x,y, xstep, ystep, ys;
	int r, g, b;
	unsigned char y0, cb0, y1, cr0;
	unsigned short pixel;

	xstep = (width << 16) / fb_width;
	ystep = (height << 16) / fb_height;

	for(y = 0; y < fb_height; y++)
	{
		ys = (y * ystep >> 16) * width << 1;
		for(x = 0; x < fb_width; x+=2)
		{
			int pos = (x * xstep >> 15) + ys;
			pos >>= 2; pos <<= 2;
			y0 = in[pos];
			cb0 = in[pos + 1];
			y1 = in[pos + 2];
			cr0 = in[pos + 3];

			YCbCr2RGB(y0, cb0, cr0, &r, &g, &b);
			pixel = GREEN(g) | RED(r) | BLUE(b);
			put_pixel(pixel, out_data, x, y, line_w);

			YCbCr2RGB(y1, cb0, cr0, &r, &g, &b);
			pixel = GREEN(g) | RED(r) | BLUE(b);
			put_pixel(pixel, out_data, x + 1, y, line_w);
		}
	}
}


// -------------------------->8--------------------------------



/**
 * changes YUYV surface to 16 bit black and white screen
 */


void bayer_to_565_bw(const uint16_t *in, int width, int height, uint16_t* out_data, int fb_width, int fb_height, int line_w)
{
	unsigned short x,y;
	unsigned short *out = (unsigned short *)out_data;
	float rx, ry, xstep, ystep;
	rx = 0;
	ry = 0;
	xstep = (float)width / (float)fb_width;
	ystep = (float)height / (float)fb_height;

	for(y = 0; y < DEFAULT_FB_WIDTH; y++)
	{
		rx = 0;
		for(x = 0; x < DEFAULT_FB_WIDTH; x++)
		{
			const unsigned char val = get_pixel(in, (short)(rx), (short)(ry), width);
			const unsigned short pixel = GREEN(val) | RED(val) | BLUE(val);
			put_pixel( pixel, out, x, y, line_w);
			rx = rx+xstep;
		}
		ry= ry + ystep;

	}
}

/**
 * changes YUYV buffer to 8880 black and white surface
 */

void bayer_to_8880_bw(const uint16_t *in, int width, int height, uint16_t* out_data, int o_w, int o_h, int clamp_width, int clamp_height, int line_w)
{
	unsigned short x,y;
	unsigned short start_x;

	uint32_t *out = (uint32_t *) out_data;
	unsigned short c_height = MIN(clamp_height, o_h);
	c_height = MIN(c_height, height);

	unsigned short c_width = MIN(clamp_width, o_w);
	c_width = MIN(c_width, width);

	start_x = 2;

	// Landscape needs this;
	if(width > height)
		start_x = 3;

	for(y = 0; y < DEFAULT_FB_HEIGHT ; y++)
	{
		for(x = 0; x < DEFAULT_FB_WIDTH; x++)
		{
			const unsigned char val = get_pixel(in, x, y, o_w);
			const unsigned short pixel = RGB888toRGB8880(val, val, val);


			put_pixel_32( pixel, out, x, y, line_w);

		}
	}
}



