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
#include <stdlib.h>

#include "rendercheck.h"

static int dot_colors[5][5] = {
	{7, 7, 7, 7, 7},
	{7, 7, 7, 7, 7},
	{7, 0, 7, 7, 7},
	{7, 7, 7, 7, 7},
	{7, 7, 7, 7, 7},
};

static picture_info *create_dot_picture(Display *dpy)
{
	picture_info *p;
	int i;

	p = malloc(sizeof(picture_info));

	p->d = XCreatePixmap(dpy, RootWindow(dpy, 0), 5, 5, 32);
	p->format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
	p->pict = XRenderCreatePicture(dpy, p->d, p->format, 0, NULL);
	p->name = "dot picture";

	for (i = 0; i < 25; i++) {
		int x = i % 5;
		int y = i / 5;
		color4d *c = &colors[dot_colors[x][y]];

		argb_fill(dpy, p, x, y, 1, 1, c->a, c->r, c->g, c->b);
	}

	return p;
}

static void destroy_dot_picture(Display *dpy, picture_info *p)
{
	XRenderFreePicture(dpy, p->pict);
	XFreePixmap(dpy, p->d);
	free(p);
}

static void init_transform (XTransform *t)
{
	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			t->matrix[i][j] = XDoubleToFixed((i == j) ? 1 : 0);
}

/* Test drawing a 5x5 source image scaled 8x, as either a source or mask.
 */
Bool
trans_coords_test(Display *dpy, picture_info *win, picture_info *white,
    Bool test_mask)
{
	color4d tested;
	int x, y;
	Bool failed = FALSE;
	int tested_colors[40][40], expected_colors[40][40];
	XTransform t;
	picture_info *src;

	src = create_dot_picture(dpy);
	if (src == NULL) {
		fprintf(stderr, "couldn't allocate picture for test\n");
		return FALSE;
	}

	init_transform(&t);
	t.matrix[2][2] = XDoubleToFixed(8);

	XRenderSetPictureTransform(dpy, src->pict, &t);

	if (!test_mask)
		XRenderComposite(dpy, PictOpSrc, src->pict, 0,
		    win->pict, 0, 0, 0, 0, 0, 0, 40, 40);
	else {
		XRenderComposite(dpy, PictOpSrc, white->pict, src->pict,
		    win->pict, 0, 0, 0, 0, 0, 0, 40, 40);
	}

	for (x = 0; x < 40; x++) {
	    for (y = 0; y < 40; y++) {
		int src_sample_x, src_sample_y;

		src_sample_x = x / 8;
		src_sample_y = y / 8;
		expected_colors[x][y] = dot_colors[src_sample_x][src_sample_y];

		get_pixel(dpy, win, x, y, &tested);

		if (tested.r == 1.0 && tested.g == 1.0 && tested.b == 1.0) {
			tested_colors[x][y] = 0;
		} else if (tested.r == 0.0 && tested.g == 0.0 &&
		    tested.b == 0.0) {
			tested_colors[x][y] = 7;
		} else {
			tested_colors[x][y] = 9;
		}
		if (tested_colors[x][y] != expected_colors[x][y])
			failed = TRUE;
	    }
	}

	if (failed) {
		printf("%s transform coordinates test failed.\n",
		    test_mask ? "mask" : "src");
		printf("expected vs tested:\n");
		for (y = 0; y < 40; y++) {
			for (x = 0; x < 40; x++)
				printf("%d", expected_colors[x][y]);
			printf(" ");
			for (x = 0; x < 40; x++)
				printf("%d", tested_colors[x][y]);
			printf("\n");
		}
		printf(" vs tested (same)\n");
		for (y = 0; y < 40; y++) {
			for (x = 0; x < 40; x++)
				printf("%d", tested_colors[x][y]);
			printf("\n");
		}
	}

	init_transform(&t);

	XRenderSetPictureTransform(dpy, src->pict, &t);

	destroy_dot_picture(dpy, src);

	return !failed;
}
