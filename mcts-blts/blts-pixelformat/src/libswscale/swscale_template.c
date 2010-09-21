/*
 * Copyright (C) 2001-2003 Michael Niedermayer <michaelni@gmx.at>
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

#include <blts_log.h>

static inline void RENAME(yuv2yuvX)(SwsContext *c, int16_t *lumFilter, uint16_t **lumSrc, int lumFilterSize,
                                    int16_t *chrFilter, uint16_t **chrSrc, int chrFilterSize,
                                    uint8_t *dest, uint8_t *uDest, uint8_t *vDest, long dstW, long chrDstW)
{
    UNUSED(c);
    yuv2yuvXinC(lumFilter, lumSrc, lumFilterSize,
                chrFilter, chrSrc, chrFilterSize,
                dest, uDest, vDest, dstW, chrDstW);
}

static inline void RENAME(yuv2nv12X)(SwsContext *c, int16_t *lumFilter, uint16_t **lumSrc, int lumFilterSize,
                                     int16_t *chrFilter, uint16_t **chrSrc, int chrFilterSize,
                                     uint8_t *dest, uint8_t *uDest, int dstW, int chrDstW, int dstFormat)
{
    UNUSED(c);
    yuv2nv12XinC(lumFilter, lumSrc, lumFilterSize,
                 chrFilter, chrSrc, chrFilterSize,
                 dest, uDest, dstW, chrDstW, dstFormat);
}

static inline void RENAME(yuv2yuv1)(SwsContext *c, uint16_t *lumSrc, uint16_t *chrSrc,
                                    uint8_t *dest, uint8_t *uDest, uint8_t *vDest, long dstW, long chrDstW)
{
    UNUSED(c);
    int i;
    for (i=0; i<dstW; i++)
    {
        int val= (lumSrc[i]+64)>>7;

        if (val&256){
            if (val<0) val=0;
            else       val=255;
        }

        dest[i]= val;
    }

    if (uDest)
        for (i=0; i<chrDstW; i++)
        {
            int u=(chrSrc[i       ]+64)>>7;
            int v=(chrSrc[i + VOFW]+64)>>7;

            if ((u|v)&256){
                if (u<0)        u=0;
                else if (u>255) u=255;
                if (v<0)        v=0;
                else if (v>255) v=255;
            }

            uDest[i]= u;
            vDest[i]= v;
        }
}


/**
 * vertical scale YV12 to RGB
 */
static inline void RENAME(yuv2packedX)(SwsContext *c, int16_t *lumFilter, uint16_t **lumSrc, int lumFilterSize,
                                       int16_t *chrFilter, uint16_t **chrSrc, int chrFilterSize,
                                       uint8_t *dest, long dstW, long dstY)
{
        yuv2packedXinC(c, lumFilter, lumSrc, lumFilterSize,
                       chrFilter, chrSrc, chrFilterSize,
                       dest, dstW, dstY);
}

/**
 * vertical bilinear scale YV12 to RGB
 */
static inline void RENAME(yuv2packed2)(SwsContext *c, uint16_t *buf0, uint16_t *buf1, uint16_t *uvbuf0, uint16_t *uvbuf1,
                          uint8_t *dest, int dstW, int yalpha, int uvalpha, int y)
{
    int  yalpha1=4095- yalpha;
    int uvalpha1=4095-uvalpha;
    int i;

YSCALE_YUV_2_ANYRGB_C(YSCALE_YUV_2_RGB2_C, YSCALE_YUV_2_PACKED2_C, YSCALE_YUV_2_GRAY16_2_C, YSCALE_YUV_2_MONO2_C)
}

/**
 * YV12 to RGB without scaling or interpolating
 */
static inline void RENAME(yuv2packed1)(SwsContext *c, uint16_t *buf0, uint16_t *uvbuf0, uint16_t *uvbuf1,
                          uint8_t *dest, int dstW, int uvalpha, int dstFormat, int flags, int y)
{
    UNUSED(dstFormat);

    const int yalpha1=0;
    int i;

    uint16_t *buf1= buf0; //FIXME needed for RGB1/BGR1
    const int yalpha= 4096; //FIXME ...

    if (flags&SWS_FULL_CHR_H_INT)
    {
        RENAME(yuv2packed2)(c, buf0, buf0, uvbuf0, uvbuf1, dest, dstW, 0, uvalpha, y);
        return;
    }

    if (uvalpha < 2048)
    {
        YSCALE_YUV_2_ANYRGB_C(YSCALE_YUV_2_RGB1_C, YSCALE_YUV_2_PACKED1_C, YSCALE_YUV_2_GRAY16_1_C, YSCALE_YUV_2_MONO2_C)
    }else{
        YSCALE_YUV_2_ANYRGB_C(YSCALE_YUV_2_RGB1B_C, YSCALE_YUV_2_PACKED1B_C, YSCALE_YUV_2_GRAY16_1_C, YSCALE_YUV_2_MONO2_C)
    }
}

//FIXME yuy2* can read up to 7 samples too much

static inline void RENAME(yuy2ToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= src[2*i];
}

static inline void RENAME(yuy2ToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
	UNUSED(unused);

	int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= src1[4*i + 1];
        dstV[i]= src1[4*i + 3];
    }
    assert(src1 == src2);
}

/* This is almost identical to the previous, end exists only because
 * yuy2ToY/UV)(dst, src+1, ...) would have 100% unaligned accesses. */
static inline void RENAME(uyvyToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= src[2*i+1];
}

static inline void RENAME(uyvyToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= src1[4*i + 0];
        dstV[i]= src1[4*i + 2];
    }
    assert(src1 == src2);
}

static inline void RENAME(yuv4packedToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= scaleupNbits(src[2*i+1]&0xf, 4, 8);
}

static void RENAME(yuv4packedToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= scaleupNbits(src1[2*i]>>4, 4, 8);
        dstV[i]= scaleupNbits(src1[2*i]&0x0f, 4, 8);
    }
    assert(src1 == src2);
}

static inline void RENAME(yuv555packedToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
	UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= scaleupNbits((src[2*i+1] & 0x7c) >> 2, 5, 8);
}

static void RENAME(yuv555packedToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= scaleupNbits(((src1[2*i] & 0xe0)>>5) | ((src1[2*i+1] & 0x03)<<3), 5, 8);
        dstV[i]= scaleupNbits(src1[2*i]&0x1f, 5, 8);
    }
    assert(src1 == src2);
}

static inline void RENAME(yuv565packedToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= scaleupNbits((src[2*i+1] & 0xf8) >> 3, 5, 8);
}

static void RENAME(yuv565packedToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
	UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= scaleupNbits(((src1[2*i] & 0xe0)>>5) | ((src1[2*i+1] & 0x07)<<3), 6, 8);
        dstV[i]= scaleupNbits(src1[2*i]&0x1f, 5, 8);
    }
    assert(src1 == src2);
}

static inline void RENAME(yuv32ToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
        dst[i]= src[4 * i + 1];
}

static void RENAME(yuv32ToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        dstU[i]= src1[4 * i + 2];
        dstV[i]= src1[4 * i + 3];
    }
    assert(src1 == src2);
}

#define BGR2Y(type, name, shr, shg, shb, maskr, maskg, maskb, RY, GY, BY, S)\
static inline void RENAME(name)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)\
{\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        int b= (((type*)src)[i]>>shb)&maskb;\
        int g= (((type*)src)[i]>>shg)&maskg;\
        int r= (((type*)src)[i]>>shr)&maskr;\
\
        dst[i]= (((RY)*r + (GY)*g + (BY)*b + (33<<((S)-1)))>>(S));\
    }\
}

#define BGR2YSWAP16(name, shr, shg, shb, maskr, maskg, maskb, RY, GY, BY, S)\
static inline void RENAME(name)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)\
{\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        uint16_t val = ((uint16_t*)src)[i];\
        val = (val>>8)|(val<<8);\
        int b= (val>>shb)&maskb;\
        int g= (val>>shg)&maskg;\
        int r= (val>>shr)&maskr;\
\
        dst[i]= (((RY)*r + (GY)*g + (BY)*b + (33<<((S)-1)))>>(S));\
    }\
}

BGR2Y(uint32_t, bgr32ToY,16, 0, 0, 0x00FF, 0xFF00, 0x00FF, RY<< 8, GY   , BY<< 8, RGB2YUV_SHIFT+8)
BGR2Y(uint32_t, rgb32ToY, 0, 0,16, 0x00FF, 0xFF00, 0x00FF, RY<< 8, GY   , BY<< 8, RGB2YUV_SHIFT+8)
BGR2Y(uint16_t, bgr16ToY, 0, 0, 0, 0x001F, 0x07E0, 0xF800, RY<<11, GY<<5, BY    , RGB2YUV_SHIFT+8)
BGR2Y(uint16_t, bgr15ToY, 0, 0, 0, 0x001F, 0x03E0, 0x7C00, RY<<10, GY<<5, BY    , RGB2YUV_SHIFT+7)
BGR2Y(uint16_t, rgb16ToY, 0, 0, 0, 0xF800, 0x07E0, 0x001F, RY    , GY<<5, BY<<11, RGB2YUV_SHIFT+8)
BGR2Y(uint16_t, rgb15ToY, 0, 0, 0, 0x7C00, 0x03E0, 0x001F, RY    , GY<<5, BY<<10, RGB2YUV_SHIFT+7)
BGR2YSWAP16(rgb16ToYswap, 0, 0, 0, 0xF800, 0x07E0, 0x001F, RY    , GY<<5, BY<<11, RGB2YUV_SHIFT+8)
BGR2YSWAP16(rgb15ToYswap, 0, 0, 0, 0x7C00, 0x03E0, 0x001F, RY    , GY<<5, BY<<10, RGB2YUV_SHIFT+7)

static inline void RENAME(sgrbg10toYUV)
(uint8_t* dstY, uint8_t* dstU, uint8_t* dstV, uint8_t* src, int srcY, int srcW, int srcH, uint32_t *pal)
{
    UNUSED(pal);

    int x;
    uint8_t r, g, b;
    const uint32_t S = RGB2YUV_SHIFT;

    for (x = 0; x < srcW; x++)
    {
        sgrbg10pixeltorgb24(src, x, srcY, srcW, srcH, &r, &g, &b);
        dstY[x]= (((RY)*r + (GY)*g + (BY)*b + (33<<((S)-1)))>>(S));
        dstU[x]= ((RU)*r + (GU)*g + (BU)*b + (257<<((S)-1)))>>(S);
        dstV[x]= ((RV)*r + (GV)*g + (BV)*b + (257<<((S)-1)))>>(S);
    }
}

#define BGR2UV(type, name, shr, shg, shb, maska, maskr, maskg, maskb, RU, GU, BU, RV, GV, BV, S)\
static inline void RENAME(name)(uint8_t *dstU, uint8_t *dstV, uint8_t *src, uint8_t *dummy, long width, uint32_t *unused)\
{\
    UNUSED(dummy);\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        int b= (((type*)src)[i]&maskb)>>shb;\
        int g= (((type*)src)[i]&maskg)>>shg;\
        int r= (((type*)src)[i]&maskr)>>shr;\
\
        dstU[i]= ((RU)*r + (GU)*g + (BU)*b + (257<<((S)-1)))>>(S);\
        dstV[i]= ((RV)*r + (GV)*g + (BV)*b + (257<<((S)-1)))>>(S);\
    }\
}\
static inline void RENAME(name ## _half)(uint8_t *dstU, uint8_t *dstV, uint8_t *src, uint8_t *dummy, long width, uint32_t *unused)\
{\
    UNUSED(dummy);\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        int pix0= ((type*)src)[2*i+0];\
        int pix1= ((type*)src)[2*i+1];\
        int g= (pix0&(maskg|maska))+(pix1&(maskg|maska));\
        int b= ((pix0+pix1-g)&(maskb|(2*maskb)))>>shb;\
        int r= ((pix0+pix1-g)&(maskr|(2*maskr)))>>shr;\
        g&= maskg|(2*maskg);\
\
        g>>=shg;\
\
        dstU[i]= ((RU)*r + (GU)*g + (BU)*b + (257<<(S)))>>((S)+1);\
        dstV[i]= ((RV)*r + (GV)*g + (BV)*b + (257<<(S)))>>((S)+1);\
    }\
}

#define BGR2UVSWAP16(name, shr, shg, shb, maska, maskr, maskg, maskb, RU, GU, BU, RV, GV, BV, S)\
static inline void RENAME(name)(uint8_t *dstU, uint8_t *dstV, uint8_t *src, uint8_t *dummy, long width, uint32_t *unused)\
{\
    UNUSED(dummy);\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        uint16_t val = ((uint16_t*)src)[i];\
        val = (val>>8)|(val<<8);\
        int b= (val&maskb)>>shb;\
        int g= (val&maskg)>>shg;\
        int r= (val&maskr)>>shr;\
\
        dstU[i]= ((RU)*r + (GU)*g + (BU)*b + (257<<((S)-1)))>>(S);\
        dstV[i]= ((RV)*r + (GV)*g + (BV)*b + (257<<((S)-1)))>>(S);\
    }\
}\
static inline void RENAME(name ## _half)(uint8_t *dstU, uint8_t *dstV, uint8_t *src, uint8_t *dummy, long width, uint32_t *unused)\
{\
    UNUSED(dummy);\
    UNUSED(unused);\
    int i;\
    for (i=0; i<width; i++)\
    {\
        uint16_t pix0 = ((uint16_t*)src)[2*i+0];\
        pix0 = (pix0>>8)|(pix0<<8);\
        uint16_t pix1 = ((uint16_t*)src)[2*i+1];\
        pix1 = (pix1>>8)|(pix1<<8);\
        int g= (pix0&(maskg|maska))+(pix1&(maskg|maska));\
        int b= ((pix0+pix1-g)&(maskb|(2*maskb)))>>shb;\
        int r= ((pix0+pix1-g)&(maskr|(2*maskr)))>>shr;\
        g&= maskg|(2*maskg);\
\
        g>>=shg;\
\
        dstU[i]= ((RU)*r + (GU)*g + (BU)*b + (257<<(S)))>>((S)+1);\
        dstV[i]= ((RV)*r + (GV)*g + (BV)*b + (257<<(S)))>>((S)+1);\
    }\
}

BGR2UV(uint32_t, bgr32ToUV,16, 0, 0, 0xFF000000, 0xFF0000, 0xFF00,   0x00FF, RU<< 8, GU   , BU<< 8, RV<< 8, GV   , BV<< 8, RGB2YUV_SHIFT+8)
BGR2UV(uint32_t, rgb32ToUV, 0, 0,16, 0xFF000000,   0x00FF, 0xFF00, 0xFF0000, RU<< 8, GU   , BU<< 8, RV<< 8, GV   , BV<< 8, RGB2YUV_SHIFT+8)
BGR2UV(uint16_t, bgr16ToUV, 0, 0, 0,          0,   0x001F, 0x07E0,   0xF800, RU<<11, GU<<5, BU    , RV<<11, GV<<5, BV    , RGB2YUV_SHIFT+8)
BGR2UV(uint16_t, bgr15ToUV, 0, 0, 0,          0,   0x001F, 0x03E0,   0x7C00, RU<<10, GU<<5, BU    , RV<<10, GV<<5, BV    , RGB2YUV_SHIFT+7)
BGR2UV(uint16_t, rgb16ToUV, 0, 0, 0,          0,   0xF800, 0x07E0,   0x001F, RU    , GU<<5, BU<<11, RV    , GV<<5, BV<<11, RGB2YUV_SHIFT+8)
BGR2UV(uint16_t, rgb15ToUV, 0, 0, 0,          0,   0x7C00, 0x03E0,   0x001F, RU    , GU<<5, BU<<10, RV    , GV<<5, BV<<10, RGB2YUV_SHIFT+7)
BGR2UVSWAP16(rgb16ToUVswap, 0, 0, 0,          0,   0xF800, 0x07E0,   0x001F, RU    , GU<<5, BU<<11, RV    , GV<<5, BV<<11, RGB2YUV_SHIFT+8)
BGR2UVSWAP16(rgb15ToUVswap, 0, 0, 0,          0,   0x7C00, 0x03E0,   0x001F, RU    , GU<<5, BU<<10, RV    , GV<<5, BV<<10, RGB2YUV_SHIFT+7)


static inline void RENAME(bgr24ToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        int b= src[i*3+0];
        int g= src[i*3+1];
        int r= src[i*3+2];

        dst[i]= ((RY*r + GY*g + BY*b + (33<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT);
    }
}

static inline void RENAME(bgr24ToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        int b= src1[3*i + 0];
        int g= src1[3*i + 1];
        int r= src1[3*i + 2];

        dstU[i]= (RU*r + GU*g + BU*b + (257<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT;
        dstV[i]= (RV*r + GV*g + BV*b + (257<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT;
    }
    assert(src1 == src2);
}

static inline void RENAME(bgr24ToUV_half)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        int b= src1[6*i + 0] + src1[6*i + 3];
        int g= src1[6*i + 1] + src1[6*i + 4];
        int r= src1[6*i + 2] + src1[6*i + 5];

        dstU[i]= (RU*r + GU*g + BU*b + (257<<RGB2YUV_SHIFT))>>(RGB2YUV_SHIFT+1);
        dstV[i]= (RV*r + GV*g + BV*b + (257<<RGB2YUV_SHIFT))>>(RGB2YUV_SHIFT+1);
    }
    assert(src1 == src2);
}

static inline void RENAME(rgb24ToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i;
    for (i=0; i<width; i++)
    {
        int r= src[i*3+0];
        int g= src[i*3+1];
        int b= src[i*3+2];

        dst[i]= ((RY*r + GY*g + BY*b + (33<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT);
    }
}

static inline void RENAME(rgb24ToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    assert(src1==src2);
    for (i=0; i<width; i++)
    {
        int r= src1[3*i + 0];
        int g= src1[3*i + 1];
        int b= src1[3*i + 2];

        dstU[i]= (RU*r + GU*g + BU*b + (257<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT;
        dstV[i]= (RV*r + GV*g + BV*b + (257<<(RGB2YUV_SHIFT-1)))>>RGB2YUV_SHIFT;
    }
}

static inline void RENAME(rgb24ToUV_half)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *unused)
{
    UNUSED(src2);
    UNUSED(unused);

    int i;
    assert(src1==src2);
    for (i=0; i<width; i++)
    {
        int r= src1[6*i + 0] + src1[6*i + 3];
        int g= src1[6*i + 1] + src1[6*i + 4];
        int b= src1[6*i + 2] + src1[6*i + 5];

        dstU[i]= (RU*r + GU*g + BU*b + (257<<RGB2YUV_SHIFT))>>(RGB2YUV_SHIFT+1);
        dstV[i]= (RV*r + GV*g + BV*b + (257<<RGB2YUV_SHIFT))>>(RGB2YUV_SHIFT+1);
    }
}


static inline void RENAME(palToY)(uint8_t *dst, uint8_t *src, long width, uint32_t *pal)
{
    int i;
    for (i=0; i<width; i++)
    {
        int d= src[i];

        dst[i]= pal[d] & 0xFF;
    }
}

static inline void RENAME(palToUV)(uint8_t *dstU, uint8_t *dstV, uint8_t *src1, uint8_t *src2, long width, uint32_t *pal)
{
    UNUSED(src2);

    int i;
    assert(src1 == src2);
    for (i=0; i<width; i++)
    {
        int p= pal[src1[i]];

        dstU[i]= p>>8;
        dstV[i]= p>>16;
    }
}

static inline void RENAME(monowhite2Y)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i, j;
    for (i=0; i<width/8; i++){
        int d= ~src[i];
        for(j=0; j<8; j++)
            dst[8*i+j]= ((d>>(7-j))&1)*255;
    }
}

static inline void RENAME(monoblack2Y)(uint8_t *dst, uint8_t *src, long width, uint32_t *unused)
{
    UNUSED(unused);

    int i, j;
    for (i=0; i<width/8; i++){
        int d= src[i];
        for(j=0; j<8; j++)
            dst[8*i+j]= ((d>>(7-j))&1)*255;
    }
}

// bilinear / bicubic scaling
static inline void RENAME(hScale)(uint16_t *dst, int dstW, uint8_t *src, int srcW, int xInc,
                                  int16_t *filter, int16_t *filterPos, long filterSize)
{
    UNUSED(srcW);
    UNUSED(xInc);

    int i;
    for (i=0; i<dstW; i++)
    {
        int j;
        int srcPos= filterPos[i];
        int val=0;
        //printf("filterPos: %d\n", filterPos[i]);
        for (j=0; j<filterSize; j++)
        {
            //printf("filter: %d, src: %d\n", filter[i], src[srcPos + j]);
            val += ((int)src[srcPos + j])*filter[filterSize*i + j];
        }
        //filter += hFilterSize;
        dst[i] = FFMIN(val>>7, (1<<15)-1); // the cubic equation does overflow ...
        //dst[i] = val>>7;
    }
}
      // *** horizontal scale Y line to temp buffer
static inline void RENAME(hyscale)(SwsContext *c, uint16_t *dst, long dstWidth, uint8_t *src, int srcW, int xInc,
                                   int flags, int canMMX2BeUsed, int16_t *hLumFilter,
                                   int16_t *hLumFilterPos, int hLumFilterSize, void *funnyYCode,
                                   int srcFormat, uint8_t *formatConvBuffer, int16_t *mmx2Filter,
                                   int32_t *mmx2FilterPos, uint32_t *pal)
{
    UNUSED(canMMX2BeUsed);
    UNUSED(funnyYCode);
    UNUSED(mmx2Filter);
    UNUSED(mmx2FilterPos);

    if (srcFormat==PIX_FMT_YUYV422 || srcFormat==PIX_FMT_GRAY16BE)
    {
        RENAME(yuy2ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_UYVY422 || srcFormat==PIX_FMT_GRAY16LE)
    {
        RENAME(uyvyToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB32)
    {
        RENAME(bgr32ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB32_1)
    {
        RENAME(bgr32ToY)(formatConvBuffer, src+ALT32_CORR, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_BGR24)
    {
        RENAME(bgr24ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_BGR565)
    {
        RENAME(bgr16ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_BGR555)
    {
        RENAME(bgr15ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_BGR32)
    {
        RENAME(rgb32ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_BGR32_1)
    {
        RENAME(rgb32ToY)(formatConvBuffer, src+ALT32_CORR, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB24)
    {
        RENAME(rgb24ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB565LE)
    {
        RENAME(rgb16ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB565BE)
    {
        RENAME(rgb16ToYswap)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB555LE)
    {
        RENAME(rgb15ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB555BE)
    {
        RENAME(rgb15ToYswap)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_RGB8 || srcFormat==PIX_FMT_BGR8 || srcFormat==PIX_FMT_PAL8 || srcFormat==PIX_FMT_BGR4_BYTE  || srcFormat==PIX_FMT_RGB4_BYTE)
    {
        RENAME(palToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_MONOBLACK)
    {
        RENAME(monoblack2Y)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_MONOWHITE)
    {
        RENAME(monowhite2Y)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_YUV444)
    {
        RENAME(yuv4packedToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_YUV555)
    {
        RENAME(yuv555packedToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_YUV565)
    {
        RENAME(yuv565packedToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }
    else if (srcFormat==PIX_FMT_YUV32)
    {
        RENAME(yuv32ToY)(formatConvBuffer, src, srcW, pal);
        src= formatConvBuffer;
    }

    if (!(flags&SWS_FAST_BILINEAR))
    {
        RENAME(hScale)(dst, dstWidth, src, srcW, xInc, hLumFilter, hLumFilterPos, hLumFilterSize);
    }
    else // fast bilinear upscale / crap downscale
    {
        int i;
        unsigned int xpos=0;
        for (i=0;i<dstWidth;i++)
        {
            register unsigned int xx=xpos>>16;
            register unsigned int xalpha=(xpos&0xFFFF)>>9;
            dst[i]= (src[xx]<<7) + (src[xx+1] - src[xx])*xalpha;
            xpos+=xInc;
        }
    }

    if(c->srcRange != c->dstRange && !(isRGB(c->dstFormat) || isBGR(c->dstFormat))){
        int i;
        //FIXME all pal and rgb srcFormats could do this convertion as well
        //FIXME all scalers more complex than bilinear could do half of this transform
        if(c->srcRange){
            for (i=0; i<dstWidth; i++)
                dst[i]= (dst[i]*14071 + 33561947)>>14;
        }else{
            for (i=0; i<dstWidth; i++)
                dst[i]= (FFMIN(dst[i],30189)*19077 - 39057361)>>14;
        }
    }
}

inline static void RENAME(hcscale)(SwsContext *c, uint16_t *dst, long dstWidth, uint8_t *src1, uint8_t *src2,
                                   int srcW, int xInc, int flags, int canMMX2BeUsed, int16_t *hChrFilter,
                                   int16_t *hChrFilterPos, int hChrFilterSize, void *funnyUVCode,
                                   int srcFormat, uint8_t *formatConvBuffer, int16_t *mmx2Filter,
                                   int32_t *mmx2FilterPos, uint32_t *pal)
{
    UNUSED(canMMX2BeUsed);
    UNUSED(funnyUVCode);
    UNUSED(mmx2Filter);
    UNUSED(mmx2FilterPos);

    if (srcFormat==PIX_FMT_YUYV422)
    {
        RENAME(yuy2ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_UYVY422)
    {
        RENAME(uyvyToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB32)
    {
        if(c->chrSrcHSubSample)
            RENAME(bgr32ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(bgr32ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB32_1)
    {
        if(c->chrSrcHSubSample)
            RENAME(bgr32ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1+ALT32_CORR, src2+ALT32_CORR, srcW, pal);
        else
            RENAME(bgr32ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1+ALT32_CORR, src2+ALT32_CORR, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_BGR24)
    {
        if(c->chrSrcHSubSample)
            RENAME(bgr24ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(bgr24ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_BGR565)
    {
        if(c->chrSrcHSubSample)
            RENAME(bgr16ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(bgr16ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_BGR555)
    {
        if(c->chrSrcHSubSample)
            RENAME(bgr15ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(bgr15ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_BGR32)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb32ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb32ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_BGR32_1)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb32ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1+ALT32_CORR, src2+ALT32_CORR, srcW, pal);
        else
            RENAME(rgb32ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1+ALT32_CORR, src2+ALT32_CORR, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB24)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb24ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb24ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB565LE)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb16ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb16ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB565BE)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb16ToUVswap_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb16ToUVswap)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB555LE)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb15ToUV_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb15ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_RGB555BE)
    {
        if(c->chrSrcHSubSample)
            RENAME(rgb15ToUVswap_half)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        else
            RENAME(rgb15ToUVswap)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (isGray(srcFormat) || srcFormat==PIX_FMT_MONOBLACK || srcFormat==PIX_FMT_MONOWHITE)
    {
        return;
    }
    else if (srcFormat==PIX_FMT_RGB8 || srcFormat==PIX_FMT_BGR8 || srcFormat==PIX_FMT_PAL8 || srcFormat==PIX_FMT_BGR4_BYTE  || srcFormat==PIX_FMT_RGB4_BYTE)
    {
        RENAME(palToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_YUV444)
    {
        RENAME(yuv4packedToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_YUV555)
    {
        RENAME(yuv555packedToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_YUV565)
    {
        RENAME(yuv565packedToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }
    else if (srcFormat==PIX_FMT_YUV32)
    {
        RENAME(yuv32ToUV)(formatConvBuffer, formatConvBuffer+VOFW, src1, src2, srcW, pal);
        src1= formatConvBuffer;
        src2= formatConvBuffer+VOFW;
    }

    if (!(flags&SWS_FAST_BILINEAR))
    {
        RENAME(hScale)(dst     , dstWidth, src1, srcW, xInc, hChrFilter, hChrFilterPos, hChrFilterSize);
        RENAME(hScale)(dst+VOFW, dstWidth, src2, srcW, xInc, hChrFilter, hChrFilterPos, hChrFilterSize);
    }
    else // fast bilinear upscale / crap downscale
    {
        int i;
        unsigned int xpos=0;
        for (i=0;i<dstWidth;i++)
        {
            register unsigned int xx=xpos>>16;
            register unsigned int xalpha=(xpos&0xFFFF)>>9;
            dst[i]=(src1[xx]*(xalpha^127)+src1[xx+1]*xalpha);
            dst[i+VOFW]=(src2[xx]*(xalpha^127)+src2[xx+1]*xalpha);
            /* slower
            dst[i]= (src1[xx]<<7) + (src1[xx+1] - src1[xx])*xalpha;
            dst[i+VOFW]=(src2[xx]<<7) + (src2[xx+1] - src2[xx])*xalpha;
            */
            xpos+=xInc;
        }
    }
    if(c->srcRange != c->dstRange && !(isRGB(c->dstFormat) || isBGR(c->dstFormat))){
        int i;
        //FIXME all pal and rgb srcFormats could do this convertion as well
        //FIXME all scalers more complex than bilinear could do half of this transform
        if(c->srcRange){
            for (i=0; i<dstWidth; i++){
                dst[i     ]= (dst[i     ]*1799 + 4081085)>>11; //1469
                dst[i+VOFW]= (dst[i+VOFW]*1799 + 4081085)>>11; //1469
            }
        }else{
            for (i=0; i<dstWidth; i++){
                dst[i     ]= (FFMIN(dst[i     ],30775)*4663 - 9289992)>>12; //-264
                dst[i+VOFW]= (FFMIN(dst[i+VOFW],30775)*4663 - 9289992)>>12; //-264
            }
        }
    }
}

static void bayertoYUV(SwsContext *c, uint16_t *dstY, uint16_t *dstUV,
	long dstWidth, uint8_t *src[], int srcrow, int srcW, int srcH, int xInc,
    int flags, int16_t *hLumFilter, int16_t *hLumFilterPos, int hLumFilterSize,
    int16_t *hChrFilter, int16_t *hChrFilterPos, int hChrFilterSize,
    int srcFormat, uint8_t *formatConvBuffer, uint32_t *pal)
{
    uint8_t* srcY = NULL;
    uint8_t* srcU = NULL;
    uint8_t* srcV = NULL;

    if (srcFormat == PIX_FMT_SGRBG10)
    {
        RENAME(sgrbg10toYUV)(formatConvBuffer, formatConvBuffer + VOFW,
            formatConvBuffer + 2 * VOFW, src[0], srcrow, srcW, srcH, pal);
        srcY = formatConvBuffer;
        srcU = formatConvBuffer + VOFW;
        srcV = formatConvBuffer + 2 * VOFW;
    }

    if (!(flags & SWS_FAST_BILINEAR))
    {
        RENAME(hScale)(dstY, dstWidth, srcY, srcW, xInc, hLumFilter,
            hLumFilterPos, hLumFilterSize);
    }
    else // fast bilinear upscale / crap downscale
    {
        int i;
        unsigned int xpos = 0;
        for (i = 0; i < dstWidth; i++)
        {
            register unsigned int xx = xpos >> 16;
            register unsigned int xalpha = (xpos & 0xFFFF) >> 9;
            dstY[i] = (srcY[xx] << 7) + (srcY[xx + 1] - srcY[xx]) * xalpha;
            xpos += xInc;
        }
    }

    if (c->srcRange != c->dstRange && !(isRGB(c->dstFormat) || isBGR(
        c->dstFormat)))
    {
        int i;
        //FIXME all pal and rgb srcFormats could do this convertion as well
        //FIXME all scalers more complex than bilinear could do half of this transform
        if (c->srcRange)
        {
            for (i = 0; i < dstWidth; i++)
                dstY[i] = (dstY[i] * 14071 + 33561947) >> 14;
        }
        else
        {
            for (i = 0; i < dstWidth; i++)
                dstY[i] = (FFMIN(dstY[i], 30189) * 19077 - 39057361) >> 14;
        }
    }

    if (!(flags & SWS_FAST_BILINEAR))
    {
        RENAME(hScale)(dstUV, dstWidth, srcU, srcW, xInc, hChrFilter,
            hChrFilterPos, hChrFilterSize);
        RENAME(hScale)(dstUV + VOFW, dstWidth, srcV, srcW, xInc, hChrFilter,
            hChrFilterPos, hChrFilterSize);
    }
    else // fast bilinear upscale / crap downscale
    {
        int i;
        unsigned int xpos = 0;
        for (i = 0; i < dstWidth; i++)
        {
            register unsigned int xx = xpos >> 16;
            register unsigned int xalpha = (xpos & 0xFFFF) >> 9;
            dstUV[i] = (srcU[xx] * (xalpha ^ 127) + srcU[xx + 1] * xalpha);
            dstUV[i + VOFW] = (srcV[xx] * (xalpha ^ 127) + srcV[xx + 1]
                * xalpha);
            /* slower
             dst[i]= (src1[xx]<<7) + (src1[xx+1] - src1[xx])*xalpha;
             dst[i+VOFW]=(src2[xx]<<7) + (src2[xx+1] - src2[xx])*xalpha;
             */
            xpos += xInc;
        }
    }
    if (c->srcRange != c->dstRange && !(isRGB(c->dstFormat) || isBGR(
        c->dstFormat)))
    {
        int i;
        //FIXME all pal and rgb srcFormats could do this convertion as well
        //FIXME all scalers more complex than bilinear could do half of this transform
        if (c->srcRange)
        {
            for (i = 0; i < dstWidth; i++)
            {
                dstUV[i] = (dstUV[i] * 1799 + 4081085) >> 11; //1469
                dstUV[i + VOFW] = (dstUV[i + VOFW] * 1799 + 4081085) >> 11; //1469
            }
        }
        else
        {
            for (i = 0; i < dstWidth; i++)
            {
                dstUV[i] = (FFMIN(dstUV[i], 30775) * 4663 - 9289992) >> 12; //-264
                dstUV[i + VOFW] = (FFMIN(dstUV[i + VOFW], 30775) * 4663
                    - 9289992) >> 12; //-264
            }
        }
    }
}

static int RENAME(swScale)(SwsContext *c, uint8_t* src[], int srcStride[], int srcSliceY,
                           int srcSliceH, uint8_t* dst[], int dstStride[]){

    /* load a few things into local vars to make the code more readable? and faster */
    const int srcW= c->srcW;
    const int dstW= c->dstW;
    const int dstH= c->dstH;
    const int chrDstW= c->chrDstW;
    const int chrSrcW= c->chrSrcW;
    const int lumXInc= c->lumXInc;
    const int chrXInc= c->chrXInc;
    const int dstFormat= c->dstFormat;
    const int srcFormat= c->srcFormat;
    const int flags= c->flags;
    const int canMMX2BeUsed= c->canMMX2BeUsed;
    int16_t *vLumFilterPos= c->vLumFilterPos;
    int16_t *vChrFilterPos= c->vChrFilterPos;
    int16_t *hLumFilterPos= c->hLumFilterPos;
    int16_t *hChrFilterPos= c->hChrFilterPos;
    int16_t *vLumFilter= c->vLumFilter;
    int16_t *vChrFilter= c->vChrFilter;
    int16_t *hLumFilter= c->hLumFilter;
    int16_t *hChrFilter= c->hChrFilter;
    int32_t *lumMmxFilter= c->lumMmxFilter;
    int32_t *chrMmxFilter= c->chrMmxFilter;
    const int vLumFilterSize= c->vLumFilterSize;
    const int vChrFilterSize= c->vChrFilterSize;
    const int hLumFilterSize= c->hLumFilterSize;
    const int hChrFilterSize= c->hChrFilterSize;
    uint16_t **lumPixBuf= c->lumPixBuf;
    uint16_t **chrPixBuf= c->chrPixBuf;
    const int vLumBufSize= c->vLumBufSize;
    const int vChrBufSize= c->vChrBufSize;
    uint8_t *funnyYCode= c->funnyYCode;
    uint8_t *funnyUVCode= c->funnyUVCode;
    uint8_t *formatConvBuffer= c->formatConvBuffer;
    const int chrSrcSliceY= srcSliceY >> c->chrSrcVSubSample;
    const int chrSrcSliceH= -((-srcSliceH) >> c->chrSrcVSubSample);
    int lastDstY;
    uint32_t *pal=c->pal_yuv;

    /* vars which will change and which we need to store back in the context */
    int dstY= c->dstY;
    int lumBufIndex= c->lumBufIndex;
    int chrBufIndex= c->chrBufIndex;
    int lastInLumBuf= c->lastInLumBuf;
    int lastInChrBuf= c->lastInChrBuf;

    if (isPacked(c->srcFormat)){
        src[0]=
        src[1]=
        src[2]= src[0];
        srcStride[0]=
        srcStride[1]=
        srcStride[2]= srcStride[0];
    }
    srcStride[1]<<= c->vChrDrop;
    srcStride[2]<<= c->vChrDrop;

    //printf("swscale %X %X %X -> %X %X %X\n", (int)src[0], (int)src[1], (int)src[2],
    //       (int)dst[0], (int)dst[1], (int)dst[2]);

#if 0 //self test FIXME move to a vfilter or something
    {
    static volatile int i=0;
    i++;
    if (srcFormat==PIX_FMT_YUV420P && i==1 && srcSliceH>= c->srcH)
        selfTest(src, srcStride, c->srcW, c->srcH);
    i--;
    }
#endif

    //printf("sws Strides:%d %d %d -> %d %d %d\n", srcStride[0],srcStride[1],srcStride[2],
    //dstStride[0],dstStride[1],dstStride[2]);

    if (dstStride[0]%8 !=0 || dstStride[1]%8 !=0 || dstStride[2]%8 !=0)
    {
        static int warnedAlready=0; //FIXME move this into the context perhaps
        if (flags & SWS_PRINT_INFO && !warnedAlready)
        {
            BLTS_WARNING("Warning: dstStride is not aligned!\n"
                   "         ->cannot do aligned memory accesses anymore\n");
            warnedAlready=1;
        }
    }

    /* Note the user might start scaling the picture in the middle so this
       will not get executed. This is not really intended but works
       currently, so people might do it. */
    if (srcSliceY ==0){
        lumBufIndex=0;
        chrBufIndex=0;
        dstY=0;
        lastInLumBuf= -1;
        lastInChrBuf= -1;
    }

    lastDstY= dstY;

    for (;dstY < dstH; dstY++){
        unsigned char *dest =dst[0]+dstStride[0]*dstY;
        const int chrDstY= dstY>>c->chrDstVSubSample;
        unsigned char *uDest=dst[1]+dstStride[1]*chrDstY;
        unsigned char *vDest=dst[2]+dstStride[2]*chrDstY;

        const int firstLumSrcY= vLumFilterPos[dstY]; //First line needed as input
        const int firstChrSrcY= vChrFilterPos[chrDstY]; //First line needed as input
        const int lastLumSrcY= firstLumSrcY + vLumFilterSize -1; // Last line needed as input
        const int lastChrSrcY= firstChrSrcY + vChrFilterSize -1; // Last line needed as input

        //printf("dstY:%d dstH:%d firstLumSrcY:%d lastInLumBuf:%d vLumBufSize: %d vChrBufSize: %d slice: %d %d vLumFilterSize: %d firstChrSrcY: %d vChrFilterSize: %d c->chrSrcVSubSample: %d\n",
        // dstY, dstH, firstLumSrcY, lastInLumBuf, vLumBufSize, vChrBufSize, srcSliceY, srcSliceH, vLumFilterSize, firstChrSrcY, vChrFilterSize,  c->chrSrcVSubSample);
        //handle holes (FAST_BILINEAR & weird filters)
        if (firstLumSrcY > lastInLumBuf) lastInLumBuf= firstLumSrcY-1;
        if (firstChrSrcY > lastInChrBuf) lastInChrBuf= firstChrSrcY-1;
        //printf("%d %d %d\n", firstChrSrcY, lastInChrBuf, vChrBufSize);
        assert(firstLumSrcY >= lastInLumBuf - vLumBufSize + 1);
        assert(firstChrSrcY >= lastInChrBuf - vChrBufSize + 1);

        // Do we have enough lines in this slice to output the dstY line
        if (lastLumSrcY < srcSliceY + srcSliceH && lastChrSrcY < -((-srcSliceY - srcSliceH)>>c->chrSrcVSubSample))
        {
            if (srcFormat==PIX_FMT_SGRBG10)
            {
                while(lastInLumBuf < lastLumSrcY)
                {
					bayertoYUV(c, lumPixBuf[ lumBufIndex ], chrPixBuf[ chrBufIndex ], dstW, src, lastInLumBuf + 1 - srcSliceY,
                        srcW, c->srcH, lumXInc,
                        flags, hLumFilter, hLumFilterPos, hLumFilterSize,
                        hChrFilter, hChrFilterPos, hChrFilterSize,
                        c->srcFormat, c->formatConvBuffer2,
                        pal);
                    lastInLumBuf++;
                    lastInChrBuf++;
                }
            }
            else
            {
                //Do horizontal scaling
                while(lastInLumBuf < lastLumSrcY)
                {
                    uint8_t *s= src[0]+(lastInLumBuf + 1 - srcSliceY)*srcStride[0];
                    lumBufIndex++;
                    //printf("%d %d %d %d\n", lumBufIndex, vLumBufSize, lastInLumBuf,  lastLumSrcY);
                    assert(lumBufIndex < 2*vLumBufSize);
                    assert(lastInLumBuf + 1 - srcSliceY < srcSliceH);
                    assert(lastInLumBuf + 1 - srcSliceY >= 0);
                    //printf("%d %d\n", lumBufIndex, vLumBufSize);
                    RENAME(hyscale)(c, lumPixBuf[ lumBufIndex ], dstW, s, srcW, lumXInc,
                        flags, canMMX2BeUsed, hLumFilter, hLumFilterPos, hLumFilterSize,
                        funnyYCode, c->srcFormat, formatConvBuffer,
                        c->lumMmx2Filter, c->lumMmx2FilterPos, pal);
                    lastInLumBuf++;
                }
                while(lastInChrBuf < lastChrSrcY)
                {
                    uint8_t *src1= src[1]+(lastInChrBuf + 1 - chrSrcSliceY)*srcStride[1];
                    uint8_t *src2= src[2]+(lastInChrBuf + 1 - chrSrcSliceY)*srcStride[2];
                    chrBufIndex++;
                    assert(chrBufIndex < 2*vChrBufSize);
                    assert(lastInChrBuf + 1 - chrSrcSliceY < (chrSrcSliceH));
                    assert(lastInChrBuf + 1 - chrSrcSliceY >= 0);
                    //FIXME replace parameters through context struct (some at least)

                    if (!(isGray(srcFormat) || isGray(dstFormat)))
                        RENAME(hcscale)(c, chrPixBuf[ chrBufIndex ], chrDstW, src1, src2, chrSrcW, chrXInc,
                            flags, canMMX2BeUsed, hChrFilter, hChrFilterPos, hChrFilterSize,
                            funnyUVCode, c->srcFormat, formatConvBuffer,
                            c->chrMmx2Filter, c->chrMmx2FilterPos, pal);
                    lastInChrBuf++;
                }
            }
            //wrap buf index around to stay inside the ring buffer
            if (lumBufIndex >= vLumBufSize) lumBufIndex-= vLumBufSize;
            if (chrBufIndex >= vChrBufSize) chrBufIndex-= vChrBufSize;
        }
        else // not enough lines left in this slice -> load the rest in the buffer
        {
            /* printf("%d %d Last:%d %d LastInBuf:%d %d Index:%d %d Y:%d FSize: %d %d BSize: %d %d\n",
            firstChrSrcY,firstLumSrcY,lastChrSrcY,lastLumSrcY,
            lastInChrBuf,lastInLumBuf,chrBufIndex,lumBufIndex,dstY,vChrFilterSize,vLumFilterSize,
            vChrBufSize, vLumBufSize);*/

            //Do horizontal scaling
            if (srcFormat==PIX_FMT_SGRBG10)
            {
                while(lastInLumBuf+1 < srcSliceY + srcSliceH)
                {
                    bayertoYUV(c, lumPixBuf[ lumBufIndex ], chrPixBuf[ chrBufIndex ], dstW, src, lastInLumBuf + 1 - srcSliceY,
                        srcW, c->srcH, lumXInc,
                        flags, hLumFilter, hLumFilterPos, hLumFilterSize,
                        hChrFilter, hChrFilterPos, hChrFilterSize,
                        c->srcFormat, c->formatConvBuffer2,
                        pal);
                    lastInLumBuf++;
                    lastInChrBuf++;
                }
            }
            else
            {
                while(lastInLumBuf+1 < srcSliceY + srcSliceH)
                {
                    uint8_t *s= src[0]+(lastInLumBuf + 1 - srcSliceY)*srcStride[0];
                    lumBufIndex++;
                    assert(lumBufIndex < 2*vLumBufSize);
                    assert(lastInLumBuf + 1 - srcSliceY < srcSliceH);
                    assert(lastInLumBuf + 1 - srcSliceY >= 0);
                    RENAME(hyscale)(c, lumPixBuf[ lumBufIndex ], dstW, s, srcW, lumXInc,
                                    flags, canMMX2BeUsed, hLumFilter, hLumFilterPos, hLumFilterSize,
                                    funnyYCode, c->srcFormat, formatConvBuffer,
                                    c->lumMmx2Filter, c->lumMmx2FilterPos, pal);
                    lastInLumBuf++;
                }
                while(lastInChrBuf+1 < (chrSrcSliceY + chrSrcSliceH))
                {
                    uint8_t *src1= src[1]+(lastInChrBuf + 1 - chrSrcSliceY)*srcStride[1];
                    uint8_t *src2= src[2]+(lastInChrBuf + 1 - chrSrcSliceY)*srcStride[2];
                    chrBufIndex++;
                    assert(chrBufIndex < 2*vChrBufSize);
                    assert(lastInChrBuf + 1 - chrSrcSliceY < chrSrcSliceH);
                    assert(lastInChrBuf + 1 - chrSrcSliceY >= 0);

                    if (!(isGray(srcFormat) || isGray(dstFormat)))
                        RENAME(hcscale)(c, chrPixBuf[ chrBufIndex ], chrDstW, src1, src2, chrSrcW, chrXInc,
                                flags, canMMX2BeUsed, hChrFilter, hChrFilterPos, hChrFilterSize,
                                funnyUVCode, c->srcFormat, formatConvBuffer,
                                c->chrMmx2Filter, c->chrMmx2FilterPos, pal);
                    lastInChrBuf++;
                }
                //wrap buf index around to stay inside the ring buffer
                if (lumBufIndex >= vLumBufSize) lumBufIndex-= vLumBufSize;
                if (chrBufIndex >= vChrBufSize) chrBufIndex-= vChrBufSize;
                break; //we can't output a dstY line so let's try with the next slice
            }
        }

        if (dstY < dstH-2)
        {
            uint16_t **lumSrcPtr= lumPixBuf + lumBufIndex + firstLumSrcY - lastInLumBuf + vLumBufSize;
            uint16_t **chrSrcPtr= chrPixBuf + chrBufIndex + firstChrSrcY - lastInChrBuf + vChrBufSize;
            if (dstFormat == PIX_FMT_NV12 || dstFormat == PIX_FMT_NV21){
                const int chrSkipMask= (1<<c->chrDstVSubSample)-1;
                if (dstY&chrSkipMask) uDest= NULL; //FIXME split functions in lumi / chromi
                RENAME(yuv2nv12X)(c,
                    vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, dstW, chrDstW, dstFormat);
            }
            else if (isPlanarYUV(dstFormat) || dstFormat==PIX_FMT_GRAY8) //YV12 like
            {
                const int chrSkipMask= (1<<c->chrDstVSubSample)-1;
                if ((dstY&chrSkipMask) || isGray(dstFormat)) uDest=vDest= NULL; //FIXME split functions in lumi / chromi
                if (vLumFilterSize == 1 && vChrFilterSize == 1) // unscaled YV12
                {
                    uint16_t *lumBuf = lumPixBuf[0];
                    uint16_t *chrBuf= chrPixBuf[0];
                    RENAME(yuv2yuv1)(c, lumBuf, chrBuf, dest, uDest, vDest, dstW, chrDstW);
                }
                else //General YV12
                {
                    RENAME(yuv2yuvX)(c,
                        vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                        vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                        dest, uDest, vDest, dstW, chrDstW);
                }
            }
            else if (dstFormat == PIX_FMT_YUV444)
            {
                yuv2yuv4packedinC(vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, vDest, dstW, chrDstW);
            }
            else if (dstFormat == PIX_FMT_YUV555)
            {
                yuv2yuv555packedinC(vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, vDest, dstW, chrDstW);
            }
            else if (dstFormat == PIX_FMT_YUV565)
            {
                yuv2yuv565packedinC(vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, vDest, dstW, chrDstW);
            }
            else if (dstFormat == PIX_FMT_YUV32)
            {
                yuv2yuv32inC(vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, vDest, dstW, chrDstW);
            }
            else
            {
                assert(lumSrcPtr + vLumFilterSize - 1 < lumPixBuf + vLumBufSize*2);
                assert(chrSrcPtr + vChrFilterSize - 1 < chrPixBuf + vChrBufSize*2);
                if (vLumFilterSize == 1 && vChrFilterSize == 2) //unscaled RGB
                {
                    int chrAlpha= vChrFilter[2*dstY+1];
                    if(flags & SWS_FULL_CHR_H_INT){
                        yuv2rgbXinC_full(c, //FIXME write a packed1_full function
                            vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                            vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                            dest, dstW, dstY);
                    }else{
                        RENAME(yuv2packed1)(c, *lumSrcPtr, *chrSrcPtr, *(chrSrcPtr+1),
                            dest, dstW, chrAlpha, dstFormat, flags, dstY);
                    }
                }
                else if (vLumFilterSize == 2 && vChrFilterSize == 2) //bilinear upscale RGB
                {
                    int lumAlpha= vLumFilter[2*dstY+1];
                    int chrAlpha= vChrFilter[2*dstY+1];
                    lumMmxFilter[2]=
                    lumMmxFilter[3]= vLumFilter[2*dstY   ]*0x10001;
                    chrMmxFilter[2]=
                    chrMmxFilter[3]= vChrFilter[2*chrDstY]*0x10001;
                    if(flags & SWS_FULL_CHR_H_INT){
                        yuv2rgbXinC_full(c, //FIXME write a packed2_full function
                            vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                            vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                            dest, dstW, dstY);
                    }else{
                        RENAME(yuv2packed2)(c, *lumSrcPtr, *(lumSrcPtr+1), *chrSrcPtr, *(chrSrcPtr+1),
                            dest, dstW, lumAlpha, chrAlpha, dstY);
                    }
                }
                else //general RGB
                {
                    if(flags & SWS_FULL_CHR_H_INT){
                        yuv2rgbXinC_full(c,
                            vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                            vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                            dest, dstW, dstY);
                    }else{
                        RENAME(yuv2packedX)(c,
                            vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                            vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                            dest, dstW, dstY);
                    }
                }
            }
        }
        else // hmm looks like we can't use MMX here without overwriting this array's tail
        {
            uint16_t **lumSrcPtr= lumPixBuf + lumBufIndex + firstLumSrcY - lastInLumBuf + vLumBufSize;
            uint16_t **chrSrcPtr= chrPixBuf + chrBufIndex + firstChrSrcY - lastInChrBuf + vChrBufSize;
            if (dstFormat == PIX_FMT_NV12 || dstFormat == PIX_FMT_NV21){
                const int chrSkipMask= (1<<c->chrDstVSubSample)-1;
                if (dstY&chrSkipMask) uDest= NULL; //FIXME split functions in lumi / chromi
                yuv2nv12XinC(
                    vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, dstW, chrDstW, dstFormat);
            }
            else if (isPlanarYUV(dstFormat) || dstFormat==PIX_FMT_GRAY8) //YV12
            {
                const int chrSkipMask= (1<<c->chrDstVSubSample)-1;
                if ((dstY&chrSkipMask) || isGray(dstFormat)) uDest=vDest= NULL; //FIXME split functions in lumi / chromi
                yuv2yuvXinC(
                    vLumFilter+dstY*vLumFilterSize   , lumSrcPtr, vLumFilterSize,
                    vChrFilter+chrDstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                    dest, uDest, vDest, dstW, chrDstW);
            }
            else
            {
                assert(lumSrcPtr + vLumFilterSize - 1 < lumPixBuf + vLumBufSize*2);
                assert(chrSrcPtr + vChrFilterSize - 1 < chrPixBuf + vChrBufSize*2);
                if(flags & SWS_FULL_CHR_H_INT){
                    yuv2rgbXinC_full(c,
                        vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                        vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                        dest, dstW, dstY);
                }else{
                    yuv2packedXinC(c,
                        vLumFilter+dstY*vLumFilterSize, lumSrcPtr, vLumFilterSize,
                        vChrFilter+dstY*vChrFilterSize, chrSrcPtr, vChrFilterSize,
                        dest, dstW, dstY);
                }
            }
        }
    }

    /* store changed local vars back in the context */
    c->dstY= dstY;
    c->lumBufIndex= lumBufIndex;
    c->chrBufIndex= chrBufIndex;
    c->lastInLumBuf= lastInLumBuf;
    c->lastInChrBuf= lastInChrBuf;

    return dstY - lastDstY;
}
