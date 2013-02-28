/* xvideo_test_cases.c -- XVideo core test cases

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <linux/videodev2.h>

#include <blts_cli_frontend.h>
#include <blts_dep_check.h>
#include <blts_log.h>
#include <blts_timing.h>
#include <blts-pixelformat.h>

#include "xvideo_util.h"
#include "xvideo_test_cases.h"

/** Rules for dep check */
#define DEP_RULES "/opt/tests/blts-xvideo-tests/cfg/blts-xvideo-req-files.cfg"

static const char *xv_event_names[] = {
	"XvStarted",
	"XvStopped",
	"XvBusy",
	"XvPreempted",
	"XvHardError"
};

enum
{
	XV_PUTIMAGE = 0,
	XV_SHMPUTIMAGE,
};

struct xv_image_wrapper
{
	XvImage *image;
	XShmSegmentInfo shminfo;
};

struct xv_cpu_usage {
	struct rusage usage_start;
	struct rusage usage_end;
	long unsigned int start_user_load;
	long unsigned int start_nice_load;
	long unsigned int start_sys_load;
	long unsigned int start_idle;
	long unsigned int user_load;
	long unsigned int nice_load;
	long unsigned int sys_load;
	long unsigned int idle;
	double total_load;
	double used_time;
};

int xvideo_presence_check(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(user_ptr)
	BLTS_UNUSED_PARAM(test_num)
	unsigned int ver, rev, eventb, reqb, errorb;
	Display *dpy;

	dpy = open_display();
	if (!dpy)
		return -1;

	if (XvQueryExtension(dpy, &ver, &rev, &reqb, &eventb, &errorb)
		!= Success) {
		BLTS_ERROR("X-Video extension not found\n");
		XCloseDisplay(dpy);
		return -1;
	}
	XCloseDisplay(dpy);

	BLTS_DEBUG("Checking required components...\n");
	return depcheck(DEP_RULES, 1);
}

/* does pretty much the same as xvinfo */
static int print_xv_adaptor_info(Display *dpy, XvAdaptorInfo *ainfo)
{
	unsigned int nencode;
	unsigned int n;
	XvFormat *format;
	XvImageFormatValues *formats;
	XvAttribute *attributes;
	XvEncodingInfo *encodings;
	int image_encodings = 0;
	int nattr, nimages;
	char imageName[5];

	BLTS_DEBUG("  Adaptor \"%s\"\n", ainfo->name);
	BLTS_DEBUG("    number of ports: %li\n", ainfo->num_ports);
	BLTS_DEBUG("    port base: %li\n", ainfo->base_id);
	BLTS_DEBUG("    operations supported: ");

	switch(ainfo->type & (XvInputMask | XvOutputMask))
	{
	case XvInputMask:
		if (ainfo->type & XvVideoMask)
			BLTS_DEBUG("PutVideo ");
		if (ainfo->type & XvStillMask)
			BLTS_DEBUG("PutStill ");
		if (ainfo->type & XvImageMask)
			BLTS_DEBUG("PutImage ");
		break;

	case XvOutputMask:
		if (ainfo->type & XvVideoMask)
			BLTS_DEBUG("GetVideo ");
		if (ainfo->type & XvStillMask)
			BLTS_DEBUG("GetStill ");
		break;

	default:
		BLTS_DEBUG("none ");
		break;
	}
	BLTS_DEBUG("\n");

	format = ainfo->formats;
	BLTS_DEBUG("    supported visuals:\n");
	for (n = 0; n < ainfo->num_formats; n++, format++)
		BLTS_DEBUG("      depth %i, visualID 0x%2lx\n", format->depth,
			format->visual_id);

	format = ainfo->formats;
	attributes = XvQueryPortAttributes(dpy, ainfo->base_id, &nattr);

	if (attributes && nattr) {
		BLTS_DEBUG("    number of attributes: %i\n", nattr);

		for (n = 0; n < (unsigned int)nattr; n++) {
			BLTS_DEBUG("      \"%s\" (range %i to %i)\n",
				attributes[n].name,
				attributes[n].min_value,
				attributes[n].max_value);

			if (attributes[n].flags & XvSettable)
				BLTS_DEBUG("              %s %s",
					(attributes[n].flags & XvSettable) ? "settable" : "",
					(attributes[n].flags & XvGettable) ? "gettable" : "");

			if (attributes[n].flags & XvGettable) {
				int value;
				Atom the_atom = XInternAtom(dpy, attributes[n].name, True);

				if (the_atom != None) {
					if ((Success == XvGetPortAttribute(dpy, ainfo->base_id,
						the_atom, &value)))
						BLTS_DEBUG(" (current value is %i)", value);
				}
				BLTS_DEBUG("\n");
			}

		}
		XFree(attributes);
	}
	else
	{
		BLTS_DEBUG("    no port attributes defined\n");
	}

	XvQueryEncodings(dpy, ainfo->base_id, &nencode, &encodings);
	if (!encodings || !nencode)
		return -1;

	image_encodings = 0;

	for (n = 0; n < nencode; n++)
		if (!strcmp(encodings[n].name, "XV_IMAGE"))
			image_encodings++;

	if (nencode - image_encodings) {
		BLTS_DEBUG("    number of encodings: %i\n",
			nencode - image_encodings);

		for (n = 0; n < nencode; n++) {
			if (strcmp(encodings[n].name, "XV_IMAGE")) {
				BLTS_DEBUG("      encoding ID #%li: \"%s\"\n",
					encodings[n].encoding_id, encodings[n].name);
				BLTS_DEBUG("        size: %li x %li\n",
					encodings[n].width, encodings[n].height);
				BLTS_DEBUG("        rate: %f\n",
					(float)encodings[n].rate.numerator /
					(float)encodings[n].rate.denominator);
			}
		}
	}

	if (!(image_encodings && (ainfo->type & XvImageMask))) {
		XvFreeEncodingInfo(encodings);
		return 0;
	}

	for (n = 0; n < nencode; n++) {
		if (!strcmp(encodings[n].name, "XV_IMAGE")) {
			BLTS_DEBUG("    maximum XvImage size: %li x %li\n",
				encodings[n].width, encodings[n].height);
			break;
		}
	}

	formats = XvListImageFormats(dpy, ainfo->base_id, &nimages);
	BLTS_DEBUG("    Number of image formats: %i\n", nimages);

	for (n = 0; n < (unsigned int)nimages; n++) {
		sprintf(imageName, "%c%c%c%c", formats[n].id & 0xff,
			(formats[n].id >> 8) & 0xff,
			(formats[n].id >> 16) & 0xff,
			(formats[n].id >> 24) & 0xff);
		BLTS_DEBUG("      id: 0x%x", formats[n].id);

		if (isprint(imageName[0]) && isprint(imageName[1]) &&
			isprint(imageName[2]) && isprint(imageName[3]))
			BLTS_DEBUG(" (%s)\n", imageName)
		else
			BLTS_DEBUG("\n")

		BLTS_DEBUG("        guid: ");
		BLTS_DEBUG("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%08x\n",
			(unsigned char)formats[n].guid[0],
			(unsigned char)formats[n].guid[1],
			(unsigned char)formats[n].guid[2],
			(unsigned char)formats[n].guid[3],
			(unsigned char)formats[n].guid[4],
			(unsigned char)formats[n].guid[5],
			(unsigned char)formats[n].guid[6],
			(unsigned char)formats[n].guid[7],
			(unsigned char)formats[n].guid[8],
			(unsigned char)formats[n].guid[9],
			(unsigned char)formats[n].guid[10],
			(unsigned char)formats[n].guid[11],
			(unsigned char)formats[n].guid[12],
			(unsigned char)formats[n].guid[13],
			(unsigned char)formats[n].guid[14],
			(unsigned char)formats[n].guid[15]);
		BLTS_DEBUG("        bits per pixel: %i\n",
			formats[n].bits_per_pixel);
		BLTS_DEBUG("        number of planes: %i\n",
			formats[n].num_planes);
		BLTS_DEBUG("        type: %s (%s)\n",
			(formats[n].type == XvRGB) ? "RGB" : "YUV",
			(formats[n].format == XvPacked) ? "packed" : "planar");

		if (formats[n].type == XvRGB)
		{
			BLTS_DEBUG("        depth: %i\n", formats[n].depth);

			BLTS_DEBUG("        rgb masks: 0x%x, 0x%x, 0x%x\n",
				formats[n].red_mask,
				formats[n].green_mask,
				formats[n].blue_mask);
		}
	}
	if (formats)
		XFree(formats);

	XvFreeEncodingInfo(encodings);

	return 0;
}

int xvideo_enumerate_adapters(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(user_ptr)
	BLTS_UNUSED_PARAM(test_num)
	int ret = 0;
	unsigned int ver, rev, eventb, reqb, errorb;
	unsigned int i, j, nscreens, nadaptors;
	Display *dpy;
	XvAdaptorInfo *ainfo;

	dpy = open_display();
	if (!dpy)
		return -1;

	if (XvQueryExtension(dpy, &ver, &rev, &reqb, &eventb,
		&errorb) != Success) {
		BLTS_ERROR("X-Video extension not found\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XVideo Extension version %i.%i\n", ver, rev);

	nscreens = ScreenCount(dpy);

	for (i = 0; i < nscreens; i++) {
		BLTS_DEBUG("screen #%i\n", i);
		XvQueryAdaptors(dpy, RootWindow(dpy, i), &nadaptors, &ainfo);

		if (!nadaptors || !ainfo) {
			BLTS_DEBUG(" no adaptors present\n");
			return -ENXIO;
		}

		for (j = 0; j < nadaptors; j++) {
			ret = print_xv_adaptor_info(dpy, &ainfo[j]);
			if (ret)
				goto cleanup;
		}

		XvFreeAdaptorInfo(ainfo);
		ainfo = NULL;
	}

cleanup:
	if (ainfo)
		XvFreeAdaptorInfo(ainfo);
	if (dpy)
		XCloseDisplay(dpy);

	return ret;
}


static int setup_port_attributes(Display *dpy, xvideo_data *xv)
{
	int nattr, t, n;
	XvAttribute *attributes = NULL;
	Atom the_atom;

	BLTS_TRACE("Setting port attributes\n");

	attributes = XvQueryPortAttributes(dpy, xv->port, &nattr);
	if (!attributes || !nattr) {
		BLTS_ERROR("Failed to get port attributes\n");
		return -1;
	}

	for (t = 0; t < xv->num_attrs; t++) {
		for (n = 0; n < nattr; n++) {
			if (strcmp(xv->attrs[t].name, attributes[n].name))
				continue;

			if (!(attributes[n].flags & XvSettable)) {
				BLTS_ERROR("Port attribute %s is not settable\n",
					xv->attrs[t].name);
				XFree(attributes);
				return -1;
			}

			the_atom = XInternAtom(dpy, attributes[n].name, True);
			if (XvSetPortAttribute(dpy, xv->port, the_atom,
				xv->attrs[t].value) != Success) {
				BLTS_ERROR("Failed to set port attribute %s\n",
					xv->attrs[t].name);
				XFree(attributes);
				return -1;
			}

			BLTS_TRACE("Port attribute %s is set to %d\n",
				xv->attrs[t].name, xv->attrs[t].value);
			break;
		}
		if (n == nattr) {
			BLTS_ERROR("Port attribute %s not found\n",
				xv->attrs[t].name)
			XFree(attributes);
			return -1;
		}
	}

	XFree(attributes);
	return 0;
}

static int setup_suitable_port(Display *dpy, xvideo_data *xv)
{
	unsigned int nadaptors;
	int nimages, t, ret = -1;
	char fourcc[5];
	XvAdaptorInfo *ainfo = NULL;
	XvImageFormatValues *formats = NULL;

	XvQueryAdaptors(dpy, RootWindow(dpy, xv->screen), &nadaptors, &ainfo);
	if (!nadaptors) {
		BLTS_ERROR("No adaptors for screen %d\n", xv->screen);
		goto cleanup;
	}

	if (xv->adaptor >= nadaptors) {
		BLTS_ERROR("Adaptor %u does not exist\n", xv->adaptor);
		goto cleanup;
	}

	if (!(ainfo[xv->adaptor].type & XvImageMask)) {
		BLTS_ERROR("Adaptor %u does not support PutImage operation\n",
			xv->adaptor);
		goto cleanup;
	}

	formats = XvListImageFormats(dpy, ainfo[xv->adaptor].base_id, &nimages);
	if (!nimages) {
		BLTS_ERROR("Could not get image formats for adaptor %u\n",
			xv->adaptor);
		goto cleanup;
	}

	for (t = 0; t < nimages; t++) {
		sprintf(fourcc, "%c%c%c%c", formats[t].id & 0xff,
			(formats[t].id >> 8) & 0xff,
			(formats[t].id >> 16) & 0xff,
			(formats[t].id >> 24) & 0xff);
		if (!strcmp(fourcc, xv->fourcc))
			break;
	}
	if (t >= nimages) {
		BLTS_ERROR("Adaptor %u does not support image format '%s'\n",
			xv->adaptor, xv->fourcc);
		goto cleanup;
	}

	xv->port = ainfo[xv->adaptor].base_id;
	xv->id = formats[t].id;
	xv->bpp = formats[t].bits_per_pixel;
	strcpy(xv->adaptor_name, ainfo[xv->adaptor].name);

	if (xv->num_attrs)
		ret = setup_port_attributes(dpy, xv);
	else
		ret = 0;

cleanup:
	if (ainfo)
		XvFreeAdaptorInfo(ainfo);
	if (formats)
		XFree(formats);
	return ret;
}

static int convert_image(struct xv_rgb32_image *rgb_img, XvImage *img,
	const char *fourcc)
{
	struct xv_pix_fmt *fmt;
	unsigned char *conv_img = NULL;
	int t, len, ret = 0;

	fmt = xv_get_format(fourcc);
	if (!fmt) {
		BLTS_WARNING("Pixel format %s not supported, using random pixels\n",
			fourcc);
		for(t = 0; t < img->data_size; t++)
			img->data[t] = random();
		return 0;
	}

	len = convert_buffer(rgb_img->w, rgb_img->h,
		(unsigned char *)rgb_img->buf, V4L2_PIX_FMT_RGB32,
		&conv_img, fmt->fmt);
	if (len < 0 || !conv_img) {
		BLTS_ERROR("Failed to convert test image!\n");
		ret = -1;
		goto done;
	}

	if (len != img->data_size) {
		BLTS_ERROR("Failed to convert test image! (%d, %d)\n", len,
		img->data_size);
		ret = -EFAULT;
		goto done;
	}

	memcpy(img->data, conv_img, img->data_size);

done:
	if (conv_img)
		free(conv_img);
	return ret;
}

static int generate_test_images(Display *dpy, struct xv_image_wrapper **images,
	xvideo_data *xv, int mode)
{
	int t, ret = 0;
	unsigned i;
	struct xv_rgb32_image *rgb_img;

	if (xv->test_images == XV_TI_RANDOM)
		xv->num_images = 3;
	else
		xv->num_images = 2;

	*images = calloc(xv->num_images, sizeof(struct xv_image_wrapper));
	if (!*images) {
		BLTS_LOGGED_PERROR("calloc");
		return 	-ENOMEM;
	}

	rgb_img = xv_alloc_rgb_img(xv->src_width, xv->src_height);
	if (!rgb_img) {
		free(*images);
		*images = NULL;
		return -ENOMEM;
	}

	memset(*images, 0, sizeof(struct xv_image_wrapper) * xv->num_images);

	for (i = 0; i < xv->num_images; i++) {
		if (mode == XV_SHMPUTIMAGE) {
			(*images)[i].image = (XvImage*)XvShmCreateImage(dpy, xv->port,
				xv->id, 0, xv->src_width, xv->src_height,
				&(*images)[i].shminfo);

			(*images)[i].shminfo.shmid = shmget(IPC_PRIVATE,
				(*images)[i].image->data_size, IPC_CREAT | 0777);
			(*images)[i].shminfo.shmaddr = (*images)[i].image->data =
				shmat((*images)[i].shminfo.shmid, 0, 0);
			(*images)[i].shminfo.readOnly = False;

			if (!XShmAttach(dpy, &(*images)[i].shminfo)) {
				BLTS_ERROR("XShmAttach failed!\n");
				ret = -1;
				break;
			}
		} else {
			(*images)[i].image = (XvImage*)XvCreateImage(dpy, xv->port,
				xv->id, 0, xv->src_width, xv->src_height);
			(*images)[i].image->data = malloc((*images)[i].image->data_size);
			if (!(*images)[i].image->data) {
				BLTS_ERROR("Failed to allocate memory!\n");
				ret = -1;
				break;
			}
		}

		switch (xv->test_images) {
		case XV_TI_RANDOM:
			for(t = 0; t < (*images)[i].image->data_size; t++)
				(*images)[i].image->data[t] = random();
			continue;
		case XV_TI_VH_LINES:
			xv_generate_vh_lines(rgb_img, i);
			break;
		case XV_TI_CHECKERBOARD:
			xv_generate_checkerboard(rgb_img, i, 10, 10);
			break;
		case XV_TI_MOVING_ARROW:
			xv_generate_arrow(rgb_img, 0);
			break;
		case XV_TI_FPS_CHECK:
			xv_generate_fps_blocks(rgb_img, i);
			break;
		case XV_TI_GRAYGRADIENT:
		case XV_TI_GRAYGRADIENT_SCROLL:
			xv_generate_fs_graygradient(rgb_img);
			break;
		case XV_TI_COLORGRADIENT:
		case XV_TI_COLORGRADIENT_SCROLL:
			xv_generate_fs_colorgradient(rgb_img);
			break;
		default:
			BLTS_ERROR("Unknown test image set (%d)\n", xv->test_images);
			break;
		}

		ret = convert_image(rgb_img, (*images)[i].image, xv->fourcc);
		if (ret)
			break;
	}

	xv_free_rgb_img(rgb_img);
	return ret;
}

static void free_images(Display *dpy, xvideo_data *xv,
	struct xv_image_wrapper *images)
{
	unsigned i;

	if (!images)
		return;

	for (i = 0; i < xv->num_images; i++) {
		if (images[i].shminfo.shmaddr)
			XShmDetach(dpy, &images[i].shminfo);
		else
			if (images[i].image && images[i].image->data)
				free(images[i].image->data);

		if (images[i].image)
			XFree(images[i].image);
	}
}

static int start_cpu_usage_measurement(struct xv_cpu_usage *cpu)
{
	FILE *fp;

	fp = fopen("/proc/stat", "r");
	if (fp) {
		char tmp[128];
		if (fscanf(fp, "%s %lu %lu %lu %lu", tmp, &cpu->start_user_load,
			&cpu->start_nice_load, &cpu->start_sys_load,
			&cpu->start_idle) != 5)
			BLTS_ERROR("Failed to read /proc/stat\n");
		fclose(fp);
	}

	getrusage(RUSAGE_SELF, &cpu->usage_start);

	return 0;
}

static int stop_cpu_usage_measurement(struct xv_cpu_usage *cpu)
{
	FILE *fp;

	getrusage(RUSAGE_SELF, &cpu->usage_end);
	cpu->used_time = cpu->usage_end.ru_utime.tv_sec;
	cpu->used_time += cpu->usage_end.ru_utime.tv_usec * 1E-6;
	cpu->used_time += cpu->usage_end.ru_stime.tv_sec;
	cpu->used_time += cpu->usage_end.ru_stime.tv_usec * 1E-6;
	cpu->used_time -= cpu->usage_start.ru_utime.tv_sec;
	cpu->used_time -= cpu->usage_start.ru_utime.tv_usec * 1E-6;
	cpu->used_time -= cpu->usage_start.ru_stime.tv_sec;
	cpu->used_time -= cpu->usage_start.ru_stime.tv_usec * 1E-6;

	cpu->total_load = 0.0f;

	fp = fopen("/proc/stat", "r");
	if (fp) {
		char tmp[128];
		if (fscanf(fp, "%s %lu %lu %lu %lu", tmp, &cpu->user_load,
			&cpu->nice_load, &cpu->sys_load, &cpu->idle) == 5)
		{
			cpu->user_load -= cpu->start_user_load;
			cpu->nice_load -= cpu->start_nice_load;
			cpu->sys_load -= cpu->start_sys_load;
			cpu->idle -= cpu->start_idle;
			cpu->total_load = (double)(cpu->user_load + cpu->nice_load +
				cpu->sys_load);
			cpu->total_load =  cpu->total_load *
				100.0f / (cpu->total_load + cpu->idle);
		} else
			BLTS_ERROR("Failed to read /proc/stat\n");

		fclose(fp);
	}

	return 0;
}

static int xvideo_benchmark(void *user_ptr, int test_num, int mode)
{
	BLTS_UNUSED_PARAM(test_num)
	int ret = 0;
	unsigned cur_image;
	long frames = 0;
	struct xv_image_wrapper *images = NULL;
	struct window_struct params;
	xvideo_data *xv = (xvideo_data *)user_ptr;
	double fps;
	double cpu_usage;
	struct xv_cpu_usage cpu_u;
	int t, offset = 0;
	uint8_t *src, *dst;
	struct xv_pix_fmt *fmt;

	if (!xv)
		return -EINVAL;

	if (xv_create_window(&params, "XVideo Test", xv->dst_width,
		xv->dst_height, xv->screen)) {
		BLTS_ERROR("Failed to create window\n");
		return -1;
	}

	ret = setup_suitable_port(params.display, xv);
	if (ret)
		goto cleanup;

	fmt = xv_get_format(xv->fourcc);
	if (!fmt) {
		BLTS_ERROR("Format '%s' not supported\n", xv->fourcc);
		ret = -1;
		goto cleanup;
	}

	ret = generate_test_images(params.display, &images, xv, mode);
	if (ret)
		goto cleanup;

	ret = XvGrabPort(params.display, xv->port, CurrentTime);
	if (ret != Success) {
		BLTS_ERROR("Failed to grab port (%d)\n", ret);
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("\n");
	BLTS_DEBUG("===================================\n");
	BLTS_DEBUG("Input parameters\n");
	BLTS_DEBUG("Used adaptor: %s\n", xv->adaptor_name);
	BLTS_DEBUG("Format: %s\n", xv->fourcc);
	BLTS_DEBUG("Image size: %d x %d x %d\n",
		xv->src_width, xv->src_height, xv->bpp);

	for (t = 0; t < images[0].image->num_planes; t++) {
		BLTS_DEBUG("Plane %d\n", t);
		BLTS_DEBUG(" Pitch: %d\n", images[0].image->pitches[t]);
		BLTS_DEBUG(" Offset: %d\n", images[0].image->offsets[t]);
	}

	BLTS_DEBUG("\n");
	BLTS_DEBUG("Output parameters\n");
	BLTS_DEBUG("Image size: %d x %d x %d\n", params.width, params.height,
		params.root_window_attributes.depth);

	XSync(params.display, True);

	start_cpu_usage_measurement(&cpu_u);
	timing_start();

	cur_image = 0;
	while (timing_elapsed() < xv->duration) {
		if (xv->test_images == XV_TI_MOVING_ARROW ||
			xv->test_images == XV_TI_GRAYGRADIENT_SCROLL ||
			xv->test_images == XV_TI_COLORGRADIENT_SCROLL) {

			src = (uint8_t *)images[1].image->data;
			dst = (uint8_t *)images[0].image->data;

			if (images[0].image->num_planes > 1) {
				int plane_size;

				for (t = 0; t < images[0].image->num_planes; t++) {
					if (t == images[0].image->num_planes - 1)
						plane_size = images[0].image->data_size -
						images[0].image->offsets[t];
					else
						plane_size = images[0].image->offsets[t + 1] -
							images[0].image->offsets[t];

					xv_scroll_buf(&src[images[1].image->offsets[t]],
						&dst[images[0].image->offsets[t]], plane_size,
						images[0].image->pitches[t], offset * plane_size /
						images[0].image->height / images[0].image->pitches[t]);
				}
				if (++offset >= images[0].image->width)
					offset = 0;
			} else {
				xv_scroll_buf(src, dst,
					images[0].image->data_size,
					images[0].image->pitches[0], offset);

				offset += fmt->mpix_bpp / 8;
				if (offset >= images[0].image->pitches[0])
					offset = 0;
			}
		} else {
			if (++cur_image >= xv->num_images)
				cur_image = 0;
		}

		if (mode == XV_SHMPUTIMAGE) {
			ret = XvShmPutImage(params.display, xv->port, params.window,
					params.gc, images[cur_image].image,
					0, 0, images[cur_image].image->width,
					images[cur_image].image->height, 0, 0, params.width,
						params.height, False);
			if (ret != Success) {
				BLTS_ERROR("XvShmPutImage failed (%d)\n", ret);
				ret = -1;
				break;
			}
		} else {
			ret = XvPutImage(params.display, xv->port, params.window,
				params.gc, images[cur_image].image,
				0, 0, images[cur_image].image->width,
				images[cur_image].image->height, 0, 0, params.width,
					params.height);
			if (ret != Success) {
				BLTS_ERROR("XvPutImage failed (%d)\n", ret);
				ret = -1;
				break;
			}
		}
		XFlush(params.display);
		frames++;
	}
	XSync(params.display, True);

	timing_stop();
	stop_cpu_usage_measurement(&cpu_u);

	fps = frames / timing_elapsed();
	cpu_usage = 100.0 * cpu_u.used_time / timing_elapsed();

	BLTS_DEBUG("\n");
	BLTS_DEBUG("Results\n");
	BLTS_DEBUG("Time elapsed: %f\n", timing_elapsed());
	BLTS_DEBUG("Frames: %d\n", frames);
	BLTS_DEBUG("FPS: %f\n", frames / timing_elapsed());
	BLTS_DEBUG("Input bytes per second: %f\n",
		xv->src_width * xv->src_height * xv->bpp * fps / 8);
	BLTS_DEBUG("Output bytes per second: %f\n",
		params.width * params.height *
		params.root_window_attributes.depth * fps / 8);
	BLTS_DEBUG("Output pixels per second: %f\n",
		params.width * params.height * fps);
	BLTS_DEBUG("CPU usage (this process): %lf %%\n", cpu_usage);
	if (cpu_u.total_load > 0.0f) {
		BLTS_DEBUG("CPU usage (all processes, all CPUs): %lf %%\n",
			cpu_u.total_load)
	} else {
		BLTS_DEBUG("CPU usage (all processes, all CPUs): N/A\n");
	}

	BLTS_DEBUG("===================================\n\n");

	ret = XvUngrabPort(params.display, xv->port, CurrentTime);
	if (ret != Success) {
		BLTS_ERROR("Failed to ungrab port (%d)\n", ret);
		ret = -1;
	}

cleanup:
	free_images(params.display, xv, images);
	free(images);
	xv_close_window(&params);

	return ret;
}

int xvideo_putimage(void *user_ptr, int test_num)
{
	return xvideo_benchmark(user_ptr, test_num, XV_PUTIMAGE);
}

int xvideo_shmputimage(void *user_ptr, int test_num)
{
	return xvideo_benchmark(user_ptr, test_num, XV_SHMPUTIMAGE);
}

int xvideo_querybestsize(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int ret = 0;
	unsigned cur_image;
	unsigned width, height;
	struct xv_image_wrapper *images = NULL;
	struct window_struct params;
	xvideo_data *xv = (xvideo_data *)user_ptr;
	unsigned int ver, rev, eventb, reqb, errorb;

	if (!xv)
		return -EINVAL;

	if (xv_create_window(&params, "XVideo Test", 0, 0, xv->screen)) {
		BLTS_ERROR("Failed to create window\n");
		return -1;
	}

	if (XvQueryExtension(params.display, &ver, &rev, &reqb, &eventb, &errorb)
		!= Success) {
		BLTS_ERROR("X-Video extension not found\n");
		ret = -1;
		goto cleanup;
	}

	ret = setup_suitable_port(params.display, xv);
	if (ret)
		goto cleanup;

	ret = generate_test_images(params.display, &images, xv, 0);
	if (ret)
		goto cleanup;

	ret = XvGrabPort(params.display, xv->port, CurrentTime);
	if (ret != Success) {
		BLTS_ERROR("Failed to grab port (%d)\n", ret);
		ret = -1;
		goto cleanup;
	}

	/* find out best output image size */
	ret = XvQueryBestSize(params.display, xv->port, True,
		xv->src_width, xv->src_height,
		params.width, params.height,
		&width, &height);
	if (ret != Success) {
		BLTS_ERROR("XvQueryBestSize failed with %d\n", ret);
		ret = -1;
		goto cleanup;
	}

	/* check for invalid size */
	if (!width || !height || width > params.width ||
		height > params.height) {
		BLTS_ERROR("XvQueryBestSize returned invalid size %ux%u\n",
			width, height);
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("\n");
	BLTS_DEBUG("===================================\n");
	BLTS_DEBUG("Input parameters\n");
	BLTS_DEBUG("Used adaptor: %s\n", xv->adaptor_name);
	BLTS_DEBUG("Format: %s\n", xv->fourcc);
	BLTS_DEBUG("Image size: %d x %d x %d\n",
		xv->src_width, xv->src_height, xv->bpp);

	BLTS_DEBUG("\n");
	BLTS_DEBUG("Output parameters\n");
	BLTS_DEBUG("Image size: %d x %d x %d\n", width, height,
		params.root_window_attributes.depth);

	XSync(params.display, True);

	timing_start();
	/* test streaming with the 'best size' image */
	cur_image = 0;
	while (timing_elapsed() < 5) {
		if (++cur_image >= xv->num_images)
			cur_image = 0;

		ret = XvPutImage(params.display, xv->port, params.window, params.gc,
			images[cur_image].image,
			0, 0, images[cur_image].image->width,
			images[cur_image].image->height, 0, 0, width, height);
		if (ret != Success) {
			BLTS_ERROR("XvPutImage failed (%d)\n", ret);
			ret = -1;
			break;
		}

		XFlush(params.display);
	}
	XSync(params.display, True);

	timing_stop();

	ret = XvUngrabPort(params.display, xv->port, CurrentTime);
	if (ret != Success) {
		BLTS_ERROR("Failed to ungrab port (%d)\n", ret);
		ret = -1;
	}

	BLTS_DEBUG("===================================\n\n");

cleanup:
	free_images(params.display, xv, images);
	free(images);
	xv_close_window(&params);

	return ret;
}

static int xvideo_notify_event_process(xvideo_data *xv)
{
	Display *dpy;
	xvideo_data *xv2;
	int ret;

	dpy = open_display();
	if (!dpy)
		return -1;

	xv2 = malloc(sizeof(*xv2));
	if (!xv2)
		goto cleanup;

	memcpy(xv2, xv, sizeof(*xv2));

	ret = setup_suitable_port(dpy, xv2);
	if (ret)
		goto cleanup;

	BLTS_DEBUG("Set attribute '%s' = %u\n", xv2->attrs[0].name,
		xv2->attrs[0].value);
	if (XvSetPortAttribute(dpy, xv2->port,
		XInternAtom(dpy, xv2->attrs[0].name, True),
		xv2->attrs[0].value) != Success) {
		BLTS_ERROR("Failed  to set port attribute %s\n",
			xv2->attrs[0].name);
		ret = -1;
	}

cleanup:
	XCloseDisplay(dpy);

	if (xv2)
		free(xv2);

	return ret;
}

int xvideo_notify_events_test(void* user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int ret = 0;
	struct window_struct params;
	xvideo_data *xv = (xvideo_data *)user_ptr;
	unsigned int ver, rev, eventb, reqb, errorb;
	int waiting, events_requested = 0;
	pid_t pid;

	if (!xv)
		return -EINVAL;

	if (xv_create_window(&params, "XVideo Test", 0, 0, xv->screen)) {
		BLTS_ERROR("Failed to create window\n");
		return -1;
	}

	if (XvQueryExtension(params.display, &ver, &rev, &reqb, &eventb, &errorb)
		!= Success) {
		BLTS_ERROR("X-Video extension not found\n");
		ret = -1;
		goto cleanup;
	}

	ret = setup_suitable_port(params.display, xv);
	if (ret)
		goto cleanup;

	/* receive events */
	ret = XvSelectPortNotify(params.display, xv->port, True);
	if (ret != Success) {
		BLTS_ERROR("XvSelectPortNotify failed with %d\n", ret);
		ret = -1;
		goto cleanup;
	}
	ret = XvSelectVideoNotify(params.display, params.window, True);
	if (ret != Success) {
		BLTS_ERROR("XvSelectVideoNotify failed with %d\n", ret);
		ret = -1;
		goto cleanup;
	}
	events_requested = 1;

	XSync(params.display, True);

	timing_start();

	/* set port attribute, expect XvPortNotifyEvent */
	if ((pid = fork()) < 0) {
		BLTS_ERROR("Failed to fork\n");
		ret = -1;
		goto cleanup;
	}
	if (!pid) {
		sleep(1);
		exit(xvideo_notify_event_process(xv));
	}

	BLTS_DEBUG("Waiting for event...\n");
	waiting = 1;
	while (timing_elapsed() < 5 && waiting) {
		XEvent event;
		XNextEvent(params.display, &event);
		switch(event.type - eventb) {
			case XvVideoNotify:
			{
				XvEvent *xve = (XvEvent *)&event;
				BLTS_DEBUG("XvVideoNotify, reason=%s\n",
					xv_event_names[xve->xvvideo.reason]);
				/* Should not receive any video notify -events */
				waiting = 0;
				ret = -1;
				break;
			}
			case XvPortNotify:
			{
				XvEvent *xve = (XvEvent *)&event;
				BLTS_DEBUG("XvPortNotify: %s=%ld\n",
					XGetAtomName(params.display, xve->xvport.attribute),
					xve->xvport.value);

				if (!strcmp(XGetAtomName(params.display,
					xve->xvport.attribute), xv->attrs[0].name)) {
					BLTS_DEBUG("Got correct event\n");
					waiting = 0;
				}
				break;
			}
		}
	}
	waitpid(pid, &ret, 0);
	if (waiting) {
		BLTS_DEBUG("Timeout while waiting event\n");
		ret = -1;
		goto cleanup;
	}
	timing_stop();

cleanup:
	if (events_requested) {
		/* no more events */
		ret = XvSelectPortNotify(params.display, xv->port, False);
		if (ret != Success) {
			BLTS_ERROR("XvSelectPortNotify failed with %d\n", ret);
			ret = -1;
		}
		ret = XvSelectVideoNotify(params.display, params.window, False);
		if (ret != Success) {
			BLTS_ERROR("XvSelectVideoNotify failed with %d\n", ret);
			ret = -1;
		}
	}

	xv_close_window(&params);

	return ret;
}

