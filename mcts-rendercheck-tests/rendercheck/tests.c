/*
 * Copyright © 2004 Eric Anholt
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
#include <string.h>
#include <math.h>

#include "rendercheck.h"

static XRenderPictFormat **format_list;
static int nformats;
static int argb32index;

/* Note: changing the order of these colors may disrupt tests that depend on
 * specific colors.  Just add to the end if you need.  These are
 * not premultiplied, but will be in main().
 */
color4d colors[] = {
	{1.0, 1.0, 1.0, 1.0},
	{1.0, 0, 0, 1.0},
	{0, 1.0, 0, 1.0},
	{0, 0, 1.0, 1.0},
	{0.5, 0, 0, .5},
};

/* Convenience pointers to 1x1 repeating colors */
picture_info *argb32white, *argb32red, *argb32green, *argb32blue;

int num_colors = sizeof(colors) / sizeof(colors[0]);

struct op_info ops[] = {
	{PictOpClear, "Clear"},
	{PictOpSrc, "Src"},
	{PictOpDst, "Dst"},
	{PictOpOver, "Over"},
	{PictOpOverReverse, "OverReverse"},
	{PictOpIn, "In"},
	{PictOpInReverse, "InReverse"},
	{PictOpOut, "Out"},
	{PictOpOutReverse, "OutReverse"},
	{PictOpAtop, "Atop"},
	{PictOpAtopReverse, "AtopReverse"},
	{PictOpXor, "Xor"},
	{PictOpAdd, "Add"},
	{PictOpSaturate, "Saturate"},
	{PictOpDisjointClear, "DisjointClear"},
	{PictOpDisjointSrc, "DisjointSrc"},
	{PictOpDisjointDst, "DisjointDst"},
	{PictOpDisjointOver, "DisjointOver"},
	{PictOpDisjointOverReverse, "DisjointOverReverse"},
	{PictOpDisjointIn, "DisjointIn"},
	{PictOpDisjointInReverse, "DisjointInReverse"},
	{PictOpDisjointOut, "DisjointOut"},
	{PictOpDisjointOutReverse, "DisjointOutReverse"},
	{PictOpDisjointAtop, "DisjointAtop"},
	{PictOpDisjointAtopReverse, "DisjointAtopReverse"},
	{PictOpDisjointXor, "DisjointXor"},
	{PictOpConjointClear, "ConjointClear"},
	{PictOpConjointSrc, "ConjointSrc"},
	{PictOpConjointDst, "ConjointDst"},
	{PictOpConjointOver, "ConjointOver"},
	{PictOpConjointOverReverse, "ConjointOverReverse"},
	{PictOpConjointIn, "ConjointIn"},
	{PictOpConjointInReverse, "ConjointInReverse"},
	{PictOpConjointOut, "ConjointOut"},
	{PictOpConjointOutReverse, "ConjointOutReverse"},
	{PictOpConjointAtop, "ConjointAtop"},
	{PictOpConjointAtopReverse, "ConjointAtopReverse"},
	{PictOpConjointXor, "ConjointXor"},
};

int num_ops = sizeof(ops) / sizeof(ops[0]);

#define round_pix(pix, mask) \
	((double)((int)(pix * (mask ) + .5)) / (double)(mask))

void
color_correct(picture_info *pi, color4d *color)
{
	if (!pi->format->direct.redMask) {
		color->r = 0.0;
		color->g = 0.0;
		color->b = 0.0;
	} else {
		color->r = round_pix(color->r, pi->format->direct.redMask);
		color->g = round_pix(color->g, pi->format->direct.greenMask);
		color->b = round_pix(color->b, pi->format->direct.blueMask);
	}
	if (!pi->format->direct.alphaMask)
		color->a = 1.0;
	else
		color->a = round_pix(color->a, pi->format->direct.alphaMask);
}

void
get_pixel(Display *dpy, picture_info *pi, int x, int y, color4d *color)
{
	XImage *image;
	unsigned long val;
	unsigned long rm, gm, bm, am;
	XRenderDirectFormat *layout = &pi->format->direct;

	image = XGetImage(dpy, pi->d, x, y, 1, 1, 0xffffffff, ZPixmap);

	val = XGetPixel(image, 0, 0);

	rm = (unsigned long)layout->redMask << layout->red;
	gm = (unsigned long)layout->greenMask << layout->green;
	bm = (unsigned long)layout->blueMask << layout->blue;
	am = (unsigned long)layout->alphaMask << layout->alpha;
	if (am != 0)
		color->a = (double)(val & am) / (double)am;
	else
		color->a = 1.0;
	if (rm != 0) {
		color->r = (double)(val & rm) / (double)rm;
		color->g = (double)(val & gm) / (double)gm;
		color->b = (double)(val & bm) / (double)bm;
	} else {
		color->r = 0.0;
		color->g = 0.0;
		color->b = 0.0;
	}
	XDestroyImage(image);
}

int
eval_diff(char *name, color4d *expected, color4d *test, int x, int y,
    Bool verbose)
{
	double rscale, gscale, bscale, ascale;
	double rdiff, gdiff, bdiff, adiff, diff;

	/* XXX: Need to be provided mask shifts so we can produce useful error
	 * values.
	 */
	rscale = 1.0 * (1 << 5);
	gscale = 1.0 * (1 << 6);
	bscale = 1.0 * (1 << 5);
	ascale = 1.0 * 32;
	rdiff = fabs(test->r - expected->r) * rscale;
	bdiff = fabs(test->g - expected->g) * gscale;
	gdiff = fabs(test->b - expected->b) * bscale;
	adiff = fabs(test->a - expected->a) * ascale;
	/*rdiff = log2(1.0 + rdiff);
	gdiff = log2(1.0 + gdiff);
	bdiff = log2(1.0 + bdiff);
	adiff = log2(1.0 + adiff);*/
	diff = max(max(max(rdiff, gdiff), bdiff), adiff);
	if (diff > 3.0) {
		printf("%s test error of %.4f at (%d, %d) --\n"
		    "           R    G    B    A\n"
		    "got:       %.2f %.2f %.2f %.2f\n"
		    "expected:  %.2f %.2f %.2f %.2f\n", name, diff, x, y,
		    test->r, test->g, test->b, test->a,
		    expected->r, expected->g, expected->b, expected->a);
		return FALSE;
	} else if (verbose) {
		printf("%s test succeeded at (%d, %d) with %.4f: "
		    "%.2f %.2f %.2f %.2f\n", name, x, y, diff,
		    expected->r, expected->g, expected->b, expected->a);
	}
	return TRUE;
}

void
argb_fill(Display *dpy, picture_info *p, int x, int y, int w, int h, float a,
    float r, float g, float b)
{
	XRenderColor rendercolor;

	rendercolor.red = r * 65535;
	rendercolor.green = g * 65535;
	rendercolor.blue = b * 65535;
	rendercolor.alpha = a * 65535;

	XRenderFillRectangle(dpy, PictOpSrc, p->pict, &rendercolor, x, y, w, h);
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Create a set of direct format XRenderPictFormats for later use.  This lets
 * us get more formats than just the standard required set, and lets us attach
 * names to them.
 */
static void
create_formats_list(Display *dpy)
{
    int i;
    int nformats_allocated = 5;
    XRenderPictFormat templ;

    memset(&templ, 0, sizeof(templ));
    templ.type = PictTypeDirect;

    format_list = malloc(sizeof(XRenderPictFormat *) * nformats_allocated);
    if (format_list == NULL)
	errx(1, "malloc error");
    nformats = 0;

    argb32index = -1;
    for (i = 0; ; i++) {
	char name[20];
	int alphabits, redbits;

	if (nformats + 1 == nformats_allocated) {
	    nformats_allocated *= 2;
	    format_list = realloc(format_list, sizeof(XRenderPictFormat *) *
		nformats_allocated);
	    if (format_list == NULL)
		errx(1, "realloc error");
	}

	format_list[nformats] = XRenderFindFormat(dpy, PictFormatType, &templ,
	    i);
	if (format_list[nformats] == NULL)
	    break;

	alphabits = bit_count(format_list[nformats]->direct.alphaMask);
	redbits = bit_count(format_list[nformats]->direct.redMask);

	/* Our testing code isn't all that hot, so don't bother trying at
	 * the low depths yet.
	 */
	if ((redbits >= 1 && redbits <= 4) ||
	    (alphabits >= 1 && alphabits <= 4))
	{
	    continue;
	}

	describe_format(name, 20, format_list[nformats]);

	if (format_whitelist_len != 0) {
	    Bool ok = FALSE;
	    int j;

	    for (j = 0; j < format_whitelist_len; j++) {
		if (strcmp(format_whitelist[j], name) == 0) {
		    ok = TRUE;
		    break;
		}
	    }
	    if (!ok) {
		printf("Ignoring server-supported format: %s\n", name);
		continue;
	    }
	}

	if (format_list[nformats] == XRenderFindStandardFormat(dpy,
	    PictStandardARGB32))
	{
	    argb32index = nformats;
	}

	printf("Found server-supported format: %s\n", name);

	nformats++;
    }
    if (argb32index == -1) {
	errx(1, "required ARGB32 format not found");
    }
}

Bool
do_tests(Display *dpy, picture_info *win)
{
	int i, j, src, dst = 0, mask;
	int num_dests;
	picture_info *dests, *pictures_1x1, *pictures_10x10, picture_3x3, *pictures_solid;
	int success_mask = 0, tests_passed = 0, tests_total = 0;
	int num_tests;

	create_formats_list(dpy);

	num_dests = nformats;
	dests = (picture_info *)malloc(num_dests * sizeof(dests[0]));
	if (dests == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_dests; i++) {
		dests[i].format = format_list[i];
		dests[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0),
		    win_width, win_height, dests[i].format->depth);
		dests[i].pict = XRenderCreatePicture(dpy, dests[i].d,
		    dests[i].format, 0, NULL);

		dests[i].name = (char *)malloc(20);
		if (dests[i].name == NULL)
			errx(1, "malloc error");
		describe_format(dests[i].name, 20, dests[i].format);
	}

	pictures_1x1 = (picture_info *)malloc(num_colors * nformats *
	    sizeof(picture_info));
	if (pictures_1x1 == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_colors * nformats; i++) {
		XRenderPictureAttributes pa;
		color4d *c = &colors[i / nformats];

		/* The standard PictFormat numbers go from 0 to 4 */
		pictures_1x1[i].format = format_list[i % nformats];
		pictures_1x1[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0), 1,
		    1, pictures_1x1[i].format->depth);
		pa.repeat = TRUE;
		pictures_1x1[i].pict = XRenderCreatePicture(dpy,
		    pictures_1x1[i].d, pictures_1x1[i].format, CPRepeat, &pa);

		pictures_1x1[i].name = (char *)malloc(20);
		if (pictures_1x1[i].name == NULL)
			errx(1, "malloc error");
		sprintf(pictures_1x1[i].name, "1x1R ");
		describe_format(pictures_1x1[i].name +
		    strlen(pictures_1x1[i].name), 20 -
		    strlen(pictures_1x1[i].name), pictures_1x1[i].format);

		argb_fill(dpy, &pictures_1x1[i], 0, 0, 1, 1,
		    c->a, c->r, c->g, c->b);

		pictures_1x1[i].color = *c;
		color_correct(&pictures_1x1[i], &pictures_1x1[i].color);
	}
	argb32white = &pictures_1x1[0 * nformats + argb32index];
	argb32red = &pictures_1x1[1 * nformats + argb32index];
	argb32green = &pictures_1x1[2 * nformats + argb32index];
	argb32blue = &pictures_1x1[3 * nformats + argb32index];

	pictures_10x10 = (picture_info *)malloc(num_colors * nformats *
	    sizeof(picture_info));
	if (pictures_10x10 == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_colors * nformats; i++) {
		XRenderPictureAttributes pa;
		color4d *c = &colors[i / nformats];

		/* The standard PictFormat numbers go from 0 to 4 */
		pictures_10x10[i].format = format_list[i % nformats];
		pictures_10x10[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0), 10,
		    10, pictures_10x10[i].format->depth);
		pa.repeat = TRUE;
		pictures_10x10[i].pict = XRenderCreatePicture(dpy,
		    pictures_10x10[i].d, pictures_10x10[i].format, 0, NULL);

		pictures_10x10[i].name = (char *)malloc(20);
		if (pictures_10x10[i].name == NULL)
			errx(1, "malloc error");
		sprintf(pictures_10x10[i].name, "10x10 ");
		describe_format(pictures_10x10[i].name +
		    strlen(pictures_10x10[i].name), 20 -
		    strlen(pictures_10x10[i].name), pictures_10x10[i].format);

		argb_fill(dpy, &pictures_10x10[i], 0, 0, 10, 10,
		    c->a, c->r, c->g, c->b);

		pictures_10x10[i].color = *c;
		color_correct(&pictures_10x10[i], &pictures_10x10[i].color);
	}

	picture_3x3.d = XCreatePixmap(dpy, RootWindow(dpy, 0), 3, 3, 32);
	picture_3x3.format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
	picture_3x3.pict = XRenderCreatePicture(dpy, picture_3x3.d,
	    picture_3x3.format, 0, NULL);
	picture_3x3.name = "3x3 sample picture";
	for (i = 0; i < 9; i++) {
		int x = i % 3;
		int y = i / 3;
		color4d *c = &colors[i % num_colors];

		argb_fill(dpy, &picture_3x3, x, y, 1, 1, c->a, c->r, c->g, c->b);
	}

        pictures_solid = malloc(num_colors * sizeof(picture_info));
	for (i = 0; i < num_colors; i++) {
            XRenderColor c;
            pictures_solid[i].color = colors[i];
            c.alpha = (int)(colors[i].a*65535);
            c.red = (int)(colors[i].r*65535);
            c.green = (int)(colors[i].g*65535);
            c.blue = (int)(colors[i].b*65535);
            pictures_solid[i].pict = XRenderCreateSolidFill(dpy, &c);
            pictures_solid[i].format = format_list[argb32index];
            pictures_solid[i].name = "Solid";
        }

#define RECORD_RESULTS()					\
do {								\
	group_ok = group_ok && ok;				\
	if (ok)							\
		tests_passed++;					\
	tests_total++;						\
} while (0)

	num_tests = num_colors * nformats;

	if (enabled_tests & TEST_FILL) {
		Bool ok, group_ok = TRUE;

		printf("Beginning testing of filling of 1x1R pictures\n");
		for (i = 0; i < num_tests; i++) {
			ok = fill_test(dpy, win, &pictures_1x1[i]);
			RECORD_RESULTS();
		}

		printf("Beginning testing of filling of 10x10 pictures\n");
		for (i = 0; i < num_tests; i++) {
			ok = fill_test(dpy, win, &pictures_10x10[i]);
			RECORD_RESULTS();
		}
		if (group_ok)
			success_mask |= TEST_FILL;
	}

	if (enabled_tests & TEST_DSTCOORDS) {
		Bool ok, group_ok = TRUE;

		printf("Beginning dest coords test\n");
		for (i = 0; i < 2; i++) {
			ok = dstcoords_test(dpy, win,
			    i == 0 ? PictOpSrc : PictOpOver, win,
			    argb32white, argb32red);
			RECORD_RESULTS();
		}
		if (group_ok)
			success_mask |= TEST_DSTCOORDS;
	}

	if (enabled_tests & TEST_SRCCOORDS) {
		Bool ok, group_ok = TRUE;

		printf("Beginning src coords test\n");
		ok = srccoords_test(dpy, win, argb32white, FALSE);
		RECORD_RESULTS();
		if (group_ok)
			success_mask |= TEST_SRCCOORDS;
	}

	if (enabled_tests & TEST_MASKCOORDS) {
		Bool ok, group_ok = TRUE;

		printf("Beginning mask coords test\n");
		ok = srccoords_test(dpy, win, argb32white, TRUE);
		RECORD_RESULTS();
		if (group_ok)
			success_mask |= TEST_MASKCOORDS;
	}

	if (enabled_tests & TEST_TSRCCOORDS) {
		Bool ok, group_ok = TRUE;

		printf("Beginning transformed src coords test\n");
		ok = trans_coords_test(dpy, win, argb32white, FALSE);
		RECORD_RESULTS();

		printf("Beginning transformed src coords test 2\n");
		ok = trans_srccoords_test_2(dpy, win, argb32white, FALSE);
		RECORD_RESULTS();
		if (group_ok)
			success_mask |= TEST_TSRCCOORDS;
	}

	if (enabled_tests & TEST_TMASKCOORDS) {
		Bool ok, group_ok = TRUE;

		printf("Beginning transformed mask coords test\n");
		ok = trans_coords_test(dpy, win, argb32white, TRUE);
		RECORD_RESULTS();

		printf("Beginning transformed mask coords test 2\n");
		ok = trans_srccoords_test_2(dpy, win, argb32white, TRUE);
		RECORD_RESULTS();

		if (group_ok)
			success_mask |= TEST_TMASKCOORDS;
	}

	if (enabled_tests & TEST_BLEND) {
		Bool ok, group_ok = TRUE;

		for (i = 0; i < num_ops; i++) {
		    if (ops[i].disabled)
			continue;

		    for (j = 0; j <= num_dests; j++) {
			picture_info *pi;

			if (j != num_dests)
				pi = &dests[j];
			else
				pi = win;
			printf("Beginning %s blend test on %s\n", ops[i].name,
			    pi->name);

			for (src = 0; src < num_tests; src++) {
				for (dst = 0; dst < num_tests; dst++) {
					ok = blend_test(dpy, win, pi, i,
					    &pictures_1x1[src],
					    &pictures_1x1[dst]);
					RECORD_RESULTS();
					ok = blend_test(dpy, win, pi, i,
					    &pictures_10x10[src],
					    &pictures_1x1[dst]);
					RECORD_RESULTS();
				}
			}
                        for (src = 0; src < num_colors; src++) {
                            for (dst = 0; dst < num_colors; dst++) {
                                ok = blend_test(dpy, win, pi, i, &pictures_solid[src], &pictures_1x1[dst]);
				RECORD_RESULTS();
                            }
                        }
		    }
		}
		if (group_ok)
			success_mask |= TEST_BLEND;
	}

	if (enabled_tests & TEST_COMPOSITE) {
		Bool ok, group_ok = TRUE;

		for (i = 0; i < num_ops; i++) {
		    if (ops[i].disabled)
			continue;

		    for (j = 0; j <= num_dests; j++) {
			picture_info *pi;

			if (j != num_dests)
				pi = &dests[j];
			else
				pi = win;
			printf("Beginning %s composite mask test on %s\n",
			    ops[i].name, pi->name);

			for (src = 0; src < num_tests; src++) {
			    for (mask = 0; mask < num_tests; mask++) {
				for (dst = 0; dst < num_tests; dst++) {
					ok = composite_test(dpy, win, pi, i,
					    &pictures_10x10[src],
					    &pictures_10x10[mask],
					    &pictures_1x1[dst], FALSE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_1x1[src],
					    &pictures_10x10[mask],
					    &pictures_1x1[dst], FALSE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_10x10[src],
					    &pictures_1x1[mask],
					    &pictures_1x1[dst], FALSE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_1x1[src],
					    &pictures_1x1[mask],
					    &pictures_1x1[dst], FALSE, TRUE);
					RECORD_RESULTS();
				}
			    }
			}
		    }
		}
		if (group_ok)
			success_mask |= TEST_COMPOSITE;
	}

	if (enabled_tests & TEST_CACOMPOSITE) {
		Bool ok, group_ok = TRUE;

		for (i = 0; i < num_ops; i++) {
		    if (ops[i].disabled)
			continue;

		    for (j = 0; j <= num_dests; j++) {
			picture_info *pi;

			if (j != num_dests)
				pi = &dests[j];
			else
				pi = win;
			printf("Beginning %s composite CA mask test on %s\n",
			    ops[i].name, pi->name);

			for (src = 0; src < num_tests; src++) {
			    for (mask = 0; mask < num_tests; mask++) {
				for (dst = 0; dst < num_tests; dst++) {
					ok = composite_test(dpy, win, pi, i,
					    &pictures_10x10[src],
					    &pictures_10x10[mask],
					    &pictures_1x1[dst], TRUE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_1x1[src],
					    &pictures_10x10[mask],
					    &pictures_1x1[dst], TRUE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_10x10[src],
					    &pictures_1x1[mask],
					    &pictures_1x1[dst], TRUE, TRUE);
					RECORD_RESULTS();
					ok = composite_test(dpy, win, pi, i,
					    &pictures_1x1[src],
					    &pictures_1x1[mask],
					    &pictures_1x1[dst], TRUE, TRUE);
					RECORD_RESULTS();
				}
			    }
			}
		    }
		}
		if (group_ok)
			success_mask |= TEST_CACOMPOSITE;
	}

        if (enabled_tests & TEST_GRADIENTS) {
	    Bool ok, group_ok = TRUE;

	    printf("Beginning render to linear gradient test\n");
	    ok = render_to_gradient_test(dpy, &pictures_1x1[0]);
	    RECORD_RESULTS();

            for (i = 0; i < num_ops; i++) {
		if (ops[i].disabled)
		    continue;

                for (j = 0; j <= num_dests; j++) {
                    picture_info *pi;
                    
                    if (j != num_dests)
                        pi = &dests[j];
                    else
                        pi = win;
                    printf("Beginning %s linear gradient test on %s\n",
                           ops[i].name, pi->name);
                    
                    for (src = 0; src < num_tests; src++) {
			ok = linear_gradient_test(dpy, win, pi, i,
						  &pictures_1x1[src]);
			RECORD_RESULTS();
                    }
                }
            }
	    if (group_ok)
		 success_mask |= TEST_GRADIENTS;
        }

        if (enabled_tests & TEST_REPEAT) {
	    Bool ok, group_ok = TRUE;

            for (i = 0; i < num_ops; i++) {
		if (ops[i].disabled)
		    continue;

                for (j = 0; j <= num_dests; j++) {
                    picture_info *pi;
                    
                    if (j != num_dests)
                        pi = &dests[j];
                    else
                        pi = win;
                    printf("Beginning %s src repeat test on %s\n",
                           ops[i].name, pi->name);
		    /* Test with white dest, and generated repeating src
		     * consisting of colors 1 and 2 (r, g).
		     */
		    ok = repeat_test(dpy, win, pi, i, argb32white, argb32red,
		        argb32green, FALSE);
		    RECORD_RESULTS();

                    printf("Beginning %s mask repeat test on %s\n",
                           ops[i].name, pi->name);
		    /* Test with white dest, translucent red src, and generated
		     * repeating mask consisting of colors 1 and 2 (r, g).
		     */
		    ok = repeat_test(dpy, win, pi, i, argb32white, argb32red,
		        argb32green, TRUE);
		    RECORD_RESULTS();
                }
            }
	    if (group_ok)
		success_mask |= TEST_REPEAT;
        }

	if (enabled_tests & TEST_TRIANGLES) {
	    Bool ok, group_ok = TRUE;

	    for (i = 0; i < num_ops; i++) {
		if (ops[i].disabled)
		    continue;

		for (j = 0; j <= num_dests; j++) {
			picture_info *pi;

			if (j != num_dests)
			    pi = &dests[j];
			else
			    pi = win;

			printf("Beginning %s Triangles test on %s\n",
			    ops[i].name, pi->name);
			ok = triangles_test(dpy, win, pi, i,
			    argb32red, argb32white);
			RECORD_RESULTS();

			printf("Beginning %s TriStrip test on %s\n",
			    ops[i].name, pi->name);
			ok = tristrip_test(dpy, win, pi, i,
			    argb32red, argb32white);
			RECORD_RESULTS();

			printf("Beginning %s TriFan test on %s\n",
			    ops[i].name, pi->name);
			ok = trifan_test(dpy, win, pi, i,
			    argb32red, argb32white);
			RECORD_RESULTS();
		}
	    }
	    if (group_ok)
		success_mask |= TEST_TRIANGLES;
	}

        if (enabled_tests & TEST_BUG7366) {
	    Bool ok, group_ok = TRUE;

	    ok = bug7366_test(dpy);
	    RECORD_RESULTS();

	    if (group_ok)
		success_mask |= TEST_BUG7366;
	}

	printf("%d tests passed of %d total\n", tests_passed, tests_total);

	return tests_passed == tests_total;
}

/**
 * \brief copies the contents of a picture to the window.
 *
 * This is used in tests so that the user sees flashing colors indicating that
 * rendercheck is really doing things.  The minimalrendering commandline option
 * indicates that this behavior should be disabled.
 */
void
copy_pict_to_win(Display *dpy, picture_info *pict, picture_info *win,
    int width, int height)
{
	if (pict == win || minimalrendering)
		return;

	XRenderComposite(dpy, PictOpSrc, pict->pict, 0, win->pict, 0, 0,
	    0, 0, 0, 0, width, height);
}
