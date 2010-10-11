/*
 * software RGB to RGB converter
 * pluralize by software PAL8 to RGB converter
 *              software YUV to YUV converter
 *              software YUV to RGB converter
 * Written by Nick Kurshev.
 * palette & YUV & runtime CPU stuff by Michael (michaelni@gmx.at)
 * lot of big-endian byte order fixes by Alex Beregszaszi
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

#include <stddef.h>

static inline void RENAME(rgb24tobgr32)(const uint8_t *src, uint8_t *dst, long src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    end = s + src_size;
    while (s < end)
    {
    #ifdef WORDS_BIGENDIAN
        /* RGB24 (= R,G,B) -> RGB32 (= A,B,G,R) */
        *dest++ = 255;
        *dest++ = s[2];
        *dest++ = s[1];
        *dest++ = s[0];
        s+=3;
    #else
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = 255;
    #endif
    }
}

static inline void RENAME(rgb32tobgr24)(const uint8_t *src, uint8_t *dst, long src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    end = s + src_size;
    while (s < end)
    {
#ifdef WORDS_BIGENDIAN
        /* RGB32 (= A,B,G,R) -> RGB24 (= R,G,B) */
        s++;
        dest[2] = *s++;
        dest[1] = *s++;
        dest[0] = *s++;
        dest += 3;
#else
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        s++;
#endif
    }
}

/*
 original by Strepto/Astral
 ported to gcc & bugfixed: A'rpi
 MMX2, 3DNOW optimization by Nick Kurshev
 32-bit C version, and and&add trick by Michael Niedermayer
*/
static inline void RENAME(rgb15to16)(const uint8_t *src, uint8_t *dst, long src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    mm_end = end - 3;
    while (s < mm_end)
    {
        register unsigned x= *((const uint32_t *)s);
        *((uint32_t *)d) = (x&0x7FFF7FFF) + (x&0x7FE07FE0);
        d+=4;
        s+=4;
    }
    if (s < end)
    {
        register unsigned short x= *((const uint16_t *)s);
        *((uint16_t *)d) = (x&0x7FFF) + (x&0x7FE0);
    }
}

static inline void RENAME(rgb16to15)(const uint8_t *src, uint8_t *dst, long src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    mm_end = end - 3;
    while (s < mm_end)
    {
        register uint32_t x= *((const uint32_t*)s);
        *((uint32_t *)d) = ((x>>1)&0x7FE07FE0) | (x&0x001F001F);
        s+=4;
        d+=4;
    }
    if (s < end)
    {
        register uint16_t x= *((const uint16_t*)s);
        *((uint16_t *)d) = ((x>>1)&0x7FE0) | (x&0x001F);
        s+=2;
        d+=2;
    }
}

static inline void RENAME(rgb32to16)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>8);
    }
}

static inline void RENAME(rgb32tobgr16)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<8) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>19);
    }
}

static inline void RENAME(rgb32to15)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>9);
    }
}

static inline void RENAME(rgb32tobgr15)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<7) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>19);
    }
}

static inline void RENAME(rgb24tobgr16)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void RENAME(rgb24to16)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void RENAME(rgb24tobgr15)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void RENAME(rgb24to15)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    while (s < end)
    {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void RENAME(sbggr8torgb24)(const uint8_t *src, uint8_t *dst,
		int width, int height)
{
	/* row 0: B00 G01 B02 G03...
	   row 1: G10 R11 G12 R13...
	   row 2: B20 G21 B22 G23...
	   row 3: G30 R31 G32 R33...
	     .
	     .
	     .
	   */

    int row, col;
    for (row = 0; row < height; row++)
    {
    	for (col = 0; col < width; col++)
    	{
			int quadrant = ((row&1)<<4) | (col&1);
			int red=0, green=0, blue=0;
			int nr=0, ng=0, nb=0;
			const uint8_t* s = src + row * width + col;

			switch (quadrant)
			{
			case 0x00:
				// Interpolation pattern (current pixel in center):
				// R G R
				// G B G
				// R G R
				blue = s[0];
				red = s[width + 1];
				nr = 1;
				green = s[1] + s[width];
				ng = 2;
				if (col != 0)
				{
					red += s[width - 1];
					nr++;
					green += s[-1];
					ng++;
				}

				if (row != 0)
				{
					green += s[-width];
					ng++;
					red += s[-width + 1];
					nr++;
					if (col != 0)
					{
						red += s[-width - 1];
						nr++;
					}
				}

				red /= nr;
				green /= ng;
				break;
			case 0x01:
				// Interpolation pattern (current pixel in center):
				// x R x
				// B G B
				// x R x
				green = s[0];
				red = s[width];
				nr = 1;
				if (row != 0)
				{
					red += s[-width];
					nr++;
				}
				blue = s[-1];
				nb = 1;
				if (col != width - 1)
				{
					blue += s[1];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			case 0x10:
				// Interpolation pattern (current pixel in center):
				// x B x
				// R G R
				// x B x
				green = s[0];
				red = s[1];
				nr = 1;
				if (col != 0)
				{
					red += s[-1];
					nr++;
				}
				blue = s[-width];
				if (row != height - 1)
				{
					blue += s[width];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			case 0x11:
				// Interpolation pattern (current pixel in center):
				// B G B
				// G R G
				// B G B
				red = s[0];
				green =  s[-1] + s[-width];
				ng = 2;
				blue = s[-width - 1];
				nb = 1;
				if (col != width - 1)
				{
					green += s[1];
					ng++;
					blue += s[-width + 1];
					nb++;
				}
				if (row != height - 1)
				{
					green += s[width];
					ng++;
					blue += s[width - 1];
					nb++;
					if (col != width - 1)
					{
						blue += s[width + 1];
						nb++;
					}
				}
				green /= ng;
				blue /= nb;
				break;
			}
			unsigned char* rgbptr = dst + (row * width + col) * 3;
			rgbptr[0] = red;
			rgbptr[1] = green;
			rgbptr[2] = blue;
    	}
    }
}

static inline void RENAME(sgrbg10torgb24)(const uint8_t *src, uint8_t *dst,
		int width, int height)
{
	/* row 0: G00 R01 G02 R03...
	   row 1: B10 G11 B12 G13...
	   row 2: G20 R21 G22 R23...
	   row 3: B30 G31 B32 G33...
	     .
	     .
	     .
	   */

	uint16_t* src16 = (uint16_t*)src;
    int row, col;
    for (row = 0; row < height; row++)
    {
    	for (col = 0; col < width; col++)
    	{
			int quadrant = ((row&1)<<4) | (col&1);
			int red=0, green=0, blue=0;
			int nr=0, ng=0, nb=0;
			const uint16_t* s = src16 + row * width + col;

			switch (quadrant)
			{
			case 0x00:
				// Interpolation pattern (current pixel in center):
				// x B x
				// R G R
				// x B x
				green = s[0];
				red = s[1];
				nr = 1;
				if (col != 0)
				{
					red += s[-1];
					nr++;
				}
				blue = s[width];
				nb = 1;
				if (row != 0)
				{
					blue += s[-width];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			case 0x01:
				// Interpolation pattern (current pixel in center):
				// B G B
				// G R G
				// B G B
				red = *s;
				nr = 1;
				blue = s[width - 1];
				nb = 1;
				green = s[-1] + s[width];
				ng = 2;
				if (row != 0)
				{
					blue += s[-width - 1];
					nb++;
					green += s[-width];
					ng++;

					if (col != width - 1)
					{
						blue += s[-width + 1];
						nb++;
					}
				}

				if (col != width - 1)
				{
					green += s[1];
					ng++;
					blue += s[width + 1];
					nb++;
				}

				green /= ng;
				blue /= nb;
				break;
			case 0x10:
				// Interpolation pattern (current pixel in center):
				// R G R
				// G B G
				// R G R
				blue = s[0];
				red = s[-width + 1];
				nr = 1;
				green = s[-width] + s[1];
				ng = 2;
				if (col != 0)
				{
					green += s[-1];
					ng++;
				}

				if (row != height - 1)
				{
					red += s[width + 1];
					nr++;
					green += s[width];
					ng++;
					if (col != 0)
					{
						red += s[width - 1];
						nr++;
					}
				}

				red /= nr;
				green /= ng;
				break;
			case 0x11:
				// Interpolation pattern (current pixel in center):
				// x R x
				// B G B
				// x R x
				red = s[-width];
				nr = 1;
				if (row != height - 1)
				{
					red += s[width];
					nr++;
				}
				green = s[0];
				blue = s[-1];
				nb = 1;
				if (col != width - 1)
				{
					blue += s[1];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			}

			unsigned char* rgbptr = dst + (row * width + col) * 3;
			rgbptr[0] = red >> 2;
			rgbptr[1] = green >> 2;
			rgbptr[2] = blue >> 2;
    	}
    }
}

static inline void RENAME(sbggr16torgb24)(const uint8_t *src, uint8_t *dst,
		int width, int height)
{
	/* row 0: B00 G01 B02 G03...
	   row 1: G10 R11 G12 R13...
	   row 2: B20 G21 B22 G23...
	   row 3: G30 R31 G32 R33...
	     .
	     .
	     .
	   */

	uint16_t* src16 = (uint16_t*)src;
    int row, col;
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
			int quadrant = ((row&1)<<4) | (col&1);
			int red=0, green=0, blue=0;
			int nr=0, ng=0, nb=0;
			const uint16_t* s = src16 + row * width + col;

			switch (quadrant)
			{
			case 0x00:
				// Interpolation pattern (current pixel in center):
				// R G R
				// G B G
				// R G R
				blue = s[0];
				red = s[width + 1];
				nr = 1;
				green = s[1] +s[width];
				ng = 2;
				if (col != 0)
				{
					red += s[width - 1];
					nr++;
					green += s[-1];
					ng++;
				}

				if (row != 0)
				{
					green += s[-width];
					ng++;
					red += s[-width + 1];
					nr++;
					if (col != 0)
					{
						red += s[-width - 1];
						nr++;
					}
				}

				red /= nr;
				green /= ng;
				break;
			case 0x01:
				// Interpolation pattern (current pixel in center):
				// x R x
				// B G B
				// x R x
				green = s[0];
				red = s[width];
				nr = 1;
				if (row != 0)
				{
					red += s[-width];
					nr++;
				}
				blue = s[-1];
				nb = 1;
				if (col != width - 1)
				{
					blue += s[1];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			case 0x10:
				// Interpolation pattern (current pixel in center):
				// x B x
				// R G R
				// x B x
				green = s[0];
				red = s[1];
				nr = 1;
				if (col != 0)
				{
					red += s[-1];
					nr++;
				}
				blue = s[-width];
				if (row != height - 1)
				{
					blue += s[width];
					nb++;
				}
				red /= nr;
				blue /= nb;
				break;
			case 0x11:
				// Interpolation pattern (current pixel in center):
				// B G B
				// G R G
				// B G B
				red = s[0];
				green =  s[-1] + s[-width];
				ng = 2;
				blue = s[-width - 1];
				nb = 1;
				if (col != width - 1)
				{
					green += s[1];
					ng++;
					blue += s[-width + 1];
					nb++;
				}
				if (row != height - 1)
				{
					green += s[width];
					ng++;
					blue += s[width - 1];
					nb++;
					if (col != width - 1)
					{
						blue += s[width + 1];
						nb++;
					}
				}
				green /= ng;
				blue /= nb;
				break;
			}
			unsigned char* rgbptr = dst + (row * width + col) * 3;
			rgbptr[0] = red >> 8;
			rgbptr[1] = green >> 8;
			rgbptr[2] = blue >> 8;
    	}
    }
}

static inline void RENAME( sgrbg10pixeltorgb24)
(const uint8_t *src,
    int x, int y, int width, int height, uint8_t* r, uint8_t* g, uint8_t* b)
{
    /* row 0: G00 R01 G02 R03...
     row 1: B10 G11 B12 G13...
     row 2: G20 R21 G22 R23...
     row 3: B30 G31 B32 G33...
     .
     .
     .
     */

    uint16_t* src16 = (uint16_t*)src;
    int quadrant = ((y&1)<<4) | (x&1);
    int red=0, green=0, blue=0;
    int nr=0, ng=0, nb=0;
    const uint16_t* s = src16 + y * width + x;

    switch (quadrant)
    {
        case 0x00:
        green = s[0];
        red = s[1];
        nr = 1;
        if (x != 0)
        {
            red += s[-1];
            nr++;
        }
        blue = s[width];
        nb = 1;
        if (y != 0)
        {
            blue += s[-width];
            nb++;
        }
        red /= nr;
        blue /= nb;
        break;
        case 0x01:
        red = *s;
        nr = 1;
        blue = s[width - 1];
        nb = 1;
        green = s[-1] + s[width];
        ng = 2;
        if (y != 0)
        {
            blue += s[-width - 1];
            nb++;
            green += s[-width];
            ng++;

            if (x != width - 1)
            {
                blue += s[-width + 1];
                nb++;
            }
        }

        if (x != width - 1)
        {
            green += s[1];
            ng++;
            blue += s[width + 1];
            nb++;
        }

        green /= ng;
        blue /= nb;
        break;
        case 0x10:
        blue = s[0];
        red = s[-width + 1];
        nr = 1;
        green = s[-width] + s[1];
        ng = 2;
        if (x != 0)
        {
            green += s[-1];
            ng++;
        }

        if (y != height - 1)
        {
            red += s[width + 1];
            nr++;
            green += s[width];
            ng++;
            if (x != 0)
            {
                red += s[width - 1];
                nr++;
            }
        }

        red /= nr;
        green /= ng;
        break;
        case 0x11:
        red = s[-width];
        nr = 1;
        if (y != height - 1)
        {
            red += s[width];
            nr++;
        }
        green = s[0];
        blue = s[-1];
        nb = 1;
        if (x != width - 1)
        {
            blue += s[1];
            nb++;
        }
        red /= nr;
        blue /= nb;
        break;
    }

    *r = red >> 2;
    *g = green >> 2;
    *b = blue >> 2;
}

/*
  I use less accurate approximation here by simply left-shifting the input
  value and filling the low order bits with zeroes. This method improves PNG
  compression but this scheme cannot reproduce white exactly, since it does
  not generate an all-ones maximum value; the net effect is to darken the
  image slightly.

  The better method should be "left bit replication":

   4 3 2 1 0
   ---------
   1 1 0 1 1

   7 6 5 4 3  2 1 0
   ----------------
   1 1 0 1 1  1 1 0
   |=======|  |===|
       |      leftmost bits repeated to fill open bits
       |
   original bits
*/
static inline void RENAME(rgb15tobgr24)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x7C00)>>7;
    }
}

static inline void RENAME(rgb16tobgr24)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = (uint8_t *)dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0xF800)>>8;
    }
}

static inline void RENAME(rgb15to32)(const uint8_t *src, uint8_t *dst, long src_size)
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
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
#else
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x7C00)>>7;
        *d++ = 255;
#endif
    }
}

static inline void RENAME(rgb16to32)(const uint8_t *src, uint8_t *dst, long src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
#ifdef WORDS_BIGENDIAN
        *d++ = 255;
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
#else
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0xF800)>>8;
        *d++ = 255;
#endif
    }
}

static inline void RENAME(rgb32tobgr32)(const uint8_t *src, uint8_t *dst, long src_size)
{
    long idx = 15 - src_size;
    const uint8_t *s = src-idx;
    uint8_t *d = dst-idx;
    for (; idx<15; idx+=4) {
        register int v = *(const uint32_t *)&s[idx], g = v & 0xff00ff00;
        v &= 0xff00ff;
        *(uint32_t *)&d[idx] = (v>>16) + g + (v<<16);
    }
}

static inline void RENAME(rgb24tobgr24)(const uint8_t *src, uint8_t *dst, long src_size)
{
    long i;
    for (i=0; i<src_size; i+=3)
    {
        register uint8_t x;
        x          = src[i + 2];
        dst[i + 1] = src[i + 1];
        dst[i + 2] = src[i + 0];
        dst[i + 0] = x;
    }
}

static inline void RENAME(yuvPlanartoyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           long width, long height,
                                           long lumStride, long chromStride, long dstStride, long vertLumPerChroma)
{
    long y;
    const long chromWidth= width>>1;
    for (y=0; y<height; y++)
    {
        int i, *idst = (int32_t *) dst;
        const uint8_t *yc = ysrc, *uc = usrc, *vc = vsrc;
        for (i = 0; i < chromWidth; i++){
#ifdef WORDS_BIGENDIAN
            *idst++ = (yc[0] << 24)+ (uc[0] << 16) +
                (yc[1] << 8) + (vc[0] << 0);
#else
            *idst++ = yc[0] + (uc[0] << 8) +
                (yc[1] << 16) + (vc[0] << 24);
#endif
            yc += 2;
            uc++;
            vc++;
        }
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1)
        {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst  += dstStride;
    }
}

/**
 * Height should be a multiple of 2 and width should be a multiple of 16.
 * (If this is a problem for anyone then tell me, and I will fix it.)
 */
static inline void RENAME(yv12toyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      long width, long height,
                                      long lumStride, long chromStride, long dstStride)
{
    //FIXME interpolate chroma
    RENAME(yuvPlanartoyuy2)(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

static inline void RENAME(yuvPlanartouyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           long width, long height,
                                           long lumStride, long chromStride, long dstStride, long vertLumPerChroma)
{
    long y;
    const long chromWidth= width>>1;
    for (y=0; y<height; y++)
    {
        int i, *idst = (int32_t *) dst;
        const uint8_t *yc = ysrc, *uc = usrc, *vc = vsrc;
        for (i = 0; i < chromWidth; i++){
#ifdef WORDS_BIGENDIAN
            *idst++ = (uc[0] << 24)+ (yc[0] << 16) +
                (vc[0] << 8) + (yc[1] << 0);
#else
            *idst++ = uc[0] + (yc[0] << 8) +
               (vc[0] << 16) + (yc[1] << 24);
#endif
            yc += 2;
            uc++;
            vc++;
        }
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1)
        {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst += dstStride;
    }
}

/**
 * Height should be a multiple of 2 and width should be a multiple of 16
 * (If this is a problem for anyone then tell me, and I will fix it.)
 */
static inline void RENAME(yv12touyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      long width, long height,
                                      long lumStride, long chromStride, long dstStride)
{
    //FIXME interpolate chroma
    RENAME(yuvPlanartouyvy)(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

/**
 * Width should be a multiple of 16.
 */
static inline void RENAME(yuv422ptouyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         long width, long height,
                                         long lumStride, long chromStride, long dstStride)
{
    RENAME(yuvPlanartouyvy)(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

/**
 * Width should be a multiple of 16.
 */
static inline void RENAME(yuv422ptoyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         long width, long height,
                                         long lumStride, long chromStride, long dstStride)
{
    RENAME(yuvPlanartoyuy2)(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

/**
 * Height should be a multiple of 2 and width should be a multiple of 16.
 * (If this is a problem for anyone then tell me, and I will fix it.)
 */
static inline void RENAME(yuy2toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      long width, long height,
                                      long lumStride, long chromStride, long srcStride)
{
    long y;
    const long chromWidth= width>>1;
    for (y=0; y<height; y+=2)
    {
        long i;
        for (i=0; i<chromWidth; i++)
        {
            ydst[2*i+0]     = src[4*i+0];
            udst[i]     = src[4*i+1];
            ydst[2*i+1]     = src[4*i+2];
            vdst[i]     = src[4*i+3];
        }
        ydst += lumStride;
        src  += srcStride;

        for (i=0; i<chromWidth; i++)
        {
            ydst[2*i+0]     = src[4*i+0];
            ydst[2*i+1]     = src[4*i+2];
        }
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src  += srcStride;
    }
}

static inline void RENAME(yvu9toyv12)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc,
                                      uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      long width, long height, long lumStride, long chromStride)
{
	UNUSED(usrc);
	UNUSED(vsrc);
	UNUSED(udst);
	UNUSED(vdst);
	UNUSED(lumStride);
	UNUSED(chromStride);

    /* Y Plane */
    memcpy(ydst, ysrc, width*height);

    /* XXX: implement upscaling for U,V */
}

static inline void RENAME(planar2x)(const uint8_t *src, uint8_t *dst, long srcWidth, long srcHeight, long srcStride, long dstStride)
{
    long x,y;

    dst[0]= src[0];

    // first line
    for (x=0; x<srcWidth-1; x++){
        dst[2*x+1]= (3*src[x] +   src[x+1])>>2;
        dst[2*x+2]= (  src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];

        dst+= dstStride;

    for (y=1; y<srcHeight; y++){
        const long mmxSize=1;
        dst[0        ]= (3*src[0] +   src[srcStride])>>2;
        dst[dstStride]= (  src[0] + 3*src[srcStride])>>2;

        for (x=mmxSize-1; x<srcWidth-1; x++){
            dst[2*x          +1]= (3*src[x+0] +   src[x+srcStride+1])>>2;
            dst[2*x+dstStride+2]= (  src[x+0] + 3*src[x+srcStride+1])>>2;
            dst[2*x+dstStride+1]= (  src[x+1] + 3*src[x+srcStride  ])>>2;
            dst[2*x          +2]= (3*src[x+1] +   src[x+srcStride  ])>>2;
        }
        dst[srcWidth*2 -1            ]= (3*src[srcWidth-1] +   src[srcWidth-1 + srcStride])>>2;
        dst[srcWidth*2 -1 + dstStride]= (  src[srcWidth-1] + 3*src[srcWidth-1 + srcStride])>>2;

        dst+=dstStride*2;
        src+=srcStride;
    }

    // last line
#if 1
    dst[0]= src[0];

    for (x=0; x<srcWidth-1; x++){
        dst[2*x+1]= (3*src[x] +   src[x+1])>>2;
        dst[2*x+2]= (  src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];
#else
    for (x=0; x<srcWidth; x++){
        dst[2*x+0]=
        dst[2*x+1]= src[x];
    }
#endif
}

/**
 * Height should be a multiple of 2 and width should be a multiple of 16.
 * (If this is a problem for anyone then tell me, and I will fix it.)
 * Chrominance data is only taken from every second line, others are ignored.
 * FIXME: Write HQ version.
 */
static inline void RENAME(uyvytoyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      long width, long height,
                                      long lumStride, long chromStride, long srcStride)
{
    long y;
    const long chromWidth= width>>1;
    for (y=0; y<height; y+=2)
    {
        long i;
        for (i=0; i<chromWidth; i++)
        {
            udst[i]     = src[4*i+0];
            ydst[2*i+0] = src[4*i+1];
            vdst[i]     = src[4*i+2];
            ydst[2*i+1] = src[4*i+3];
        }
        ydst += lumStride;
        src  += srcStride;

        for (i=0; i<chromWidth; i++)
        {
            ydst[2*i+0] = src[4*i+1];
            ydst[2*i+1] = src[4*i+3];
        }
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src  += srcStride;
    }
}

/**
 * Height should be a multiple of 2 and width should be a multiple of 2.
 * (If this is a problem for anyone then tell me, and I will fix it.)
 * Chrominance data is only taken from every second line,
 * others are ignored in the C version.
 * FIXME: Write HQ version.
 */
static inline void RENAME(rgb24toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                       long width, long height,
                                       long lumStride, long chromStride, long srcStride)
{
    long y;
    const long chromWidth= width>>1;
    y=0;
    for (; y<height; y+=2)
    {
        long i;
        for (i=0; i<chromWidth; i++)
        {
            unsigned int b = src[6*i+0];
            unsigned int g = src[6*i+1];
            unsigned int r = src[6*i+2];

            unsigned int Y  =  ((RY*r + GY*g + BY*b)>>RGB2YUV_SHIFT) + 16;
            unsigned int V  =  ((RV*r + GV*g + BV*b)>>RGB2YUV_SHIFT) + 128;
            unsigned int U  =  ((RU*r + GU*g + BU*b)>>RGB2YUV_SHIFT) + 128;

            udst[i]     = U;
            vdst[i]     = V;
            ydst[2*i]   = Y;

            b = src[6*i+3];
            g = src[6*i+4];
            r = src[6*i+5];

            Y  =  ((RY*r + GY*g + BY*b)>>RGB2YUV_SHIFT) + 16;
            ydst[2*i+1]     = Y;
        }
        ydst += lumStride;
        src  += srcStride;

        for (i=0; i<chromWidth; i++)
        {
            unsigned int b = src[6*i+0];
            unsigned int g = src[6*i+1];
            unsigned int r = src[6*i+2];

            unsigned int Y  =  ((RY*r + GY*g + BY*b)>>RGB2YUV_SHIFT) + 16;

            ydst[2*i]     = Y;

            b = src[6*i+3];
            g = src[6*i+4];
            r = src[6*i+5];

            Y  =  ((RY*r + GY*g + BY*b)>>RGB2YUV_SHIFT) + 16;
            ydst[2*i+1]     = Y;
        }
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src  += srcStride;
    }
}

static void RENAME(interleaveBytes)(uint8_t *src1, uint8_t *src2, uint8_t *dest,
                             long width, long height, long src1Stride,
                             long src2Stride, long dstStride){
    long h;

    for (h=0; h < height; h++)
    {
        long w;

        for (w=0; w < width; w++)
        {
            dest[2*w+0] = src1[w];
            dest[2*w+1] = src2[w];
        }
        dest += dstStride;
                src1 += src1Stride;
                src2 += src2Stride;
    }
}

static inline void RENAME(vu9_to_vu12)(const uint8_t *src1, const uint8_t *src2,
                                       uint8_t *dst1, uint8_t *dst2,
                                       long width, long height,
                                       long srcStride1, long srcStride2,
                                       long dstStride1, long dstStride2)
{
    long y,x,w,h;
    w=width/2; h=height/2;
    for (y=0;y<h;y++){
    const uint8_t* s1=src1+srcStride1*(y>>1);
    uint8_t* d=dst1+dstStride1*y;
    x=0;
    for (;x<w;x++) d[2*x]=d[2*x+1]=s1[x];
    }
    for (y=0;y<h;y++){
    const uint8_t* s2=src2+srcStride2*(y>>1);
    uint8_t* d=dst2+dstStride2*y;
    x=0;
    for (;x<w;x++) d[2*x]=d[2*x+1]=s2[x];
    }
}

static inline void RENAME(yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                                        uint8_t *dst,
                                        long width, long height,
                                        long srcStride1, long srcStride2,
                                        long srcStride3, long dstStride)
{
    long y,x,w,h;
    w=width/2; h=height;
    for (y=0;y<h;y++){
    const uint8_t* yp=src1+srcStride1*y;
    const uint8_t* up=src2+srcStride2*(y>>2);
    const uint8_t* vp=src3+srcStride3*(y>>2);
    uint8_t* d=dst+dstStride*y;
    x=0;
    for (; x<w; x++)
    {
        const long x2 = x<<2;
        d[8*x+0] = yp[x2];
        d[8*x+1] = up[x];
        d[8*x+2] = yp[x2+1];
        d[8*x+3] = vp[x];
        d[8*x+4] = yp[x2+2];
        d[8*x+5] = up[x];
        d[8*x+6] = yp[x2+3];
        d[8*x+7] = vp[x];
    }
    }
}

static inline void RENAME(rgb2rgb_init)(void){
    rgb15to16       = RENAME(rgb15to16);
    rgb15tobgr24    = RENAME(rgb15tobgr24);
    rgb15to32       = RENAME(rgb15to32);
    rgb16tobgr24    = RENAME(rgb16tobgr24);
    rgb16to32       = RENAME(rgb16to32);
    rgb16to15       = RENAME(rgb16to15);
    rgb24tobgr16    = RENAME(rgb24tobgr16);
    rgb24tobgr15    = RENAME(rgb24tobgr15);
    rgb24tobgr32    = RENAME(rgb24tobgr32);
    rgb32to16       = RENAME(rgb32to16);
    rgb32to15       = RENAME(rgb32to15);
    rgb32tobgr24    = RENAME(rgb32tobgr24);
    rgb24to15       = RENAME(rgb24to15);
    rgb24to16       = RENAME(rgb24to16);
    rgb24tobgr24    = RENAME(rgb24tobgr24);
    rgb32tobgr32    = RENAME(rgb32tobgr32);
    rgb32tobgr16    = RENAME(rgb32tobgr16);
    rgb32tobgr15    = RENAME(rgb32tobgr15);
    sbggr8torgb24   = RENAME(sbggr8torgb24);
    sbggr16torgb24  = RENAME(sbggr16torgb24);
    sgrbg10torgb24  = RENAME(sgrbg10torgb24);
    sgrbg10pixeltorgb24  = RENAME(sgrbg10pixeltorgb24);
    yv12toyuy2      = RENAME(yv12toyuy2);
    yv12touyvy      = RENAME(yv12touyvy);
    yuv422ptoyuy2   = RENAME(yuv422ptoyuy2);
    yuv422ptouyvy   = RENAME(yuv422ptouyvy);
    yuy2toyv12      = RENAME(yuy2toyv12);
//    uyvytoyv12      = RENAME(uyvytoyv12);
//    yvu9toyv12      = RENAME(yvu9toyv12);
    planar2x        = RENAME(planar2x);
    rgb24toyv12     = RENAME(rgb24toyv12);
    interleaveBytes = RENAME(interleaveBytes);
    vu9_to_vu12     = RENAME(vu9_to_vu12);
    yvu9_to_yuy2    = RENAME(yvu9_to_yuy2);
}
