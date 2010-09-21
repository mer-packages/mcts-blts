/* blts-bayer.h -- pixel format changes

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

#ifndef _LDX_BAYER_H_
#define _LDX_BAYER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdbool.h>



#include <asm/types.h>
#include <linux/videodev2.h>


#define RED(x) ((x >> 3) << 11)
#define GREEN(x) ((x >> 2) << 5)
#define BLUE(x) ((x >> 3))
#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))

#define RGB888toRGB565(r,g,b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))
#define RGB888toRGB8880(r,g,b) ((r << 16) | (g << 8) | (b))

// framebuffer expected values
#define DEFAULT_FB_WIDTH	800
#define DEFAULT_FB_HEIGHT	480


/**
 * Header is subject to change, ignore commented out lines
 */

//void bayer_to_565(const unsigned int *in, int width, int height, unsigned short* out_data, int o_w, int o_h, int clamp_width, int clamp_height, int line_w);
//static void put_pixel(unsigned short pix, unsigned short *p, const unsigned short w, const unsigned short h, const unsigned short width);

//void bayer_to_565_bw(const unsigned short *in, int width, int height, unsigned short* out_data, int fb_width, int fb_height, int line_w);
//void bayer_to_565_bw(const unsigned int *in, int width, int height, unsigned short* out_data, int fb_width, int fb_height, int line_w);
//void bayer_to_8880_bw(const unsigned short *in, int width, int height, unsigned short* out_data, int o_w, int o_h, int clamp_width, int clamp_height, int line_w);
//void bayer_to_888_bw(const unsigned short *in, int width, int height, unsigned char* out_data, int fb_width, int fb_height);

//void bayer_to_565(const unsigned char *in, int width, int height, unsigned short* out_data, int fb_width, int fb_height, int line_w);

void yuyv_to_565(const unsigned char *in, int width, int height, unsigned short* out_data, int fb_width, int fb_height, int line_w);
void yuyv_to_888(const unsigned char *in, int width, int height, unsigned char* out_data, int fb_width, int fb_height, int line_w);
void yuyv_to_8880(const unsigned char *in, int width, int height, unsigned char* out_data, int fb_width, int fb_height, int line_w);


#endif

