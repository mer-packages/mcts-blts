/* xrender_bench.h -- Benchmarks for X Render extension

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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <X11/extensions/Xrender.h>
#include "blts_x11_util.h"

#undef LOGTRACE
#define LOGTRACE(...)

static const char* data_path = "/usr/share/blts-x11-tests/images/";

unsigned int fastrand_val;
#define fastrand() (fastrand_val=fastrand_val*1103515245+12345)

typedef struct
{
	const char *name;
	int pict_op;
} render_test;

typedef struct
{
	int w;
	int h;
	int depth;
	Visual* vis;
	Drawable draw;
	Picture pic;
} xrender_surf;

typedef struct
{
	char* name;
	xrender_surf* src;
	xrender_surf* dst;
} test_scenario;

typedef struct
{
	int		biSize;
	int		biWidth;
	int		biHeight;
	short	biPlanes;
	short	biBitCount;
	int		biCompression;
	int		biSizeImage;
	int		biXPelsPerMeter;
	int		biYPelsPerMeter;
	int		biClrUsed;
	int		biClrImportant;
} bitmap_header;

static render_test all_render_tests[] =
{
	{"PictOpClear", PictOpClear},
	{"PictOpSrc", PictOpSrc},
	{"PictOpDst", PictOpDst},
	{"PictOpOver", PictOpOver},
	{"PictOpOverReverse", PictOpOverReverse},
	{"PictOpIn", PictOpIn},
	{"PictOpInReverse", PictOpInReverse},
	{"PictOpOut", PictOpOut},
	{"PictOpOutReverse", PictOpOutReverse},
	{"PictOpAtop", PictOpAtop},
	{"PictOpAtopReverse", PictOpAtopReverse},
	{"PictOpXor", PictOpXor},
	{"PictOpAdd", PictOpAdd},
	{"PictOpSaturate", PictOpSaturate},
	{"PictOpDisjointClear", PictOpDisjointClear},
	{"PictOpDisjointSrc", PictOpDisjointSrc},
	{"PictOpDisjointDst", PictOpDisjointDst},
	{"PictOpDisjointOver", PictOpDisjointOver},
	{"PictOpDisjointOverReverse", PictOpDisjointOverReverse},
	{"PictOpDisjointIn", PictOpDisjointIn},
	{"PictOpDisjointInReverse", PictOpDisjointInReverse},
	{"PictOpDisjointOut", PictOpDisjointOut},
	{"PictOpDisjointOutReverse", PictOpDisjointOutReverse},
	{"PictOpDisjointAtop", PictOpDisjointAtop},
	{"PictOpDisjointAtopReverse", PictOpDisjointAtopReverse},
	{"PictOpDisjointXor", PictOpDisjointXor},
	{"PictOpConjointClear", PictOpConjointClear},
	{"PictOpConjointSrc", PictOpConjointSrc},
	{"PictOpConjointDst", PictOpConjointDst},
	{"PictOpConjointOver", PictOpConjointOver},
	{"PictOpConjointOverReverse", PictOpConjointOverReverse},
	{"PictOpConjointIn", PictOpConjointIn},
	{"PictOpConjointInReverse", PictOpConjointInReverse},
	{"PictOpConjointOut", PictOpConjointOut},
	{"PictOpConjointOutReverse", PictOpConjointOutReverse},
	{"PictOpConjointAtop", PictOpConjointAtop},
	{"PictOpConjointAtopReverse", PictOpConjointAtopReverse},
	{"PictOpConjointXor", PictOpConjointXor},
	{NULL, 0}
};

static window_struct ws;
static int iterations;

static xrender_surf* xrender_surf_new(Drawable draw, int format, int w, int h)
{
	xrender_surf* rs;
	XRenderPictFormat* fmt;
	XRenderPictureAttributes att;
	Visual *vis = DefaultVisual(ws.display, DefaultScreen(ws.display));

	rs = (xrender_surf*)calloc(1, sizeof(xrender_surf));
	if(format == -1)
	{
		fmt = XRenderFindVisualFormat(ws.display, vis);
		rs->draw = draw;
	}
	else
	{
		fmt = XRenderFindStandardFormat(ws.display, format);
		rs->draw = XCreatePixmap(ws.display, draw, w, h, fmt->depth);
	}

	rs->w = w;
	rs->h = h;
	rs->depth = fmt->depth;
	rs->vis = vis;
	rs->pic = XRenderCreatePicture(ws.display, rs->draw, fmt, 0, &att);
	return rs;
}

static unsigned char* read_bitmap(const char* filename, bitmap_header* header)
{
	unsigned int filedatalen;
	int x, y, ys, sxinc, syinc, k = 0;
	unsigned char* sourcedata;

	FILE* fp = fopen(filename, "rb");
	if(!fp)
	{
		logged_perror("fopen");
		return NULL;
	}

	fseek(fp, 10, SEEK_SET);
	int offset = 0;
	if(fread(&offset, 1, 4, fp) != 4)
	{
		LOGERR("Failed to read bitmap header offset from file %s\n", filename);
		fclose(fp);
		return NULL;
	}

	if(fread(header, 1, sizeof(bitmap_header), fp) != sizeof(bitmap_header))
	{
		LOGERR("Failed to read bitmap header from file %s\n", filename);
		fclose(fp);
		return NULL;
	}

	fseek(fp, offset, SEEK_SET);

	unsigned char* p_data = (unsigned char *) malloc(header->biWidth *
		header->biHeight * 4);
	if(!p_data)
	{
		logged_perror("malloc");
		fclose(fp);
		return NULL;
	}

	filedatalen = header->biWidth * header->biHeight * 4;

	unsigned char* p_temp = (unsigned char *) malloc( filedatalen );
	if(!p_temp)
	{
		logged_perror("malloc");
		fclose(fp);
		return NULL;
	}

	if(fread(p_temp, 1, filedatalen, fp) != filedatalen)
	{
		LOGERR("Failed to read bitmap data from file %s\n", filename);
		fclose(fp);
		free(p_data);
		return NULL;
	}
	fclose(fp);

	sxinc = (header->biWidth << 16) / header->biWidth;
	syinc = (header->biHeight << 16) / header->biHeight;

	k = 0;
	for(y = header->biHeight - 1; y >= 0; y--)
	{
		ys = (y * syinc >> 16) * header->biWidth;
		for(x = 0; x < header->biWidth; x++)
		{
			sourcedata = &p_temp[4 * ((x * sxinc >> 16) + ys)];

			p_data[k] = sourcedata[0];
			p_data[k+1] = sourcedata[1];
			p_data[k+2] = sourcedata[2];
			p_data[k+3] = sourcedata[3];
			k += 4;
		}
	}

	free(p_temp);

	return p_data;
}

static int surface_from_file(xrender_surf* rs, const char* file,
	int semi_transp)
{
	GC gc;
	XGCValues gcv;
	XImage* xim;
	int x, y;
	bitmap_header header;
	char full_name[PATH_MAX];
	unsigned char* data = NULL;
	unsigned char* ptr;
	int a, r, g, b;

	sprintf(full_name, "%s%s", data_path, file);
	data = read_bitmap(full_name, &header);
	if(!data)
	{
		LOGERR("Failed to load bmp %s\n", full_name);
		return -1;
	}

	LOGTRACE("%s, %d, %d, %d\n", full_name, header.biWidth,
		header.biHeight, header.biBitCount);

	gc = XCreateGC(ws.display, rs->draw, 0, &gcv);
	xim = XCreateImage(ws.display, rs->vis, rs->depth, ZPixmap, 0, NULL,
		header.biWidth, header.biHeight, 32, 0);
	xim->data = (char*)malloc(xim->bytes_per_line * xim->height);

	ptr = data;
	for (y = 0; y < header.biHeight; y++)
	{
		for (x = 0; x < header.biWidth; x++)
		{
			b = *ptr++;
			g = *ptr++;
			r = *ptr++;
			a = *ptr++;
			if(semi_transp) a >>= 1;

			r = (r * (a + 1)) >> 8;
			g = (g * (a + 1)) >> 8;
			b = (b * (a + 1)) >> 8;
			XPutPixel(xim, x, y, (a << 24) | (r << 16) | (g << 8) | b);
		}
	}

	XPutImage(ws.display, rs->draw, gc, xim, 0, 0, 0, 0, header.biWidth,
		header.biHeight);
	free(data);
	free(xim->data);
	xim->data = NULL;
	XDestroyImage(xim);
	XFreeGC(ws.display, gc);
	return 0;
}

static test_scenario* create_test_scenarios(int w, int h)
{
	test_scenario* scenarios;
	XTransform xf;

	memset(&xf, 0, sizeof(xf));
	xf.matrix[0][0] = 32768;
	xf.matrix[1][1] = 65536;
	xf.matrix[2][2] = 65536;

	scenarios = (test_scenario*)malloc(sizeof(test_scenario) * 4);
	if(!scenarios)
	{
		logged_perror("malloc");
		return NULL;
	}

	scenarios[0].name = strdup("Plain");
	scenarios[0].src = xrender_surf_new(ws.window, PictStandardRGB24,
		128, 128);
	scenarios[0].dst = xrender_surf_new(ws.window, -1, w, h);
	if(surface_from_file(scenarios[0].dst, "image1.bmp", 0)) return NULL;
	if(surface_from_file(scenarios[0].src, "image2.bmp", 0)) return NULL;

	scenarios[1].name = strdup("Plain With Alpha");
	scenarios[1].src = xrender_surf_new(ws.window, PictStandardARGB32,
		128, 128);
	scenarios[1].dst = xrender_surf_new(ws.window, -1, w, h);
	if(surface_from_file(scenarios[1].src, "image2.bmp", 1)) return NULL;

	scenarios[2].name = strdup("Transformation");
	scenarios[2].src = xrender_surf_new(ws.window, PictStandardRGB24,
		128, 128);
	scenarios[2].dst = xrender_surf_new(ws.window, -1, w, h);
	if(surface_from_file(scenarios[2].src, "image2.bmp", 0)) return NULL;
	XRenderSetPictureTransform(ws.display, scenarios[2].src->pic, &xf);
	XRenderSetPictureFilter(ws.display, scenarios[2].src->pic, "nearest",
		NULL, 0);

	scenarios[3].name = strdup("Transformation/Bilinear filter");
	scenarios[3].src = xrender_surf_new(ws.window, PictStandardRGB24,
		128, 128);
	scenarios[3].dst = xrender_surf_new(ws.window, -1, w, h);
	if(surface_from_file(scenarios[3].src, "image2.bmp", 0)) return NULL;
	XRenderSetPictureTransform(ws.display, scenarios[3].src->pic, &xf);
	XRenderSetPictureFilter(ws.display, scenarios[3].src->pic, "bilinear",
		NULL, 0);

	return scenarios;
}

static Picture create_background_pic()
{
	Picture bg_pic;
	static XFixed stop[2] = { 0, 65536 };
	XLinearGradient linear;
	XRenderColor color[2] = {
		{ 0x0, 0x0, 0x0, 0xFFFF },
		{ 0xffff, 0xffff, 0xffff, 0xffff }
	};

	linear.p1.x = XDoubleToFixed(0.0f);
	linear.p1.y = XDoubleToFixed(0.0f);
	linear.p2.x = XDoubleToFixed(0.0f);
	linear.p2.y = XDoubleToFixed(40.0f);

	bg_pic = XRenderCreateLinearGradient(ws.display, &linear, stop, color, 2);

	XRenderPictureAttributes pa;
	pa.repeat = 1;
	XRenderChangePicture(ws.display, bg_pic, CPRepeat, &pa);

	return bg_pic;
}

static int time_test(test_scenario* test_s, render_test *op)
{
	int i, x, y, ret;
	char buf[64];

	snprintf(buf, 40, "%s.............................................",
		test_s->name);

	LOG("\t %s", buf);
	ret = timing_start();
	if(ret) return ret;
	for (i = 0; i < iterations; ++i)
	{
		x = fastrand() % (test_s->dst->w - (test_s->src->w >> 1));
		y = fastrand() % (test_s->dst->h - (test_s->src->h >> 1));
		XRenderComposite(ws.display, op->pict_op, test_s->src->pic, None,
			test_s->dst->pic, 0, 0, 0, 0, x, y, test_s->src->w, test_s->src->h);
	}
	XSync(ws.display, False);
	ret = timing_stop();
	if(ret) return ret;
	LOG(" %.3f s\n", timing_elapsed());

	return 0;
}

static int execute_test(render_test* current_op,
	test_scenario* scenarios, Picture bg_pic)
{
	int j, ret;

	for (j = 0; j < 4; ++j)
	{
		ret = time_test(&scenarios[j], current_op);
		if(ret) return ret;
		if ((j + 1) < 4)
		{
			XRenderComposite(ws.display, PictOpSrc, bg_pic, None,
				scenarios[j+1].dst->pic, 0, 0, 0, 0, 0, 0, ws.width, ws.height);
		}
		XSync(ws.display, False);
	}

	return 0;
}

static int main_loop(int num_tests, int* test_list)
{
	render_test current_op;
	Picture bg_pic;
	int i, j;
	test_scenario* scenarios;

	if(!XRenderQueryVersion(ws.display, &i, &j))
	{
		LOGERR("XRenderQueryVersion failed\n");
		return -1;
	}

	LOG("Xrender version: %d.%d\n", i, j);

	bg_pic = create_background_pic(ws.display);

	scenarios = create_test_scenarios(ws.width, ws.height);
	if(!scenarios)
	{
		return -1;
	}

	XRenderComposite(ws.display, PictOpSrc, bg_pic, None,
		scenarios[0].dst->pic, 0, 0, 0, 0, 0, 0, ws.width, ws.height);
	XFlush(ws.display);

	i = 0;
	while(1)
	{
		if(num_tests)
		{
			if(i >= num_tests)
			{
				break;
			}
			current_op = all_render_tests[test_list[i] - 1];
			LOG("Starting test %d: %s\n", test_list[i++], current_op.name);
		}
		else
		{
			current_op = all_render_tests[i++];
			if(!current_op.name)
			{
				break;
			}
			LOG("Starting test %d: %s\n", i, current_op.name);
		}

		j = execute_test(&current_op, scenarios, bg_pic);
		if(j)
		{
			LOGERR("Test failed (%d).\n", j);
		}
		else
		{
			LOG("Test passed.\n");
		}
	}
	return 0;
}

static void show_tests()
{
	int t = 0;
	while(all_render_tests[t].name)
	{
		fprintf(stdout, "%d: %s\n", t + 1, all_render_tests[t].name);
		t++;
	}
}

static void show_help()
{
	const char* usage_msg =
	{
		"USAGE: blts-xrender-bench [-a] [-l logfile] "
		"[-e test_num1,test_num2...] [-s] [-w window_width] [-h window_height]"
		"[-i num_iters] [-c]\n"
		"-l: Used logfile (default: ./xrender_bench.txt)\n"
		"-e: Execute single or multiple selected tests, for example -e 1,4,5\n"
		"-s: Show list of all tests\n"
		"-w: Used window width. If 0 uses desktop width. (default: 0)\n"
		"-h: Used window height. If 0 uses desktop height. (default: 0)\n"
		"-i: Number of iterations per case. (default: 512)\n"
		"-c: Do not output log to console\n"
	};

	fprintf(stdout, "%s\n", usage_msg);
}

static int parse_int_list(const char* list, int* out)
{
	char tmp[8];
	unsigned int  t, p, i;

	i = 0; p = 0;
	for(t = 0; t < strlen(list); t++)
	{
		if(list[t] == ',')
		{
			out[p++] = atoi(tmp);
			i = 0;
		}
		else
		{
			if(i >= 8) return 0;
			tmp[i++] = list[t];
			tmp[i] = 0;
		}
	}

	if(i) out[p++] = atoi(tmp);
	out[p] = 0;
	return p;
}

int main(int argc, char *argv[])
{
	int ret = -1;
	int t;
	char* logfile = "xrender_bench.txt";
	int width = 0;
	int height = 0;
	int num_tests = 0;
	int test_list[32];
	int console_log = 1;

	iterations = 512;
	fastrand_val = 0x12345678;

	for(t = 1; t < argc; t++)
	{
		if(!strcmp(argv[t], "-l"))
		{
			if(++t >= argc)
			{
				show_help();
				return 1;
			}
			logfile = argv[t];
		}
		else if(!strcmp(argv[t], "-w"))
		{
			if(++t >= argc)
			{
				show_help();
				return 1;
			}
			width = atoi(argv[t]);
		}
		else if(!strcmp(argv[t], "-h"))
		{
			if(++t >= argc)
			{
				show_help();
				return 1;
			}
			height = atoi(argv[t]);
		}
		else if(!strcmp(argv[t], "-i"))
		{
			if(++t >= argc)
			{
				show_help();
				return 1;
			}
			iterations = atoi(argv[t]);
		}
		else if(!strcmp(argv[t], "-c"))
		{
			console_log = 0;
		}
		else if(strcmp(argv[t], "-e") == 0)
		{
			if(++t >= argc)
			{
				show_help();
				return 1;
			}
			if(!(num_tests = parse_int_list(argv[t], test_list)))
			{
				show_help();
				return 1;
			}
		}
		else if(!strcmp(argv[t], "-s"))
		{
			show_tests();
			return 1;
		}
		else
		{
			show_help();
			return 1;
		}
	}

	log_open(logfile, console_log);

	if(create_window_ex(&ws, "Test Window", width, height))
	{
		goto cleanup;
	}

	ret = main_loop(num_tests, test_list);

cleanup:
	close_window(&ws);
	log_close();
	return ret;
}

