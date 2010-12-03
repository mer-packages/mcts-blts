/*
 * software RGB to RGB converter
 * pluralize by software PAL8 to RGB converter
 *              software YUV to YUV converter
 *              software YUV to RGB converter
 * Written by Nick Kurshev.
 * palette & YUV & runtime CPU stuff by Michael (michaelni@gmx.at)
 *
 * Copyright (C) 2000-2010, Nokia Corporation.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * The C code (not assembly, MMX, ...) of this file can be used
 * under the LGPL license.
 */
#include <inttypes.h>
#include "config.h"
#include "libavutil/bswap.h"
#include "rgb2rgb.h"
#include "swscale.h"
#include "swscale_internal.h"

#define FAST_BGR2YV12 // use 7-bit instead of 15-bit coefficients

void (*rgb24tobgr32)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb24tobgr16)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb24tobgr15)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32tobgr24)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32to16)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32to15)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb15to16)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb15tobgr24)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb15to32)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb16to15)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb16tobgr24)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb16to32)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb24tobgr24)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb24to16)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb24to15)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32tobgr32)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32tobgr16)(const uint8_t *src, uint8_t *dst, long src_size);
void (*rgb32tobgr15)(const uint8_t *src, uint8_t *dst, long src_size);

void (*sbggr8torgb24)(const uint8_t *src, uint8_t *dst, int width, int height);
void (*sbggr16torgb24)(const uint8_t *src, uint8_t *dst, int width, int height);
void (*sgrbg10torgb24)(const uint8_t *src, uint8_t *dst, int width, int height);
void (*sgrbg10pixeltorgb24)(const uint8_t *src,
		int x, int y, int width, int height, uint8_t* r, uint8_t* g, uint8_t* b);

void (*yv12toyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                   long width, long height,
                   long lumStride, long chromStride, long dstStride);
void (*yv12touyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                   long width, long height,
                   long lumStride, long chromStride, long dstStride);
void (*yuv422ptoyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                      long width, long height,
                      long lumStride, long chromStride, long dstStride);
void (*yuv422ptouyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                      long width, long height,
                      long lumStride, long chromStride, long dstStride);
void (*yuy2toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                   long width, long height,
                   long lumStride, long chromStride, long srcStride);
void (*rgb24toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                    long width, long height,
                    long lumStride, long chromStride, long srcStride);
void (*planar2x)(const uint8_t *src, uint8_t *dst, long width, long height,
                 long srcStride, long dstStride);
void (*interleaveBytes)(uint8_t *src1, uint8_t *src2, uint8_t *dst,
                        long width, long height, long src1Stride,
                        long src2Stride, long dstStride);
void (*vu9_to_vu12)(const uint8_t *src1, const uint8_t *src2,
                    uint8_t *dst1, uint8_t *dst2,
                    long width, long height,
                    long srcStride1, long srcStride2,
                    long dstStride1, long dstStride2);
void (*yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                     uint8_t *dst,
                     long width, long height,
                     long srcStride1, long srcStride2,
                     long srcStride3, long dstStride);

#define RGB2YUV_SHIFT 8
#define BY ((int)( 0.098*(1<<RGB2YUV_SHIFT)+0.5))
#define BV ((int)(-0.071*(1<<RGB2YUV_SHIFT)+0.5))
#define BU ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define GY ((int)( 0.504*(1<<RGB2YUV_SHIFT)+0.5))
#define GV ((int)(-0.368*(1<<RGB2YUV_SHIFT)+0.5))
#define GU ((int)(-0.291*(1<<RGB2YUV_SHIFT)+0.5))
#define RY ((int)( 0.257*(1<<RGB2YUV_SHIFT)+0.5))
#define RV ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define RU ((int)(-0.148*(1<<RGB2YUV_SHIFT)+0.5))

//Note: We have C, MMX, MMX2, 3DNOW versions, there is no 3DNOW + MMX2 one.
//plain C versions
#undef HAVE_MMX
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef HAVE_SSE2
#define HAVE_MMX 0
#define HAVE_MMX2 0
#define HAVE_AMD3DNOW 0
#define HAVE_SSE2 0
#define RENAME(a) a ## _C
#include "rgb2rgb_template.c"

/*
 RGB15->RGB16 original by Strepto/Astral
 ported to gcc & bugfixed : A'rpi
 MMX2, 3DNOW optimization by Nick Kurshev
 32-bit C version, and and&add trick by Michael Niedermayer
*/

void sws_rgb2rgb_init(int flags)
{
	(void)flags;

    rgb2rgb_init_C();
}

/**
 * Convert the palette to the same packet 32-bit format as the palette
 */
void palette8topacked32(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;

    for (i=0; i<num_pixels; i++)
        ((uint32_t *) dst)[i] = ((const uint32_t *) palette)[src[i]];
}

/**
 * Palette format: ABCD -> dst format: ABC
 */
void palette8topacked24(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;

    for (i=0; i<num_pixels; i++)
    {
        //FIXME slow?
        dst[0]= palette[src[i]*4+0];
        dst[1]= palette[src[i]*4+1];
        dst[2]= palette[src[i]*4+2];
        dst+= 3;
    }
}

/**
 * Palette is assumed to contain BGR16, see rgb32to16 to convert the palette.
 */
void palette8torgb16(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = ((const uint16_t *)palette)[src[i]];
}
void palette8tobgr16(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = bswap_16(((const uint16_t *)palette)[src[i]]);
}

/**
 * Palette is assumed to contain BGR15, see rgb32to15 to convert the palette.
 */
void palette8torgb15(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = ((const uint16_t *)palette)[src[i]];
}
void palette8tobgr15(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = bswap_16(((const uint16_t *)palette)[src[i]]);
}

void rgb32to24(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size >> 2;
    for (i=0; i<num_pixels; i++)
    {
        #ifdef WORDS_BIGENDIAN
            /* RGB32 (= A,B,G,R) -> BGR24 (= B,G,R) */
            dst[3*i + 0] = src[4*i + 1];
            dst[3*i + 1] = src[4*i + 2];
            dst[3*i + 2] = src[4*i + 3];
        #else
            dst[3*i + 0] = src[4*i + 2];
            dst[3*i + 1] = src[4*i + 1];
            dst[3*i + 2] = src[4*i + 0];
        #endif
    }
}

void rgb24to32(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    for (i=0; 3*i<src_size; i++)
    {
        #ifdef WORDS_BIGENDIAN
            /* RGB24 (= R,G,B) -> BGR32 (= A,R,G,B) */
            dst[4*i + 0] = 255;
            dst[4*i + 1] = src[3*i + 0];
            dst[4*i + 2] = src[3*i + 1];
            dst[4*i + 3] = src[3*i + 2];
        #else
            dst[4*i + 0] = src[3*i + 2];
            dst[4*i + 1] = src[3*i + 1];
            dst[4*i + 2] = src[3*i + 0];
            dst[4*i + 3] = 255;
        #endif
    }
}

void rgb16tobgr32(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        #ifdef WORDS_BIGENDIAN
            *d++ = 255;
            *d++ = (bgr&0x1F)<<3;
            *d++ = (bgr&0x7E0)>>3;
            *d++ = (bgr&0xF800)>>8;
        #else
            *d++ = (bgr&0xF800)>>8;
            *d++ = (bgr&0x7E0)>>3;
            *d++ = (bgr&0x1F)<<3;
            *d++ = 255;
        #endif
    }
}

void rgb16to24(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb16to24swap(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        bgr = (bgr >> 8) | (bgr << 8);
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb16tobgr16(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++)
    {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = (rgb>>11) | (rgb&0x7E0) | (rgb<<11);
    }
}

void rgb16tobgr15(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++)
    {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = (rgb>>11) | ((rgb&0x7C0)>>1) | ((rgb&0x1F)<<10);
    }
}

void rgb15tobgr32(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        #ifdef WORDS_BIGENDIAN
            *d++ = 255;
            *d++ = (bgr&0x1F)<<3;
            *d++ = (bgr&0x3E0)>>2;
            *d++ = (bgr&0x7C00)>>7;
        #else
            *d++ = (bgr&0x7C00)>>7;
            *d++ = (bgr&0x3E0)>>2;
            *d++ = (bgr&0x1F)<<3;
            *d++ = 255;
        #endif
    }
}

void rgb15to24(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb15to24swap(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        bgr = (bgr >> 8) | (bgr << 8);
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb15tobgr16(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++)
    {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = ((rgb&0x7C00)>>10) | ((rgb&0x3E0)<<1) | (rgb<<11);
    }
}

void rgb15tobgr15(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++)
    {
        unsigned br;
        unsigned rgb = ((const uint16_t*)src)[i];
        br = rgb&0x7c1F;
        ((uint16_t*)dst)[i] = (br>>10) | (rgb&0x3E0) | (br<<10);
    }
}

void bgr8torgb8(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    long num_pixels = src_size;
    for (i=0; i<num_pixels; i++)
    {
        unsigned b,g,r;
        register uint8_t rgb;
        rgb = src[i];
        r = (rgb&0x07);
        g = (rgb&0x38)>>3;
        b = (rgb&0xC0)>>6;
        dst[i] = ((b<<1)&0x07) | ((g&0x07)<<3) | ((r&0x03)<<6);
    }
}