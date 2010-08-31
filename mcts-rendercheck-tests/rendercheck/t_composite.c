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

/* Test a composite of a given operation, source, mask, and destination picture.
 * Fills the window, and samples from the 0,0 pixel corner.
 */
Bool
composite_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *mask_color, picture_info *dst_color,
    Bool componentAlpha, Bool print_errors)
{
	color4d expected, tested, tdst, tmsk;
	char testname[40];
	XRenderPictureAttributes pa;
	Bool success = TRUE;
	int i;

	if (componentAlpha) {
		pa.component_alpha = TRUE;
		XRenderChangePicture(dpy, mask_color->pict, CPComponentAlpha,
		    &pa);
	}
	for (i = 0; i < pixmap_move_iter; i++) {
		XRenderComposite(dpy, PictOpSrc, dst_color->pict, 0, dst->pict,
		    0, 0, 0, 0, 0, 0, TEST_WIDTH, TEST_HEIGHT);
		XRenderComposite(dpy, ops[op].op, src_color->pict,
		    mask_color->pict, dst->pict, 0, 0, 0, 0, 0, 0,
		    TEST_WIDTH, TEST_HEIGHT);
	}
	get_pixel(dpy, dst, 0, 0, &tested);
	copy_pict_to_win(dpy, dst, win, TEST_WIDTH, TEST_HEIGHT);

	if (componentAlpha) {
		pa.component_alpha = FALSE;
		XRenderChangePicture(dpy, mask_color->pict, CPComponentAlpha,
		    &pa);
	}

	if (componentAlpha && mask_color->format->direct.redMask == 0) {
		/* Ax component-alpha masks expand alpha into all color
		 * channels.  XXX: This should be located somewhere generic.
		 */
		tmsk.a = mask_color->color.a;
		tmsk.r = mask_color->color.a;
		tmsk.g = mask_color->color.a;
		tmsk.b = mask_color->color.a;
	} else
		tmsk = mask_color->color;

	tdst = dst_color->color;
	color_correct(dst, &tdst);
	do_composite(ops[op].op, &src_color->color, &tmsk, &tdst,
	    &expected, componentAlpha);
	color_correct(dst, &expected);

	snprintf(testname, 40, "%s %scomposite", ops[op].name,
	    componentAlpha ? "CA " : "");
	if (!eval_diff(testname, &expected, &tested, 0, 0, is_verbose &&
	    print_errors)) {
		if (print_errors)
			printf("src color: %.2f %.2f %.2f %.2f\n"
			    "msk color: %.2f %.2f %.2f %.2f\n"
			    "dst color: %.2f %.2f %.2f %.2f\n",
			    src_color->color.r, src_color->color.g,
			    src_color->color.b, src_color->color.a,
			    mask_color->color.r, mask_color->color.g,
			    mask_color->color.b, mask_color->color.a,
			    dst_color->color.r, dst_color->color.g,
			    dst_color->color.b, dst_color->color.a);
		printf("src: %s, mask: %s, dst: %s\n", src_color->name,
		    mask_color->name, dst->name);
		success = FALSE;
	} else if (is_verbose) {
		printf("src: %s, mask: %s, dst: %s\n", src_color->name,
		    mask_color->name, dst->name);
	}
	return success;
}
