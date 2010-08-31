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

#include "rendercheck.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

extern int num_ops;
extern int num_colors;

Bool is_verbose = FALSE, minimalrendering = FALSE;
int enabled_tests = ~0;		/* Enable all tests by default */

int format_whitelist_len = 0;
char **format_whitelist;

/* Number of times to repeat operations so that pixmaps will tend to get moved
 * into offscreen memory and allow hardware acceleration.
 */
int pixmap_move_iter = 1;

int win_width = 40;
int win_height = 40;

int
bit_count(int i)
{
	int count;

	count = (i >> 1) & 033333333333;
	count = i - count - ((count >> 1) & 033333333333);
	count = (((count + (count >> 3)) & 030707070707) % 077);
	/* HAKMEM 169 */
	return count;
}

/* This is not complete, but decent enough for now.*/
void
describe_format(char *desc, int len, XRenderPictFormat *format)
{
	char ad[4] = "", rd[4] = "", gd[4] = "", bd[4] = "";
	int ac, rc, gc, bc;
	int ashift;

	ac = bit_count(format->direct.alphaMask);
	rc = bit_count(format->direct.redMask);
	gc = bit_count(format->direct.greenMask);
	bc = bit_count(format->direct.blueMask);

	if (ac != 0) {
		snprintf(ad, 4, "a%d", ac);
		ashift = format->direct.alpha;
	} else if (rc + bc + gc < format->depth) {
		/* There are bits that are not part of A,R,G,B. Mark them with
		 * an x.
		 */
		snprintf(ad, 4, "x%d", format->depth - rc - gc - bc);
		if (format->direct.red == 0 || format->direct.blue == 0)
			ashift = format->depth;
		else
			ashift = 0;
	} else
		ashift = 0;

	if (rc  != 0)
		snprintf(rd, 4, "r%d", rc);
	if (gc  != 0)
		snprintf(gd, 4, "g%d", gc);
	if (bc  != 0)
		snprintf(bd, 4, "b%d", bc);

	if (ashift > format->direct.red) {
		if (format->direct.red > format->direct.blue)
			snprintf(desc, len, "%s%s%s%s", ad, rd, gd, bd);
		else
			snprintf(desc, len, "%s%s%s%s", ad, bd, gd, rd);
	} else {
		if (format->direct.red > format->direct.blue)
			snprintf(desc, len, "%s%s%s%s", rd, gd, bd, ad);
		else
			snprintf(desc, len, "%s%s%s%s", bd, gd, rd, ad);
	}
}

static void
usage (char *program)
{
    fprintf(stderr, "usage: %s [-d|--display display] [-v|--verbose]\n"
	"\t[-t test1,test2,...] [-o op1,op2,...] [-f format1,format2,...]\n"
	"\t[--sync] [--minimalrendering] [--version]\n"
            "\tAvailable tests: fill,dcoords,scoords,mcoords,tscoords,\n"
            "\t\ttmcoords,blend,composite,cacomposite,gradients,repeat,triangles,\n"
            "\t\tbug7366\n",
	program);
    exit(1);
}

int main(int argc, char **argv)
{
	Display *dpy;
	XEvent ev;
	int i, o, maj, min;
	static Bool is_sync = FALSE, print_version = FALSE;
	XWindowAttributes a;
	XSetWindowAttributes as;
	picture_info window;
	char *display = NULL;
	char *test, *format, *opname, *nextname;

	static struct option longopts[] = {
		{ "display",	required_argument,	NULL,	'd' },
		{ "iterations",	required_argument,	NULL,	'i' },
		{ "formats",	required_argument,	NULL,	'f' },
		{ "tests",	required_argument,	NULL,	't' },
		{ "ops",	required_argument,	NULL,	'o' },
		{ "verbose",	no_argument,		NULL,	'v' },
		{ "sync",	no_argument,		&is_sync, TRUE},
		{ "minimalrendering", no_argument,	&minimalrendering,
		    TRUE},
		{ "version",	no_argument,		&print_version, TRUE },
		{ NULL,		0,			NULL,	0 }
	};

	while ((o = getopt_long(argc, argv, "d:i:f:t:o:v", longopts, NULL)) != -1) {
		switch (o) {
		case 'd':
			display = optarg;
			break;
		case 'i':
			pixmap_move_iter = atoi(optarg);
			break;
		case 'o':
			for (i = 0; i < num_ops; i++)
				ops[i].disabled = TRUE;

			nextname = optarg;
			while ((opname = strsep(&nextname, ",")) != NULL) {
				for (i = 0; i < num_ops; i++) {
					if (strcasecmp(ops[i].name, opname) !=
					    0)
						continue;
					ops[i].disabled = FALSE;
					break;
				}
				if (i == num_ops)
					usage(argv[0]);
			}
			break;
		case 'f':
			nextname = optarg;
			for (format_whitelist_len = 0;;format_whitelist_len++)
			{
				if ((format = strsep(&nextname, ",")) == NULL)
					break;
			}

			format_whitelist = malloc(sizeof(char *) *
			    format_whitelist_len);
			if (format_whitelist == NULL)
				errx(1, "malloc");

			/* Now the list is separated by \0s, so use strlen to
			 * step between entries.
			 */
			format = optarg;
			for (i = 0; i < format_whitelist_len; i++) {
				format_whitelist[i] = strdup(format);
				format += strlen(format) + 1;
			}

			break;
		case 't':
			nextname = optarg;

			/* disable all tests */
			enabled_tests = 0;

			while ((test = strsep(&nextname, ",")) != NULL) {
				if (strcmp(test, "fill") == 0) {
					enabled_tests |= TEST_FILL;
				} else if (strcmp(test, "dcoords") == 0) {
					enabled_tests |= TEST_DSTCOORDS;
				} else if (strcmp(test, "scoords") == 0) {
					enabled_tests |= TEST_SRCCOORDS;
				} else if (strcmp(test, "mcoords") == 0) {
					enabled_tests |= TEST_MASKCOORDS;
				} else if (strcmp(test, "tscoords") == 0) {
					enabled_tests |= TEST_TSRCCOORDS;
				} else if (strcmp(test, "tmcoords") == 0) {
					enabled_tests |= TEST_TMASKCOORDS;
				} else if (strcmp(test, "blend") == 0) {
					enabled_tests |= TEST_BLEND;
				} else if (strcmp(test, "composite") == 0) {
					enabled_tests |= TEST_COMPOSITE;
				} else if (strcmp(test, "cacomposite") == 0) {
					enabled_tests |= TEST_CACOMPOSITE;
				} else if (strcmp(test, "gradients") == 0) {
					enabled_tests |= TEST_GRADIENTS;
				} else if (strcmp(test, "repeat") == 0) {
					enabled_tests |= TEST_REPEAT;
				} else if (strcmp(test, "triangles") == 0) {
					enabled_tests |= TEST_TRIANGLES;
				} else if (strcmp(test, "bug7366") == 0) {
					enabled_tests |= TEST_BUG7366;
				} else {
					usage(argv[0]);
				}
			}

			break;
		case 'v':
			is_verbose = TRUE;
			break;
		case 0:
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	/* Print the version string.  Bail out if --version was requested and
	 * continue otherwise.
	 */
	puts(PACKAGE_STRING);
	if (print_version)
		return 0;

	dpy = XOpenDisplay(display);
	if (dpy == NULL)
		errx(1, "Couldn't open display.");
	if (is_sync)
		XSynchronize(dpy, 1);

	if (!XRenderQueryExtension(dpy, &i, &i))
		errx(1, "Render extension missing.");

	XRenderQueryVersion(dpy, &maj, &min);
	if (maj != 0 || min < 1)
		errx(1, "Render extension version too low (%d.%d).", maj, min);

	printf("Render extension version %d.%d\n", maj, min);

	/* Conjoint/Disjoint were added in version 0.2, so disable those ops if
	 * the server doesn't support them.
	 */
	if (min < 2) {
		printf("Server doesn't support conjoint/disjoint ops, disabling.\n");
		num_ops = PictOpSaturate;
	}

	window.d = XCreateSimpleWindow(dpy, RootWindow(dpy, 0), 0, 0, win_width,
	    win_height, 0, 0, WhitePixel(dpy, 0));

	as.override_redirect = True;
	XChangeWindowAttributes(dpy, window.d, CWOverrideRedirect, &as);

	XGetWindowAttributes(dpy, window.d, &a);
	window.format = XRenderFindVisualFormat(dpy, a.visual);
	window.pict = XRenderCreatePicture(dpy, window.d,
	    window.format, 0, NULL);
	window.name = (char *)malloc(20);
	if (window.name == NULL)
		errx(1, "malloc error");
	describe_format(window.name, 20, window.format);
	printf("Window format: %s\n", window.name);
	strncat(window.name, " window", 20);
	XSelectInput(dpy, window.d, ExposureMask);
	XMapWindow(dpy, window.d);

	/* We have to premultiply the alpha into the r, g, b values of the
	 * sample colors.  Render colors are premultiplied with alpha, so r,g,b
	 * can never be greater than alpha.
	 */
	for (i = 0; i < num_colors; i++) {
		colors[i].r *= colors[i].a;
		colors[i].g *= colors[i].a;
		colors[i].b *= colors[i].a;
	}

	while (XNextEvent(dpy, &ev) == 0) {
		if (ev.type == Expose && !ev.xexpose.count) {
			if (do_tests(dpy, &window))
				exit(0);
			else
				exit(1);
		}
	}

        XCloseDisplay(dpy);
	return 0;
}
