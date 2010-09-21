/* blts-pixelformat.h -- Pixel format conversion

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

#ifndef _BLTS_PIXFMT_H_
#define _BLTS_PIXFMT_H_

#include <asm/types.h>

/** Converts the provided image from the source format to the destination format.
 * The destination buffer dstbuffer is allocated in the function, and must be
 * freed by the caller.
 *
 * @param srcbuffer The image data to be converted.
 * @param srcformat V4L2 format code for the source image.
 * @param width Width of the image in pixels.
 * @param height Height of the image in pixels.
 * @param dstbuffer After successful conversion, contains a pointer to the destination image.
 * @param dstformat V4L2 format code for the destination image.
 *
 * @return Size of the destination image buffer in bytes, or a negative error code if conversion failed.
 */
int convert_buffer(int width, int height, const unsigned char* srcbuffer,
	__u32 srcformat, unsigned char** dstbuffer, __u32 dstformat);

#endif // _BLTS_PIXFMT_H_
