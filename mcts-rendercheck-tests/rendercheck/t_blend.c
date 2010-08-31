/*
 * Copyright © 2005 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

#include "rendercheck.h"

#define TEST_WIDTH	10
#define TEST_HEIGHT	10

/* Test a composite of a given operation, source, and destination picture.
 * Fills the window, and samples from the 0,0 pixel corner.
 */
Bool
blend_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *dst_color)
{
	color4d expected, tested, tdst;
	char testname[20];
	int i;

	for (i = 0; i < pixmap_move_iter; i++) {
		XRenderComposite(dpy, PictOpSrc, dst_color->pict, 0,
		    dst->pict, 0, 0, 0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
		XRenderComposite(dpy, ops[op].op, src_color->pict, 0,
		    dst->pict, 0, 0, 0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
	}
	get_pixel(dpy, dst, 0, 0, &tested);
	copy_pict_to_win(dpy, dst, win, TEST_WIDTH, TEST_HEIGHT);

	tdst = dst_color->color;
	color_correct(dst, &tdst);
	do_composite(ops[op].op, &src_color->color, NULL, &tdst, &expected,
	    FALSE);
	color_correct(dst, &expected);

	snprintf(testname, 20, "%s blend", ops[op].name);
	if (!eval_diff(testname, &expected, &tested, 0, 0, is_verbose)) {
		char srcformat[20];

		describe_format(srcformat, 20, src_color->format);
		printf("src color: %.2f %.2f %.2f %.2f (%s)\n"
		    "dst color: %.2f %.2f %.2f %.2f\n",
		    src_color->color.r, src_color->color.g,
		    src_color->color.b, src_color->color.a,
		    srcformat,
		    dst_color->color.r, dst_color->color.g,
		    dst_color->color.b, dst_color->color.a);
		printf("src: %s, dst: %s\n", src_color->name, dst->name);
		return FALSE;
	} else if (is_verbose) {
		printf("src: %s, dst: %s\n", src_color->name, dst->name);
	}
	return TRUE;
}
