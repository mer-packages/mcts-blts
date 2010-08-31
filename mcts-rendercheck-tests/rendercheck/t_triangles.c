/*
 * Copyright © 2006 Eric Anholt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <anholt@FreeBSD.org>
 *
 */

#include <stdio.h>

#include "rendercheck.h"

#define TEST_WIDTH	10
#define TEST_HEIGHT	10

/* Test basic functionality of the triangle operations.  We don't care that much
 * probably (nobody has used them yet), but we can trivially test by filling
 * doing two triangles that will exactly cover the rectangle from 2,2 to 4,4.
 */
Bool
triangles_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *dst_color)
{
	XTriangle triangles[2];
	color4d tdst, tsrc;
	int x, y;
	Bool success = TRUE;

	triangles[0].p1.x = XDoubleToFixed(2);
	triangles[0].p1.y = XDoubleToFixed(2);
	triangles[0].p2.x = XDoubleToFixed(4);
	triangles[0].p2.y = XDoubleToFixed(2);
	triangles[0].p3.x = XDoubleToFixed(4);
	triangles[0].p3.y = XDoubleToFixed(4);

	triangles[1].p1.x = XDoubleToFixed(2);
	triangles[1].p1.y = XDoubleToFixed(2);
	triangles[1].p2.x = XDoubleToFixed(2);
	triangles[1].p2.y = XDoubleToFixed(4);
	triangles[1].p3.x = XDoubleToFixed(4);
	triangles[1].p3.y = XDoubleToFixed(4);

	/* Fill the dst to dst_color */
	XRenderComposite(dpy, PictOpSrc, dst_color->pict, None, dst->pict, 0, 0,
	    0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
	/* Paint the triangles with src_color */
	XRenderCompositeTriangles(dpy, ops[op].op, src_color->pict, dst->pict,
	    XRenderFindStandardFormat(dpy, PictStandardA8), 0, 0, triangles, 2);

	copy_pict_to_win(dpy, dst, win, TEST_WIDTH, TEST_HEIGHT);

	/* Color expected outside of the triangles */
	tdst = dst_color->color;
	color_correct(dst, &tdst);

	/* Color expected inside of the triangles */
	do_composite(ops[op].op, &src_color->color, NULL, &tdst, &tsrc, FALSE);
	color_correct(dst, &tsrc);

	for (x = 0; x < 5; x++) {
	    for (y = 0; y < 5; y++) {
		color4d expected, tested;

		if (x >= 2 && x < 4 && y >= 2 && y < 4) {
			expected = tsrc;
		} else {
			expected = tdst;
		}

		get_pixel(dpy, dst, x, y, &tested);

		if (!eval_diff("triangles", &expected, &tested, x, y,
		    is_verbose))
		{
			success = FALSE;
		}
	    }
	}
	if (!success) {
		printf("src color: %.2f %.2f %.2f %.2f\n"
		    "dst color: %.2f %.2f %.2f %.2f\n",
		    src_color->color.r, src_color->color.g,
		    src_color->color.b, src_color->color.a,
		    dst_color->color.r, dst_color->color.g,
		    dst_color->color.b, dst_color->color.a);
	}

	return success;
}

Bool
trifan_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *dst_color)
{
	XPointFixed points[4];
	color4d tdst, tsrc;
	int x, y;
	Bool success = TRUE;

	points[0].x = XDoubleToFixed(2);
	points[0].y = XDoubleToFixed(2);
	points[1].x = XDoubleToFixed(4);
	points[1].y = XDoubleToFixed(2);
	points[2].x = XDoubleToFixed(4);
	points[2].y = XDoubleToFixed(4);
	points[3].x = XDoubleToFixed(2);
	points[3].y = XDoubleToFixed(4);

	/* Fill the dst to dst_color */
	XRenderComposite(dpy, PictOpSrc, dst_color->pict, None, dst->pict, 0, 0,
	    0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
	/* Paint the triangles with src_color */
	XRenderCompositeTriFan(dpy, ops[op].op, src_color->pict, dst->pict,
	    XRenderFindStandardFormat(dpy, PictStandardA8), 0, 0, points, 4);

	copy_pict_to_win(dpy, dst, win, TEST_WIDTH, TEST_HEIGHT);

	/* Color expected outside of the triangles */
	tdst = dst_color->color;
	color_correct(dst, &tdst);

	/* Color expected inside of the triangles */
	do_composite(ops[op].op, &src_color->color, NULL, &tdst, &tsrc, FALSE);
	color_correct(dst, &tsrc);

	for (x = 0; x < 5; x++) {
	    for (y = 0; y < 5; y++) {
		color4d expected, tested;

		if (x >= 2 && x < 4 && y >= 2 && y < 4) {
			expected = tsrc;
		} else {
			expected = tdst;
		}

		get_pixel(dpy, dst, x, y, &tested);

		if (!eval_diff("triangles", &expected, &tested, x, y,
		    is_verbose))
		{
			success = FALSE;
		}
	    }
	}
	if (!success) {
		printf("src color: %.2f %.2f %.2f %.2f\n"
		    "dst color: %.2f %.2f %.2f %.2f\n",
		    src_color->color.r, src_color->color.g,
		    src_color->color.b, src_color->color.a,
		    dst_color->color.r, dst_color->color.g,
		    dst_color->color.b, dst_color->color.a);
	}

	return success;
}

Bool
tristrip_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *dst_color)
{
	XPointFixed points[4];
	color4d tdst, tsrc;
	int x, y;
	Bool success = TRUE;

	points[0].x = XDoubleToFixed(2);
	points[0].y = XDoubleToFixed(2);
	points[1].x = XDoubleToFixed(4);
	points[1].y = XDoubleToFixed(2);
	points[2].x = XDoubleToFixed(2);
	points[2].y = XDoubleToFixed(4);
	points[3].x = XDoubleToFixed(4);
	points[3].y = XDoubleToFixed(4);

	/* Fill the dst to dst_color */
	XRenderComposite(dpy, PictOpSrc, dst_color->pict, None, dst->pict, 0, 0,
	    0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
	/* Paint the triangles with src_color */
	XRenderCompositeTriStrip(dpy, ops[op].op, src_color->pict, dst->pict,
	    XRenderFindStandardFormat(dpy, PictStandardA8), 0, 0, points, 4);

	copy_pict_to_win(dpy, dst, win, TEST_WIDTH, TEST_HEIGHT);

	/* Color expected outside of the triangles */
	tdst = dst_color->color;
	color_correct(dst, &tdst);

	/* Color expected inside of the triangles */
	do_composite(ops[op].op, &src_color->color, NULL, &tdst, &tsrc, FALSE);
	color_correct(dst, &tsrc);

	for (x = 0; x < 5; x++) {
	    for (y = 0; y < 5; y++) {
		color4d expected, tested;

		if (x >= 2 && x < 4 && y >= 2 && y < 4) {
			expected = tsrc;
		} else {
			expected = tdst;
		}

		get_pixel(dpy, dst, x, y, &tested);

		if (!eval_diff("triangles", &expected, &tested, x, y,
		    is_verbose))
		{
			success = FALSE;
		}
	    }
	}
	if (!success) {
		printf("src color: %.2f %.2f %.2f %.2f\n"
		    "dst color: %.2f %.2f %.2f %.2f\n",
		    src_color->color.r, src_color->color.g,
		    src_color->color.b, src_color->color.a,
		    dst_color->color.r, dst_color->color.g,
		    dst_color->color.b, dst_color->color.a);
	}

	return success;
}
