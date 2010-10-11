/* blts-pixelformat.c -- Pixel format conversion

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

#include "blts-pixelformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/avcodec.h"

#include <blts_log.h>

#include <errno.h>
#include <linux/videodev2.h>

struct pixelformat {
	__u32 v4l2format;
	int pixelformat;
};

static struct pixelformat pixelformats[] = {
	{ V4L2_PIX_FMT_RGB332, PIX_FMT_RGB8 },
	{ V4L2_PIX_FMT_RGB444, PIX_FMT_RGB4 },
	{ V4L2_PIX_FMT_RGB555, PIX_FMT_RGB555LE },
	{ V4L2_PIX_FMT_RGB565, PIX_FMT_RGB565LE },
	{ V4L2_PIX_FMT_RGB24, PIX_FMT_RGB24 },
	{ V4L2_PIX_FMT_BGR24, PIX_FMT_BGR24 },
	{ V4L2_PIX_FMT_BGR32, PIX_FMT_BGR32 },
	{ V4L2_PIX_FMT_RGB32, PIX_FMT_RGB32 },
	{ V4L2_PIX_FMT_GREY, PIX_FMT_GRAY8 },
	{ V4L2_PIX_FMT_Y16, PIX_FMT_GRAY16 },
	{ V4L2_PIX_FMT_YUYV, PIX_FMT_YUYV422 },
	{ V4L2_PIX_FMT_UYVY, PIX_FMT_UYVY422 },
	{ V4L2_PIX_FMT_Y41P, PIX_FMT_UYYVYY411 },
	{ V4L2_PIX_FMT_YUV422P, PIX_FMT_YUV422P },
	{ V4L2_PIX_FMT_YUV411P, PIX_FMT_YUV411P },
	{ V4L2_PIX_FMT_NV12, PIX_FMT_NV12 },
	{ V4L2_PIX_FMT_YUV420, PIX_FMT_YUV420P },
	{ V4L2_PIX_FMT_YUV410, PIX_FMT_YUV410P },

	// Custom conversions
	{ V4L2_PIX_FMT_YVU420, PIX_FMT_YVU420P },	// Like YUV420 but swapped UV -> VU
	{ V4L2_PIX_FMT_YVU410, PIX_FMT_YVU410P },	// Like YUV410 but swapped UV -> VU
	{ V4L2_PIX_FMT_SBGGR8, PIX_FMT_SBGGR8 },	// 8-bit Bayer
	{ V4L2_PIX_FMT_SBGGR16, PIX_FMT_SBGGR16 },	// 16-bit Bayer
	{ V4L2_PIX_FMT_SGRBG10, PIX_FMT_SGRBG10 },	// 10-bit Bayer packed to 16 bits
	{ V4L2_PIX_FMT_YUV444, PIX_FMT_YUV444 },	// Packed 16-bit YUV, 4 bits/component, 4-bit alpha
	{ V4L2_PIX_FMT_YUV555, PIX_FMT_YUV555 },	// Packed 16-bit YUV, 5 bits/component, 1-bit alpha
	{ V4L2_PIX_FMT_YUV565, PIX_FMT_YUV565 },	// Packed 16-bit YUV, Y5 U6 V5
	{ V4L2_PIX_FMT_YUV32, PIX_FMT_YUV32 },		// Packed 32-bit YUV, 8 bits/component, 8-bit alpha
	{ V4L2_PIX_FMT_RGB555X, PIX_FMT_RGB555BE }, // Big-endian A1R5G5B5
	{ V4L2_PIX_FMT_RGB565X, PIX_FMT_RGB565BE }, // Big-endian R5G6B5
};

static int v4l2fmt2pixfmt(__u32 v4l2fmt)
{
	const int numformats = sizeof(pixelformats) / sizeof(pixelformats[0]);
	int i;

	for (i = 0; i < numformats; i++)
	{
		if (pixelformats[i].v4l2format == v4l2fmt)
			return pixelformats[i].pixelformat;
	}

	return PIX_FMT_NONE;
}

static int setup_planes(int pixfmt, const unsigned char* planeptrs[3], int* strides, const unsigned char* buf, int width, int height)
{
	int res = 0;

	switch (pixfmt)
	{
	case PIX_FMT_RGB24:
	case PIX_FMT_BGR24:
		// Packed 24-bit
		planeptrs[0] = buf;
		planeptrs[1] = planeptrs[2] = NULL;
		strides[0] = 3 * width;
		strides[1] = 0;
		strides[2] = 0;
		break;
	case PIX_FMT_YUV422P:
		// Planar 16-bit
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height;
		planeptrs[2] = planeptrs[1] + ((width * height) >> 1);
		strides[0] = width;
		strides[1] = strides[2] = (width >> 1);
		break;
	case PIX_FMT_YUYV422:
	case PIX_FMT_UYVY422:
		// Packed 16-bit
		planeptrs[0] = buf;
		planeptrs[1] = NULL;
		planeptrs[2] = NULL;
		strides[0] = 2 * width;
		strides[1] = strides[2] = 0;
		break;
	case PIX_FMT_RGB8:
	case PIX_FMT_GRAY8:
	case PIX_FMT_SBGGR8:
		// Packed 8-bit or single 8-bit plane
		planeptrs[0] = buf;
		planeptrs[1] = planeptrs[2] = NULL;
		strides[0] = width;
		strides[1] = strides[2] = 0;
		break;
	case PIX_FMT_RGB4:
	case PIX_FMT_RGB555LE:
	case PIX_FMT_RGB555BE:
	case PIX_FMT_RGB565LE:
	case PIX_FMT_RGB565BE:
	case PIX_FMT_GRAY16LE:
	case PIX_FMT_YUV444:
	case PIX_FMT_YUV555:
	case PIX_FMT_YUV565:
	case PIX_FMT_SBGGR16:
	case PIX_FMT_SGRBG10:
		// Packed 16-bit or single 16-bit plane
		planeptrs[0] = buf;
		planeptrs[1] = planeptrs[2] = NULL;
		strides[0] = 2 * width;
		strides[1] = strides[2] = 0;
		break;
	case PIX_FMT_BGR32:
	case PIX_FMT_RGB32:
	case PIX_FMT_YUV32:
		// Packed 32-bit
		planeptrs[0] = buf;
		planeptrs[1] = planeptrs[2] = NULL;
		strides[0] = 4 * width;
		strides[1] = strides[2] = 0;
		break;
	case PIX_FMT_YUV420P:
		// Planar 12-bit (1/4 resolution U and V)
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height;
		planeptrs[2] = planeptrs[1] + ((width * height) >> 2);
		strides[0] = width;
		strides[1] = strides[2] = (width >> 1);
		break;
	case PIX_FMT_YUV410P:
	case PIX_FMT_YUV411P:
		// Planar 9-bit, 1/16 resolution U and V
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height;
		planeptrs[2] = planeptrs[1] + ((width * height) >> 4);
		strides[0] = width;
		strides[1] = strides[2] = (width >> 2);
		break;
	case PIX_FMT_NV12:
		// Planar 9-bit, 1/16 resolution U and V in a single plane
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height;
		planeptrs[2] = 0;
		strides[0] = width;
		strides[1] = (width >> 4);
		strides[2] = 0;
		break;
	case PIX_FMT_UYYVYY411:
		// Packed 12-bit
		planeptrs[0] = buf;
		planeptrs[1] = planeptrs[2] = NULL;
		strides[0] = width + (width >> 1);
		strides[1] = strides[2] = 0;
		break;
	case PIX_FMT_YVU420P:
		// Planar 12-bit, swapped U and V
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height + ((width * height) >> 2);
		planeptrs[2] = buf + width * height;
		strides[0] = width;
		strides[1] = strides[2] = (width >> 1);
		break;
	case PIX_FMT_YVU410P:
		// Planar 9-bit, swapped U and V
		planeptrs[0] = buf;
		planeptrs[1] = buf + width * height + ((width * height) >> 4);
		planeptrs[2] = buf + width * height;
		strides[0] = width;
		strides[1] = strides[2] = (width >> 2);
		break;

	default:
		res = -1;
		break;
	}

	return res;
}

int convert_buffer(int width, int height, const unsigned char* srcbuffer,
		__u32 srcformat, unsigned char** dstbuffer, __u32 dstformat)
{
	int srcpixfmt, dstpixfmt;
	int realsrcpixfmt, realdstpixfmt;
	const unsigned char* srcplanes[3];
	const unsigned char* dstplanes[3];
	int srcstride[3];
	int dststride[3];

	srcpixfmt = v4l2fmt2pixfmt(srcformat);
	if (srcpixfmt == PIX_FMT_NONE)
		return -1;

	dstpixfmt = v4l2fmt2pixfmt(dstformat);
	if (dstpixfmt == PIX_FMT_NONE)
		return -1;

	// YVU420P and YVU410P are handled as the corresponding YUV formats,
	// but with swapped U and V planes
	if (srcpixfmt == PIX_FMT_YVU420P)
		realsrcpixfmt = PIX_FMT_YUV420P;
	else if (srcpixfmt == PIX_FMT_YVU410P)
		realsrcpixfmt = PIX_FMT_YUV410P;
	else
		realsrcpixfmt = srcpixfmt;

	if (dstpixfmt == PIX_FMT_YVU420P)
		realdstpixfmt = PIX_FMT_YUV420P;
	else if (dstpixfmt == PIX_FMT_YVU410P)
		realdstpixfmt = PIX_FMT_YUV410P;
	else
		realdstpixfmt = dstpixfmt;

	if (setup_planes(srcpixfmt, srcplanes, srcstride, srcbuffer, width, height) == -1)
		return -1;

	int dstsize = avpicture_get_size(realdstpixfmt, width, height);
	if (dstsize < 0)
	{
		BLTS_ERROR("avpicture_get_size failed with result %d\n", dstsize);
		return -1;
	}
	*dstbuffer = malloc(dstsize);
	if (*dstbuffer == NULL)
		return -1;

	if (setup_planes(dstpixfmt, dstplanes, dststride, *dstbuffer, width, height) == -1)
		return -1;

	struct SwsContext* swscontext = sws_getContext(width, height, realsrcpixfmt,
		width, height, realdstpixfmt,
		SWS_POINT, NULL, NULL, NULL);
	if (swscontext == NULL)
	{
		BLTS_ERROR("sws_getContext failed\n");
		return -2;
	}

	int res = sws_scale(swscontext, (unsigned char**)srcplanes, srcstride, 0,
			height, (unsigned char**)dstplanes, dststride);
	if (res <= 0)
	{
		BLTS_ERROR("sws_scale failed with result %d\n", res);
		sws_freeContext(swscontext);
		return -2;
	}

	sws_freeContext(swscontext);

	return dstsize;
}
