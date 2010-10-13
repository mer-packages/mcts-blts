/* camera.c -- V4L2 commands

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

   based on freerly distributed V4L2 example

*/

#define _GNU_SOURCE
#include "v4l2-xvideo.h"
#include "blts-bayer.h"
#include "camera.h"
#include "v4l2-ioctl.h"
#include "blts-v4l2-definitions.h"
#include <stdio.h>
#include <blts_log.h>
#include <blts_timing.h>
#include <blts-pixelformat.h>
#include <poll.h>
#include <math.h>
#include <float.h>

#define CAPABILITY(from, what) { \
		if(from & what) { \
			 LOG("\tCapability %s is available\n", #what); \
		}\
	}

#define STATUS(from, what) { \
		if(from & what) { \
			 BLTS_DEBUG("%s is TRUE\n", #what); \
		} \
	}

void process_image(v4l2_dev_data *device, void *p);

/**
 * greatest common divisor, used with aspect ratio
 *	http://en.wikipedia.org/wiki/Euclidean_algorithm
 */

int gcd(int a, int b)
{
	if( b == 0 )
		return a;
	else
		return gcd(b, a % b);
}

/**
 * calculate and return aspect ratio of width vs height as string
 */

char * get_aspect_ratio(int w, int h)
{
	FUNC_ENTER()
	char *ret;
	float divisor;
	divisor = gcd(w, h);
	float upper = (float)w / (float)divisor;
	float lower = (float)h / (float)divisor;
	if(lower > 1)
	{
		upper = upper / lower;
		lower = lower / lower;
	}
	if(-1 == asprintf(&ret, "%.2f:%.2f", upper, lower))
	{
		FUNC_LEAVE()
		return NULL;
	}
	FUNC_LEAVE()
	return ret;
}


/**
 * calculate and print aspect ratio of width vs height
 */

void print_aspect_ratio(int w, int h)
{
	FUNC_ENTER()
	char * aspect_ratio;
	if(NULL!= (aspect_ratio = get_aspect_ratio(w,h)))
	{
		LOG("%s,", aspect_ratio);
		LOG("aspect ratio:%s\n", aspect_ratio);
		free(aspect_ratio);
	}
	else
	{
		LOG("%s,", "N/A");
		LOG("aspect ratio not available\n");
	}
	FUNC_LEAVE()
}

/**
 * helper function during error
 */

void print_errno(const char * s)
{
	FUNC_ENTER()
	LOGERR ("%s error %d, %s\n",
		s, errno, strerror (errno));
	FUNC_LEAVE()
}

/**
 * helper function during error
 */

void errno_continue(const char * s)
{
	FUNC_ENTER()
	LOGERR ("%s error %d, %s\n",
		s, errno, strerror (errno));
	FUNC_LEAVE()
}


/**
 * check and print camera capabilities
 */

int query_capabilites(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	struct v4l2_capability capabilities;

	memset (&capabilities, 0, sizeof (capabilities));

	if (0 == ioctl_VIDIOC_QUERYCAP (dev->dev_fd, &capabilities, 0))
	{
		LOG("Driver capabilities:\n");
		LOG("\t.driver = %s\n", capabilities.driver);
		LOG("\t.card = %s\n", capabilities.card);
		LOG("\t.bus_info = %s\n", capabilities.bus_info);
		LOG("\t.version = %u.%u.%u\n",
			(capabilities.version >> 16) & 0xFF,
			(capabilities.version >> 8) & 0xFF,
			capabilities.version & 0xFF);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_VIDEO_CAPTURE);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_VIDEO_OUTPUT);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_VIDEO_OVERLAY);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_VBI_CAPTURE);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_VBI_OUTPUT);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_RDS_CAPTURE);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_TUNER);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_AUDIO);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_READWRITE);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_ASYNCIO);
		CAPABILITY(capabilities.capabilities, V4L2_CAP_STREAMING);
	}
	else
	{
		errno_continue("query_capabilities(): capability query not functional\n");
		FUNC_LEAVE()
		return FALSE;
	}
	FUNC_LEAVE()
	return TRUE;
}

/**
 * helper function for controls
 * enumerates "control menu" -item
 */

int enumerate_menu (v4l2_dev_data *dev, struct v4l2_queryctrl queryctrl)
{
	FUNC_ENTER()
	struct v4l2_querymenu querymenu;

	memset (&querymenu, 0, sizeof (querymenu));
	querymenu.id = queryctrl.id;

	for (querymenu.index = queryctrl.minimum;
			querymenu.index <= queryctrl.maximum;
			querymenu.index++)
	{
		if (0 == ioctl_VIDIOC_QUERYMENU (dev->dev_fd, &querymenu, 0))
		{
			LOG("\t%s\n", querymenu.name);
		}
		else
		{
			errno_continue("VIDIOC_QUERYMENU failure, should not happed\n");
			FUNC_LEAVE()
			return FALSE;
		}
	}
	FUNC_LEAVE()
	return TRUE;
}

/**
 * Enumerate through camera controls
 */

int enum_controls(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	struct v4l2_queryctrl queryctrl;

	memset (&queryctrl, 0, sizeof (queryctrl));

	for (queryctrl.id = V4L2_CID_BASE;
			queryctrl.id < V4L2_CID_LASTP1;
			queryctrl.id++)
	{
		if (0 == ioctl_VIDIOC_QUERYCTRL(dev->dev_fd, &queryctrl, 0))
		{
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;

			LOG("Control %s\n", queryctrl.name);

			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerate_menu (dev, queryctrl);
		}
		else
		{
			if (errno == EINVAL)
				continue;
			/* we should never reach this, or driver is failing */
			errno_continue("enum_controls(): ioctl:VIDIOC_QUERYCTRL failure\n");
			FUNC_LEAVE()
			return FALSE;
		}
	}
	/* private base enumeration */
	for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
			queryctrl.id++)
	{
		if (0 == ioctl_VIDIOC_QUERYCTRL(dev->dev_fd, &queryctrl, 0))
		{
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;
			LOG("Control %s\n", queryctrl.name);

			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerate_menu (dev, queryctrl);
		}
		else
		{
			if (errno == EINVAL)
				break;

			errno_continue("VIDIOC_QUERYCTRL failure\n");
			FUNC_LEAVE()
			return FALSE;
		}
	}
	/* extended controls */

	memset (&queryctrl, 0, sizeof (queryctrl));
	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl_VIDIOC_QUERYCTRL(dev->dev_fd, &queryctrl, 0))
	{
		if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
			continue;
		LOG("Extended control %s\n", queryctrl.name);
		if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
			enumerate_menu (dev, queryctrl);
		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	if (errno != EINVAL)
	{
		/* we should never reach this, or driver is failing */
		errno_continue("enum_controls(): ioctl:VIDIOC_QUERYCTRL failure\n");
		FUNC_LEAVE()
		return FALSE;
	}
	FUNC_LEAVE()
	return TRUE;
}


/**
 * Check camera inputs and print them
 */

int enum_inputs(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	int id;
	struct v4l2_input argp;

	memset (&argp, 0, sizeof (argp));

	for ( id = 0;;id++)
	{
		argp.index = id;
		if (0 == ioctl_VIDIOC_ENUMINPUT(dev->dev_fd, &argp, 0))
		{
			BLTS_DEBUG("type: %s\n", argp.name);
			STATUS(argp.type,V4L2_INPUT_TYPE_TUNER);
			STATUS(argp.type,V4L2_INPUT_TYPE_CAMERA);

			STATUS(argp.status, V4L2_IN_ST_NO_POWER);
			STATUS(argp.status, V4L2_IN_ST_NO_SIGNAL);
			STATUS(argp.status, V4L2_IN_ST_NO_COLOR);

			STATUS(argp.status, V4L2_IN_ST_NO_H_LOCK);
			STATUS(argp.status, V4L2_IN_ST_COLOR_KILL);

			STATUS(argp.status, V4L2_IN_ST_NO_SYNC);
			STATUS(argp.status, V4L2_IN_ST_NO_EQU);
			STATUS(argp.status, V4L2_IN_ST_NO_CARRIER);

			STATUS(argp.status, V4L2_IN_ST_MACROVISION);
			STATUS(argp.status, V4L2_IN_ST_NO_ACCESS);
			STATUS(argp.status, V4L2_IN_ST_VTR);
		}
		else
		{
			if (errno == EINVAL)
				break;

			errno_continue("VIDIOC_QUERYCTRL failure\n");
			FUNC_LEAVE()
			return FALSE;
		}
	}
	FUNC_LEAVE()
	return TRUE;
}

/**
 * enumerate and print through camera formats
 */

int enum_formats(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	int id,buftype;
	int cmpr;
	struct v4l2_fmtdesc argp;

	memset (&argp, 0, sizeof (argp));

	/* go through all buffer types (V4L2) */
	LOG("Supported formats:\n");
	for ( buftype = 1; buftype < 4; buftype++)
	{
		/* go through all formats (V4L2) */
		for ( id = 0;;id++)
		{
			cmpr=0;
			argp.index = id;
			argp.type = buftype;
			if (0 == ioctl_VIDIOC_ENUM_FMT(dev->dev_fd, &argp, 0))
			{
				switch (argp.type)
				{
				case(1):

					if (argp.flags & V4L2_FMT_FLAG_COMPRESSED)
					{
						cmpr=1;
					}
					LOG("V4L2_BUF_TYPE_VIDEO_CAPTURE;%s;%c%c%c%c;%i\n",
						argp.description,
						(argp.pixelformat >> 0) & 0xFF,
						(argp.pixelformat >> 8) & 0xFF,
						(argp.pixelformat >> 16) & 0xFF,
						(argp.pixelformat >> 24) & 0xFF,
						cmpr
						);

					if ( strlen((char*)argp.description) == 0 )
					{
							BLTS_DEBUG("\n");
					}

					break;
				case(2):
					if (argp.flags & V4L2_FMT_FLAG_COMPRESSED)
					{
						LOG("buffer type V4L2_BUF_TYPE_VIDEO_OUTPUT: this format is compressed\n");
						cmpr=1;
					}
					LOG("V4L2_BUF_TYPE_VIDEO_OUTPUT;%s;%c%c%c%c;%i\n",
						argp.description,
						(argp.pixelformat >> 0) & 0xFF,
						(argp.pixelformat >> 8) & 0xFF,
						(argp.pixelformat >> 16) & 0xFF,
						(argp.pixelformat >> 24) & 0xFF,
						cmpr
						);

					break;

				case(3):
					if (argp.flags & V4L2_FMT_FLAG_COMPRESSED)
					{
						LOG("buffer type V4L2_BUF_TYPE_VIDEO_OVERLAY: this format is compressed\n");
						cmpr=1;
					}

				BLTS_DEBUG("V4L2_BUF_TYPE_VIDEO_OVERLAY;%s;%c%c%c%c;%i\n",
						argp.description,
						(argp.pixelformat >> 0) & 0xFF,
						(argp.pixelformat >> 8) & 0xFF,
						(argp.pixelformat >> 16) & 0xFF,
						(argp.pixelformat >> 24) & 0xFF,
						cmpr
						);
					break;
				default:
					if (argp.flags & V4L2_FMT_FLAG_COMPRESSED)
					{
						cmpr=1;
					}

					LOG("buffer enum:%i;%s;%c%c%c%c;%i\n",
						argp.index,
						argp.description,
						(argp.pixelformat >> 0) & 0xFF,
						(argp.pixelformat >> 8) & 0xFF,
						(argp.pixelformat >> 16) & 0xFF,
						(argp.pixelformat >> 24) & 0xFF,
						cmpr
						);
				}
			}
			else
			{
				if (errno == EINVAL)
				{
					break;
				}
				/* we should never reach this, or driver is failing */
				errno_continue("enum_formats(): ioctl:VIDIOC_ENUM_FMT failure\n");
				FUNC_LEAVE()
				return FALSE;
			}
		}
	}
	FUNC_LEAVE()
	return TRUE;
}

static char* fourcc_to_string(int code)
{
	char *buf = malloc(5);
	buf[0] = code & 0xff;
	buf[1] = (code & 0xff00) >> 8;
	buf[2] = (code & 0xff0000) >> 16;
	buf[3] = (code & 0xff000000) >> 24;
	buf[4] = 0;
	return buf;
}

int try_format(v4l2_dev_data *dev, const int id)
{
	char *c_req, *c_actual;
	dev->format.fmt.pix.pixelformat = id;

	if (ioctl_VIDIOC_TRY_FMT(dev->dev_fd, &dev->format, 0) < 0 )
	{
		print_errno("VIDIOC_TRY_FMT");
		return FALSE;
	}

	if (ioctl_VIDIOC_S_FMT(dev->dev_fd, &dev->format, 0) < 0 )
	{
		print_errno("VIDIOC_S_FMT");
		return FALSE;
	}
	if (dev->format.fmt.pix.pixelformat != id) {
		c_req = fourcc_to_string(id);
		c_actual = fourcc_to_string(dev->format.fmt.pix.pixelformat);
		BLTS_ERROR("Error: Pixel format change not expected: wanted '%s', got '%s'\n",
			c_req, c_actual);
		free(c_req);
		free(c_actual);
		return FALSE;
	}
	return TRUE;
}


int do_poll_in_wait(int fd, int timeout)
{
	int ret;
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	ret = poll(&pfd, 1, timeout);
	if (ret > 0) {
		if (pfd.revents & POLLERR) {
			BLTS_ERROR("poll() failed (POLLERR is set)\n");
			return -1;
		}
		return 0;
	}
	if (!ret) {
		BLTS_ERROR("poll() timed out\n");
		return -ETIMEDOUT;
	}

	BLTS_ERROR("Error in poll() - %s\n", errno ? strerror(errno): "unknown error");
	return errno?-errno:-1;
}



/**
 * process image and stream if use_streaming flag is set
 */
void process_image(v4l2_dev_data *device, void *p)
{
	FUNC_ENTER();
	assert(device);
	/* dot printing removed */
	if (device->use_streaming)
	{
		if (device->format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
		{
			// YUYV can be displayed directly without conversion
			xvideo_process_image(p);
		}
		else
		{
			// Convert to YUYV before displaying
			unsigned char* yuyvbuf;
			int w = device->format.fmt.pix.width;
			int h = device->format.fmt.pix.height;
			__u32 fmt = device->format.fmt.pix.pixelformat;
			int res = convert_buffer(w, h, p, fmt, &yuyvbuf, V4L2_PIX_FMT_YUYV);
			if (res > 0)
			{
				xvideo_process_image(yuyvbuf);
				free(yuyvbuf);
			}
		}
	}
	FUNC_LEAVE();
}

/**
 * write frame data straigth to JPEG file
 */
void process_native_jpeg_file(v4l2_dev_data *dev, const void * p, char *filename)
{

	FILE *outfile;
	__u16 *word_to_write = (__u16*)p;
	unsigned int data_counter = 0;

	if ((outfile = fopen(filename, "wb")) == NULL) {
		BLTS_ERROR("can't open '%s' for writing \n", filename);
		return;
	}

	//write data until End Of File (JPEG) is encountered
	do
	{
		fwrite(word_to_write, sizeof(__u16), 1, outfile);
		word_to_write++;
		data_counter++;
		if(data_counter > dev->format.fmt.pix.width*dev->format.fmt.pix.height)
		{
			LOGERR("JPEG image in buffer probably corrupted. Check image consistency.\n");
			LOGERR("JPEG image in buffer probably corrupted. Check image consistency.\n");
			LOGERR("\tData counter %i \n", data_counter);
			LOGERR("\tWas more than %i \n", dev->format.fmt.pix.width*dev->format.fmt.pix.height);
			break;
		}
	} while((*word_to_write) != 0xFF9D);
	// write EOI to file
	fwrite(word_to_write, sizeof(__u16), 1, outfile);
	fclose(outfile);
	LOG("Created file %s\n", filename);
}

/**
 * Process image to JPG file
 */
void process_image_file(v4l2_dev_data *dev, const void * p, char *filename)
{
	int w = dev->format.fmt.pix.width;
	int h = dev->format.fmt.pix.height;

	if(dev->format.fmt.pix.pixelformat == v4l2_fourcc('J','P','E','G'))
	{
	    process_native_jpeg_file(dev, p, filename);
	    return;
	}


	FILE * outfile;
	int row_stride = w * 3;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	unsigned char* rgbbuffer;
	int res = convert_buffer(w, h, p, dev->format.fmt.pix.pixelformat, &rgbbuffer, V4L2_PIX_FMT_RGB24);
	if (res < 0)
	{
		LOGERR("Failed to convert buffer, result %d\n", res);
		return;
	}

	JSAMPROW scanline[1];

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		LOGERR("can't open '%s' for writing\n", filename);
		free(rgbbuffer);
		return;
	}

	jpeg_stdio_dest(&cinfo, outfile);


	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE);		// maximum quality

	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height)
	{
		scanline[0] = &rgbbuffer[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, scanline, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);
	LOG("Created file %s\n", filename);
	free(rgbbuffer);
}

/** check wheter the image is readable or not
 * return 0 on success
 */
int read_jpeg_image(char *filename)
{

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];

	FILE *infile = fopen( filename, "rb" );

	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", filename );
		return -1;
	}
	cinfo.err = jpeg_std_error( &jerr );

	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, infile );
	jpeg_read_header( &cinfo, TRUE );

	LOG("JPEG File Information \n");
	LOG("\tImage width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height);
	LOG("\tColor components per pixel: %d.\n", cinfo.num_components);
	LOG("\tColor space: %d.\n", cinfo.jpeg_color_space);

	jpeg_start_decompress( &cinfo );
	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
	}
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	free( row_pointer[0] );
	fclose( infile );
	return 0;
}


void dump_buffer_info(unsigned loglevel, v4l2_dev_data *dev, struct v4l2_buffer *buf)
{
	int i;
	assert(dev);
	assert(buf);
	log_print_level(loglevel, "Buffer state:\n");
	log_print_level(loglevel, "  Returned buffer:\n");
	log_print_level(loglevel, "\tindex=%u type=%u bytesused=%u flags=%u field=%u\n",
		buf->index, buf->type, buf->bytesused, buf->flags, buf->field);
	log_print_level(loglevel, "\ttimestamp=%u:%u timecode=%u:%u:%u:%u:%u:%u:%u:%u:%u:%u sequence=%u\n",
		buf->timestamp.tv_sec, buf->timestamp.tv_usec,
		buf->timecode.type, buf->timecode.flags, buf->timecode.frames,
		buf->timecode.seconds, buf->timecode.minutes, buf->timecode.hours,
		buf->timecode.userbits[0], buf->timecode.userbits[1],
		buf->timecode.userbits[2], buf->timecode.userbits[3], buf->sequence);
	switch (buf->memory) {
	case V4L2_MEMORY_MMAP:
		log_print_level(loglevel, "\tmemory=MMAP offset=%u\n", buf->m.offset);
		break;
	case V4L2_MEMORY_USERPTR:
		log_print_level(loglevel, "\tmemory=USERPTR userptr=%p\n",(void*) buf->m.userptr);
		break;
	case V4L2_MEMORY_OVERLAY:
		log_print_level(loglevel, "\tmemory=OVERLAY\n");
		break;
	}
	log_print_level(loglevel, "\tlength=%u input=%u reserved=%u\n",buf->length, buf->input, buf->reserved);
	log_print_level(loglevel, "  Our buffers:\n");
	for (i = 0; i < dev->n_buffers; ++i)
		log_print_level(loglevel, "\t%p (length %u)\n",
			dev->buffers[i].start, dev->buffers[i].length);
	log_print_level(loglevel, "\n");
}

static int validate_dequeued_buffer(v4l2_dev_data *dev, struct v4l2_buffer *buf)
{
	int i = 0;

	assert(dev);
	assert(buf);

	if (dev->io_method == IO_METHOD_USERPTR) {
		for (i = 0; i < dev->n_buffers; ++i)
			if (buf->m.userptr == (unsigned long) dev->buffers[i].start)
				break;
		if (i >= dev->n_buffers) {
			BLTS_ERROR("Error: Driver dequeued mystery buffer.\n");
			dump_buffer_info(LEVEL_ERROR, dev, buf);
			return -1;
		}
		BLTS_DEBUG("Buffer #%d (%p) dequeued\n", i, (void *)buf->m.userptr);
		if (dev->buffers[i].queued) {
			dev->buffers[i].queued = 0;
		} else {
			BLTS_ERROR("Error: Driver dequeued a buffer not currently queued.\n");
			return -1;
		}
	} else if (dev->io_method == IO_METHOD_MMAP) {
		i = buf->index;
		if (i >= dev->n_buffers) {
			BLTS_ERROR("Error: Driver dequeued mystery buffer.\n");
			dump_buffer_info(LEVEL_ERROR, dev, buf);
			return -1;
		}
		BLTS_DEBUG("Buffer #%d (offset = %u) dequeued\n", i, buf->m.offset);
		if (dev->buffers[i].queued) {
			dev->buffers[i].queued = 0;
		} else {
			BLTS_ERROR("Error: Driver dequeued a buffer not currently queued.\n");
			dump_buffer_info(LEVEL_ERROR, dev, buf);
			return -1;
		}
	}

	if (buf->length > dev->buffers[i].length) {
		BLTS_ERROR("Error: Buffer larger than queued length\n");
		dump_buffer_info(LEVEL_ERROR, dev, buf);
		return -1;
	}
	if (buf->length < dev->buffers[i].length) {
		if (buf->length == buf->bytesused) {
			BLTS_DEBUG("Driver returns short buffer (length == payload).\n");
		} else if (buf->length < buf->bytesused) {
			BLTS_ERROR("Invalid buffer (length > payload).\n");
			dump_buffer_info(LEVEL_ERROR, dev, buf);
			return -1;
		}
	}
	if (buf->length < dev->format.fmt.pix.sizeimage) {
		BLTS_ERROR("Error: Returned buffer too small for reported image size.\n");
		dump_buffer_info(LEVEL_ERROR, dev, buf);
		return -1;
	}

	return 0;
}

/**
 *  basic frame reading
 */
int read_frame(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	struct v4l2_buffer buf;
	unsigned int i;

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		LOGERR("No IO method selected");
		FUNC_LEAVE()
		return FALSE;
		break;
	case IO_METHOD_READ:
		if (-1 == read (dev->dev_fd, dev->buffers[0].start, dev->buffers[0].length))
		{
			switch (errno)
			{
			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				LOGERR ("read");
				FUNC_LEAVE()
				return FALSE;
			}
		}
		dev->capture_started = 1;

		process_image (dev, dev->buffers[0].start);

		break;

	case IO_METHOD_MMAP:
		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == ioctl_VIDIOC_DQBUF (dev->dev_fd, &buf,1))
		{
			switch (errno)
			{
			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				print_errno ("VIDIOC_DQBUF");
				FUNC_LEAVE()
				return FALSE;
			}
		}

		if (validate_dequeued_buffer(dev, &buf) < 0) {
			BLTS_ERROR("Error: Driver dequeued invalid buffer\n");
			return 0;
		}

		process_image (dev, dev->buffers[buf.index].start);

		if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf,1))
		{
			print_errno("VIDIOC_QBUF");
			FUNC_LEAVE()
			return FALSE;
		}
		dev->buffers[buf.index].queued = 1;
		break;

	case IO_METHOD_USERPTR:
		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == ioctl_VIDIOC_DQBUF (dev->dev_fd, &buf, 1))
		{
			switch (errno)
			{
			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				print_errno("VIDIOC_DQBUF");
				FUNC_LEAVE()
				return FALSE;
			}
		}

		if (validate_dequeued_buffer(dev, &buf) < 0) {
			BLTS_ERROR("Error: Driver dequeued invalid buffer\n");
			return 0;
		}
		for (i = 0; i < dev->n_buffers; ++i)
			if (buf.m.userptr == (unsigned long) dev->buffers[i].start)
				break;

		process_image (dev, (void *) buf.m.userptr);

		if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf,1))
		{
			print_errno("VIDIOC_QBUF");
			return FALSE;
		}
		dev->buffers[i].queued = 1;

		break;
	}
	return 1;
}


/**
 *  frame reading when saving JPG
 */

int read_frame_to_file(v4l2_dev_data *dev, char *filename)
{
	FUNC_ENTER()
	struct v4l2_buffer buf;
	unsigned int i;

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		LOGERR("No IO method selected");
		FUNC_LEAVE()
		return FALSE;
		break;

	case IO_METHOD_READ:
		if (-1 == read (dev->dev_fd, dev->buffers[0].start, dev->buffers[0].length))
		{
			switch (errno)
			{
			LOG("processing image..");
			fflush(stdout);

			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				LOGERR ("read");
				FUNC_LEAVE()
				return FALSE;
			}
		}
		process_image_file (dev, dev->buffers[0].start, filename);

		break;

	case IO_METHOD_MMAP:
		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == ioctl_VIDIOC_DQBUF (dev->dev_fd, &buf,1))
		{
			switch (errno)
			{
			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				print_errno ("VIDIOC_DQBUF");
				FUNC_LEAVE()
				return FALSE;
			}
		}

		assert (buf.index < dev->n_buffers);
		dev->buffers[buf.index].queued = 0;

		process_image_file (dev, dev->buffers[0].start, filename);

		if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf,1))
		{
			print_errno("VIDIOC_QBUF");
			FUNC_LEAVE()
			return FALSE;
		}
		dev->buffers[buf.index].queued = 1;

		break;

	case IO_METHOD_USERPTR:
		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == ioctl_VIDIOC_DQBUF (dev->dev_fd, &buf, 1))
		{
			switch (errno)
			{
			case EAGAIN:
				FUNC_LEAVE()
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				print_errno ("VIDIOC_DQBUF");
				FUNC_LEAVE()
				return FALSE;
			}
		}

		for (i = 0; i < dev->n_buffers; ++i)
			if (buf.m.userptr == (unsigned long) dev->buffers[i].start)
				break;

		assert (i < dev->n_buffers);
		dev->buffers[i].queued = 0;

		process_image_file (dev, dev->buffers[0].start, filename);

		if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf, 1))
		{
			print_errno ("VIDIOC_QBUF");
			FUNC_LEAVE()
			return FALSE;
		}
		dev->buffers[i].queued = 1;

		break;
	}
	FUNC_LEAVE()
	return 1;
}

/**
 * basic mainloop, best to use when measuring FPS
 */

boolean mainloop(v4l2_dev_data *dev, int loops)
{
	unsigned int count;
	int ret, retries;
	double elapsed;

	if (loops <= V4L2_DEVICE_INIT_FRAMES)
		loops += V4L2_DEVICE_INIT_FRAMES;

	count = loops;

	timing_start();
	while (count-- > 0)
	{
		for (retries = 10; retries; retries--)
		{
			if (dev->use_poll) {
				ret = do_poll_in_wait(dev->dev_fd, 2000);
				if (ret)
					return 0;
			} else {
				fd_set fds;
				struct timeval tv;
				int r;

				FD_ZERO (&fds);
				FD_SET (dev->dev_fd, &fds);

				/* Timeout. */
				tv.tv_sec = 2;
				tv.tv_usec = 0;

				r = select (dev->dev_fd + 1, &fds, NULL, NULL, &tv);

				if (-1 == r)
				{
					if (EINTR == errno)
						continue;
					if(dev->use_progress)
						LOGERR ("select");
					return FALSE;
				}

				if (0 == r)
				{
					//log_close();
					if(dev->use_progress)
						LOGERR ("Select timeout\n");
					return FALSE;
				}
			}

			if (read_frame (dev))
				break;

			/* EAGAIN - continue select loop. */
		}
		if (!retries) {
			BLTS_ERROR("Too many errors, giving up.\n");
			return FALSE;
		}
	}
	elapsed = timing_elapsed();
	LOG("\n%i frames read in %.2f seconds\n", loops, elapsed);
	dev->calculated_frame_rate = ((float)loops / elapsed);
	LOG("Frames per second: %.2f\n", dev->calculated_frame_rate);
	timing_stop();
	return TRUE;
}

/**
 * Mainloop when screen streaming required
 */

boolean mainloop_stream(v4l2_dev_data *dev,int loops, int sx, int sy, void *fb)
{
	unsigned int count;
	double elapsed;
	count = loops;
	timing_start();
	while (count-- > 0)
	{
		for (;;)
		{

			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO (&fds);
			FD_SET (dev->dev_fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select (dev->dev_fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r)
			{
				if (EINTR == errno)
					continue;
				print_errno ("select");
				return FALSE;
			}

			if (0 == r)
			{
				log_close();
				LOGERR("select timeout\n");
				return FALSE;
			}
			if (read_frame(dev))
			{
				if (errno == EAGAIN)
					continue;
				break;
			}
		}
	}

	elapsed = timing_elapsed();
	LOG("\n%i frames read in %.2f seconds\n", loops, elapsed);
	dev->calculated_frame_rate = ((float)loops / elapsed);
	LOG("Frames per second: %.2f\n", dev->calculated_frame_rate);
	timing_stop();

	return TRUE;
}

/**
 * Take one Frame and progress it to JPG file
 */
boolean do_snapshot(v4l2_dev_data *dev, char * filename)
{
	int retries;
	FUNC_ENTER()
	for (retries = 10; retries; retries--)
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO (&fds);
		FD_SET (dev->dev_fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select (dev->dev_fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r)
		{
			if (EINTR == errno)
				continue;

			LOGERR("select");
			return FALSE;
		}

		if (0 == r)
		{
			LOGERR ("select timeout\n");
			return FALSE;
		}

		if (read_frame_to_file (dev, filename))
			break;
		/* EAGAIN - continue select loop. */
	}
	if (!retries) {
		BLTS_ERROR("Too many errors, giving up.\n");
		return FALSE;
	}
	return TRUE;
}


/**
 * Tells camera to stop streaming
 */

boolean stop_capturing (v4l2_dev_data *dev)
{
	int buf_type, i;

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		/* fall through */
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl_VIDIOC_STREAMOFF (dev->dev_fd, &buf_type, 1))
		{
			print_errno("VIDIOC_STREAMOFF");
			return FALSE;
		}

		/* STREAMOFF clears queues, so we can clear queue check flags */
		for (i = 0; i < dev->n_buffers; ++i)
			dev->buffers[i].queued = 0;

		break;
	}
	return TRUE;
}


/**
 * Tells camera to start streaming and adress where to
 */

boolean start_capturing (v4l2_dev_data *dev)
{
	unsigned int i;
	int buf_type;

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
	case IO_METHOD_READ:
		/* Nothing to do */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < dev->n_buffers; ++i)
		{
			struct v4l2_buffer buf;

			CLEAR (buf);

			buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory	= V4L2_MEMORY_MMAP;
			buf.index	= i;

			if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf, 1))
			{
				print_errno("VIDIOC_QBUF");
				return FALSE;
			}
			dev->buffers[i].queued = 1;
		}

		buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl_VIDIOC_STREAMON (dev->dev_fd, &buf_type, 1))
		{
			print_errno ("VIDIOC_STREAMON");
			return FALSE;
		}

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < dev->n_buffers; ++i)
		{
			struct v4l2_buffer buf;

			CLEAR (buf);

			buf.type			= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory		= V4L2_MEMORY_USERPTR;
			buf.index		= i;
			buf.m.userptr	= (unsigned long) dev->buffers[i].start;
			buf.length		= dev->buffers[i].length;

			if (-1 == ioctl_VIDIOC_QBUF (dev->dev_fd, &buf, 1))
			{
				print_errno("VIDIOC_QBUF");
				return FALSE;
			}
			dev->buffers[i].queued = 1;
		}

		buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == ioctl_VIDIOC_STREAMON (dev->dev_fd, &buf_type, 1))
		{
			print_errno ("VIDIOC_STREAMON");
			return FALSE;
		}

		break;
	}

	dev->capture_started = 1;
	return TRUE;
}


/**
 * release mapped memory
 */
boolean uninit_device (v4l2_dev_data *dev)
{
	FUNC_ENTER();

	unsigned int i;
	assert(dev);

	if (!dev->buffers)
		return TRUE;

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		break;

	case IO_METHOD_READ:
		if (dev->n_buffers > 0 && dev->buffers[0].start)
			free(dev->buffers[0].start);
		dev->buffers[0].start = NULL;
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < dev->n_buffers; ++i){
			if (dev->buffers[i].queued)
				BLTS_WARNING("Warning: Buffer #%d (%p) still queued!\n",
					i, dev->buffers[i].start);
			if (dev->buffers[i].start) {
				if (munmap(dev->buffers[i].start,
					dev->buffers[i].length) < 0) {
					print_errno ("munmap");
					return FALSE;
				}
				dev->buffers[i].start = NULL;
			}
		}
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < dev->n_buffers; ++i) {
			if (dev->buffers[i].queued)
				BLTS_ERROR("Warning: Buffer #%d (%p) still queued!\n",
					i, dev->buffers[i].start);
			/* FIXME: likely we should force a STREAMOFF here, or at least
			   attempt to individually dequeue buffers. */
			if (dev->buffers[i].start)
				free(dev->buffers[i].start);
			dev->buffers[i].start = NULL;
		}
		break;
	}

	free(dev->buffers);
	dev->buffers = NULL;
	return TRUE;
}

/**
 * Try to get memory for camera
 */

boolean init_read (v4l2_dev_data *dev, unsigned int buffer_size)
{
	FUNC_ENTER();
	assert(dev);
	if (!dev->n_buffers)
		dev->n_buffers++;

	dev->buffers = malloc(dev->n_buffers * sizeof (struct buffer));

	if (!dev->buffers)
	{
		LOGERR("Out of memory\n");
		return FALSE;
	}

	dev->buffers[0].length = buffer_size;
	dev->buffers[0].start = malloc (buffer_size);
	dev->buffers[0].queued = 0;

	if (!dev->buffers[0].start)
	{
		LOGERR ("Out of memory\n");
		return FALSE;
	}
	return TRUE;
}


/**
 * map initialized memory for camera, used when MMAP method selected
 */

boolean init_mmap(v4l2_dev_data *dev)
{
	FUNC_ENTER();

	struct v4l2_requestbuffers req;

	CLEAR (req);
	req.count			= REQUESTED_BUFFERS_COUNT;
	req.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory			= V4L2_MEMORY_MMAP;

	if (-1 == ioctl_VIDIOC_REQBUFS (dev->dev_fd, &req, 1))
	{
		if (EINVAL == errno)
		{
			LOGERR ("%s does not support "
					 "memory mapping\n", dev->dev_name);
			FUNC_LEAVE()
			return FALSE;
		}
		else
		{
			print_errno ("VIDIOC_REQBUFS");
			FUNC_LEAVE()
			return FALSE;
		}
	}

	if (req.count < 2)
	{
		LOGERR ("Insufficient buffer memory on %s\n",
				 dev->dev_name);
		FUNC_LEAVE()
		return FALSE;
	}

	dev->buffers = calloc (req.count, sizeof (*dev->buffers));
	if (!dev->buffers)
	{
		LOGERR("Out of memory\n");
		FUNC_LEAVE()
		return FALSE;
	}
	memset(dev->buffers, 0, req.count * sizeof (*dev->buffers));

	for (dev->n_buffers = 0; dev->n_buffers < req.count; ++dev->n_buffers)
	{
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= dev->n_buffers;

		if (-1 == ioctl_VIDIOC_QUERYBUF (dev->dev_fd, &buf, 1))
		{
			print_errno ("VIDIOC_QUERYBUF");
			FUNC_LEAVE()
			return FALSE;
		}

		dev->buffers[dev->n_buffers].length = buf.length;
		dev->buffers[dev->n_buffers].start =
			mmap (NULL /* start anywhere */,
			buf.length,
			PROT_READ | PROT_WRITE /* required */,
			MAP_SHARED /* recommended */,
			dev->dev_fd, buf.m.offset);

		if (MAP_FAILED == dev->buffers[dev->n_buffers].start)
		{
			LOGERR ("mmap error");
			FUNC_LEAVE()
			return FALSE;
		}
	}
	return TRUE;
}
/**
 * user pointer case
 */
boolean init_userp(v4l2_dev_data *dev, unsigned int buffer_size)
{
	FUNC_ENTER();

	struct v4l2_requestbuffers req;
	unsigned int page_size;

	page_size = getpagesize ();
	buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

	CLEAR (req);

	req.count			= REQUESTED_BUFFERS_COUNT;
	req.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory			= V4L2_MEMORY_USERPTR;

	if (-1 == ioctl_VIDIOC_REQBUFS (dev->dev_fd, &req, 1))
	{
		if (EINVAL == errno)
		{
			LOGERR ("%s does not support "
					 "user pointer i/o\n", dev->dev_name);
			return FALSE;
		}
		else
		{
			print_errno ("VIDIOC_REQBUFS");
			return FALSE;
		}
	}

	dev->buffers = calloc (req.count, sizeof (*dev->buffers));

	if (!dev->buffers)
	{
		LOGERR("Out of memory\n");
		return FALSE;
	}
	memset(dev->buffers, 0, req.count * sizeof (*dev->buffers));

	for (dev->n_buffers = 0; dev->n_buffers < req.count; ++dev->n_buffers)
	{
		dev->buffers[dev->n_buffers].length = buffer_size;
		dev->buffers[dev->n_buffers].start = memalign (/* boundary */ page_size,
			buffer_size);

		if (!dev->buffers[dev->n_buffers].start)
		{
			LOGERR ("Out of memory\n");
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * open camera device to requested width and height
 */

boolean init_device (v4l2_dev_data *dev, int req_cam_width, int req_cam_height)
{
	FUNC_ENTER();

	unsigned int min;

	if (-1 == ioctl_VIDIOC_QUERYCAP (dev->dev_fd, &dev->capability, 1))
	{
		if (EINVAL == errno)
		{
			LOGERR("%s is no V4L2 device\n",
					 dev->dev_name);
			FUNC_LEAVE();
			return FALSE;
		}
		else
		{
			LOG("VIDIOC_QUERYCAP error\n");
			FUNC_LEAVE();
			return FALSE;
		}
	}

	if (!(dev->capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		LOGERR ("%s is no video capture device\n",
				 dev->dev_name);
		FUNC_LEAVE();
		return FALSE;
	}

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		break;
	case IO_METHOD_READ:
		if (!(dev->capability.capabilities & V4L2_CAP_READWRITE))
		{
			LOGERR("%s does not support read i/o\n",
					 dev->dev_name);
			FUNC_LEAVE();
			return FALSE;
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(dev->capability.capabilities & V4L2_CAP_STREAMING))
		{
			LOGERR("%s does not support streaming i/o\n",
					 dev->dev_name);
			FUNC_LEAVE();
			return FALSE;
		}
		break;
	}


	/* Select video input, video standard and tune here. */


	CLEAR (dev->cropcap);

	dev->cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == ioctl_VIDIOC_CROPCAP (dev->dev_fd, &dev->cropcap, 1))
	{
		dev->crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		dev->crop.c = dev->cropcap.defrect; /* reset to default */

		if (-1 == ioctl_VIDIOC_S_CROP (dev->dev_fd, &dev->crop, 1))
		{
			switch (errno)
			{
			case EINVAL:
				LOGERR("Cropping not supported\n");
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	}
	else
	{
		/* Errors ignored. */
	}


	CLEAR (dev->format);

	dev->format.type						= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dev->format.fmt.pix.width			= req_cam_width;
	dev->format.fmt.pix.height			= req_cam_height;
	dev->format.fmt.pix.field 			= V4L2_FIELD_INTERLACED;


	/* Note: may change width and height. */
	/* LOG("%c %c %c %c\n", dev->fmt_a,dev->fmt_b,dev->fmt_c,dev->fmt_d); */

	if (dev->requested_format.fmt.pix.pixelformat)
		if (try_format(dev, dev->requested_format.fmt.pix.pixelformat) == FALSE)
		{
			LOGERR("Try requested pixelformat failed!\n");
			return FALSE;
		}

	if (-1 == ioctl_VIDIOC_G_FMT (dev->dev_fd, &dev->format, 1))
	{
		print_errno ("VIDIOC_G_FMT error");
		FUNC_LEAVE();
		return FALSE;
	}

	/* JPEG format speciality */
	if(dev->format.fmt.pix.pixelformat == v4l2_fourcc('J','P','E','G'))
	{
		LOG("Trying to get and set compression options\n");
		// let device process the image by itself, just add our "watermark"
		struct v4l2_jpegcompression compression;
		CLEAR (compression);
		if (ioctl_VIDIOC_G_JPEGCOMP(dev->dev_fd, &compression, 1))
		{
		    BLTS_ERROR("VIDIOC_G_JPEGCOMP\n");
		}
		// Mark BLTS to APP data
		// APP_data is 60 bytes long field and we just add few bytes there
		strcpy(compression.APP_data, BLTS_WATERMARK);

		if (ioctl_VIDIOC_S_JPEGCOMP (dev->dev_fd, &compression, 1))
		{
			BLTS_ERROR("VIDIOC_S_JPEGCOMP\n");
		}
	}


	/* Buggy driver paranoia. */
	min = dev->format.fmt.pix.width * 2;
	if (dev->format.fmt.pix.bytesperline < min)
		dev->format.fmt.pix.bytesperline = min;
	min = dev->format.fmt.pix.bytesperline * dev->format.fmt.pix.height;
	if (dev->format.fmt.pix.sizeimage < min)
		dev->format.fmt.pix.sizeimage = min;

	BLTS_DEBUG("-- bytesperline=%i\n", dev->format.fmt.pix.bytesperline);
	BLTS_DEBUG("-- sizeimage=%i\n", dev->format.fmt.pix.sizeimage);

	switch (dev->io_method)
	{
	case IO_METHOD_NONE:
		break;
	case IO_METHOD_READ:
		return init_read (dev, dev->format.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		return init_mmap (dev);
		break;

	case IO_METHOD_USERPTR:
		return init_userp (dev, dev->format.fmt.pix.sizeimage);
		break;
	}
	FUNC_LEAVE();
	return TRUE;
}


boolean close_device (v4l2_dev_data *dev)
{
	FUNC_ENTER();

	if (-1 == close(dev->dev_fd))
	{
		BLTS_ERROR("Device cannot be closed.\n");
		FUNC_LEAVE();
		return FALSE;
	}
	LOG("Device closed.\n");
	dev->dev_fd = -1;

	FUNC_LEAVE();
	return TRUE;
}

boolean open_device (v4l2_dev_data *dev)
{
	FUNC_ENTER();

	struct stat st;

	if (-1 == stat (dev->dev_name, &st))
	{
		LOGERR("Cannot identify '%s': %d, %s\n",
				 dev->dev_name, errno, strerror (errno));
		FUNC_LEAVE();
		return FALSE;
	}

	if (!S_ISCHR (st.st_mode))
	{
		LOGERR ("%s is no device\n", dev->dev_name);
		FUNC_LEAVE();
		return FALSE;
	}

	dev->dev_fd = open (dev->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == dev->dev_fd)
	{
		LOGERR("Cannot open '%s': %d, %s\n",
			 dev->dev_name, errno, strerror (errno));
		FUNC_LEAVE();
		return FALSE;
	}
	LOG("Device %s opened\n", dev->dev_name);

	FUNC_LEAVE();
	return TRUE;
}

boolean device_enumeration(v4l2_dev_data *dev)
{
	int i;
	int ret;

	if(!open_device (dev))
	{
		LOG("Can't open device %s\n", dev->dev_name);
		return FALSE;
	}

	LOG("Video inputs on \"%s\":\n", dev->dev_name);
	for (i = 0, ret = 0; ret == 0; ++i)
	{
		struct v4l2_input input;
		memset(&input, 0, sizeof(struct v4l2_input));
		input.index = i;
		ret = ioctl_VIDIOC_ENUMINPUT(dev->dev_fd, &input, 0);

		if (!ret)
		{
			struct v4l2_standard standard;
			LOG("Current input \"%s\" supports:\n", input.name);
			memset (&standard, 0, sizeof (standard));
			standard.index = 0;

			while (0 == ioctl_VIDIOC_ENUMSTD(dev->dev_fd, &standard, 0))
			{
				if (standard.id & input.std)
						LOG("%s\n", standard.name);
				standard.index++;
			}
		}
	}


	LOG("Video outputs on \"%s\":\n", dev->dev_name);
	for (i = 0, ret = 0; ret == 0; ++i)
	{
		struct v4l2_output output;
		output.index = i;
		ret = ioctl_VIDIOC_ENUMOUTPUT(dev->dev_fd, &output, 0);
		if (!ret)
		{
			struct v4l2_standard standard;
			LOG("Current output \"%s\" supports:\n", output.name);
			memset (&standard, 0, sizeof (standard));
			standard.index = 0;

			while (0 == ioctl_VIDIOC_ENUMSTD(dev->dev_fd, &standard, 0))
			{
				if (standard.id & output.std)
					LOG("%s\n", standard.name);
				standard.index++;
			}
		}
	}

	close_device(dev);

	return TRUE;
}

char *create_picture_filename(v4l2_dev_data *dev)
{
	FUNC_ENTER()
	char *name = malloc(sizeof(char) * 50);
	sprintf (name, "image_%ix%i_%c%c%c%c_dev%c_", dev->requested_format.fmt.pix.width, dev->requested_format.fmt.pix.height,
		dev->fmt_a, dev->fmt_b, dev->fmt_c, dev->fmt_d, dev->dev_name[strlen(dev->dev_name)-1]);
	switch(dev->io_method)
	{
	case IO_METHOD_MMAP:
		strcat(name, "mmap.jpg");
		break;
	case IO_METHOD_USERPTR:
		strcat(name, "userp.jpg");
		break;
	case IO_METHOD_READ:
		strcat(name, "read.jpg");
		break;
	case IO_METHOD_NONE:
		strcat(name, ".jpg");
		break;
	}
	FUNC_LEAVE()
	return name;

}

void print_crop_data(v4l2_dev_data *dev)
{
	int ret;
	struct v4l2_crop crop;
	struct v4l2_cropcap cropcap;

	memset (&crop, 0, sizeof (crop));
	memset (&cropcap, 0, sizeof (cropcap));

	if (0 == ioctl_VIDIOC_CROPCAP(dev->dev_fd, &cropcap, 1))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ret = ioctl_VIDIOC_G_CROP(dev->dev_fd, &crop, 0);
		if(ret)
		{
			perror("VIDIOC_G_CROP\n");
			return;
		}
		else
		{
			LOG("VIDIOC_G_CROP successful\n");
		}

		LOG("crop: top = %d, left = %d, width = %d, height = %d\n",crop.c.top, crop.c.left,
			crop.c.width, crop.c.height);
		LOG("cropcap.bounds: top = %d, left = %d, width = %d, height = %d\n", cropcap.bounds.top,
			cropcap.bounds.left, cropcap.bounds.width, cropcap.bounds.height);
		LOG("cropcap.defrect: top = %d, left = %d, width = %d, height = %d\n",
			cropcap.defrect.left, cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height);
	}
}

int do_set_crop(v4l2_dev_data *dev, unsigned int set_crop, struct v4l2_rect *vcrop, enum v4l2_buf_type type)
{
	struct v4l2_crop in_crop;

	in_crop.type = type;

	if (ioctl_VIDIOC_G_CROP(dev->dev_fd, &in_crop, 0) == 0)
	{
		if (set_crop & CROP_WIDTH)
		{
			LOG("Set width:%d\n",vcrop->width);
			in_crop.c.width = vcrop->width;
		}
		if (set_crop & CROP_HEIGHT)
		{
			LOG("Set height:%d\n", vcrop->height);
			in_crop.c.height = vcrop->height;
		}
		if (set_crop & CROP_LEFT)
		{
			LOG("Set left:%d\n", vcrop->left);
			in_crop.c.left = vcrop->left;
		}
		if (set_crop & CROP_TOP)
		{
			LOG("Set top:%d\n",vcrop->top);
				in_crop.c.top = vcrop->top;
		}

		if(ioctl_VIDIOC_S_CROP(dev->dev_fd, &in_crop, 0)!=0)
		{
			LOG("error setting cropping info\n");
			return FALSE;
		}
	}
	else
	{
		LOG("error getting cropping info\n");
		return FALSE;
	}

	return TRUE;
}

boolean mainloop_xvideo_stream(v4l2_dev_data *dev, int loops)
{
	unsigned int count = loops;

	while (count-- > 0)
	{
		for (;;)
		{
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO (&fds);
			FD_SET (dev->dev_fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select (dev->dev_fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r)
			{
				if (EINTR == errno)
					continue;

				print_errno ("select");
				return FALSE;
			}

			if (0 == r)
			{
				LOGERR("select timeout\n");
				return FALSE;
			}

			if (read_frame (dev))
				break;

			/* EAGAIN - continue select loop. */
		}
	}
	return TRUE;
}

int crop_corners(v4l2_dev_data *dev, int loops)
{
	struct v4l2_cropcap cropcap;
	struct v4l2_rect rec;

	int set = CROP_LEFT | CROP_TOP | CROP_WIDTH | CROP_HEIGHT;
	memset(&cropcap, 0, sizeof(struct v4l2_cropcap));

	if (-1 == ioctl_VIDIOC_CROPCAP(dev->dev_fd, &cropcap, 0))
	{
		perror ("VIDIOC_CROPCAP");
		return FALSE;
	}

	LOG("Loops / cropping area :%d\n", loops);

	//+-----+
	//|x |  |
	//|--+--|
	//|  |  |
	//+-----+
	rec.left = cropcap.bounds.left;
	rec.width = (cropcap.bounds.width / 2);
	rec.top = cropcap.bounds.top;
	rec.height = cropcap.bounds.height - (cropcap.bounds.height / 2);

	if(!do_set_crop(dev, set, &rec, V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return FALSE;
	print_crop_data(dev);

	if(!mainloop (dev, loops))
		return FALSE;
	//+-----+
	//|  |  |
	//|--+--|
	//|x |  |
	//+-----+
	rec.left = cropcap.bounds.left;
	rec.width = (cropcap.bounds.width / 2);
	rec.top = cropcap.bounds.top + (cropcap.bounds.height / 2);
	rec.height = cropcap.bounds.height - (cropcap.bounds.height / 2);

	if(!do_set_crop(dev, set, &rec, V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return FALSE;
	print_crop_data(dev);

	if(!mainloop (dev, loops))
		return FALSE;

	//+-----+
	//|  |x |
	//|--+--|
	//|  |  |
	//+-----+
	rec.left = cropcap.bounds.left + (cropcap.bounds.width / 2);
	rec.width = (cropcap.bounds.width / 2);
	rec.top = cropcap.bounds.top;
	rec.height = cropcap.bounds.height - (cropcap.bounds.height / 2);

	if(!do_set_crop(dev, set, &rec, V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return FALSE;
	print_crop_data(dev);

	if(!mainloop (dev, loops))
		return FALSE;

	//+-----+
	//|  |  |
	//|--+--|
	//|  |x |
	//+-----+
	rec.left = cropcap.bounds.left + (cropcap.bounds.width / 2);
	rec.width = (cropcap.bounds.width / 2);
	rec.top = cropcap.bounds.top + (cropcap.bounds.height / 2);
	rec.height = cropcap.bounds.height - (cropcap.bounds.height / 2);

	if(!do_set_crop(dev, set, &rec, V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return FALSE;
	print_crop_data(dev);

	if(!mainloop (dev, loops))
		return FALSE;

	return TRUE;
}

int stream_image_xv(v4l2_dev_data *dev, int loops)
{
	return mainloop(dev, loops);
}

/* float_to_fraction -functions originally from program called uvcview */
static int float_to_fraction_recursive(double f, double p, int *num, int *den)
{
	int whole = (int)f;
	f = fabs(f - whole);

	if(f > p) {
		int n, d;
		int a = float_to_fraction_recursive(1 / f, p + p / f, &n, &d);
		*num = d;
		*den = d * a + n;
	}
	else {
		*num = 0;
		*den = 1;
	}
	return whole;
}

static void float_to_fraction(float f, int *num, int *den)
{
	int whole = float_to_fraction_recursive(f, FLT_EPSILON, num, den);
	*num += whole * *den;
}

int get_framerate_fps(v4l2_dev_data *dev, float* fps)
{
	int numerator = 0;
	int denominator = 0;

	return get_framerate(dev, &numerator, &denominator, fps);
}

int get_framerate(v4l2_dev_data *dev, int *numerator, int *denominator, float* fps)
{
	int ret;
	struct v4l2_streamparm parm;

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl_VIDIOC_G_PARM(dev->dev_fd, &parm, 0);
	if (ret < 0)
	{
		print_errno("VIDIOC_G_PARM");
		return ret;
	}

	*numerator = parm.parm.capture.timeperframe.numerator;
	*denominator = parm.parm.capture.timeperframe.denominator;
	*fps = (float)parm.parm.capture.timeperframe.denominator / (float)parm.parm.capture.timeperframe.numerator;

	LOG("Current frame rate: %u/%u (%g fps)\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator,
		*fps);

	return 0;
}


int set_framerate_fps(v4l2_dev_data *dev, float* fps)
{
	int ret = 0 ;
	int n = 0;
	int d = 0;
	float cfps = 0.0;

	float_to_fraction(*fps, &n, &d);

	LOG("\nfps=%lf, n=%d, d=%d\n", *fps, n, d);

	ret = set_framerate(dev, d, n, &cfps);

	if(ret != 0)
	{
		LOG("Unable to set frame rate!\n");
		return ret;
	}

	if (cfps != (float)n / (float)d)
	{
		LOG("Frame rate: %g fps (requested %g fps not supported)\n", cfps, *fps);
	}
	else
	{
		LOG("Frame rate: %g fps\n", cfps);
	}
	*fps = cfps;

	return 0;
}

int set_framerate(v4l2_dev_data *dev, int numerator, int denominator, float* fps)
{
	int ret;
	struct v4l2_streamparm parm;

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl_VIDIOC_G_PARM(dev->dev_fd, &parm, 0);
	if (ret < 0)
	{
		print_errno("VIDIOC_G_PARM");
		return ret;
	}

	LOG("Current frame rate: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	parm.parm.capture.timeperframe.numerator = numerator;
	parm.parm.capture.timeperframe.denominator = denominator;

	ret = ioctl_VIDIOC_S_PARM(dev->dev_fd, &parm, 0);
	if (ret < 0)
	{
		print_errno("VIDIOC_S_PARM");
		return ret;
	}

	ret = ioctl_VIDIOC_G_PARM(dev->dev_fd, &parm, 0);
	if (ret < 0)
	{
		print_errno("VIDIOC_G_PARM");
		return ret;
	}

	*fps = (float)parm.parm.capture.timeperframe.denominator / (float)parm.parm.capture.timeperframe.numerator;

	LOG("Frame rate set: %u/%u (%g fps)\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator,
		*fps);

	return 0;
}


int driver_debug(v4l2_dev_data *dev)
{
	FUNC_ENTER();
	int ret;
	ret = ioctl_VIDIOC_LOG_STATUS(dev->dev_fd, 0);
	BLTS_DEBUG("VIDIOC_LOG_STATUS returned %i\n", ret);
	FUNC_LEAVE();
	return ret;
}



int has_output(v4l2_dev_data *dev, int index)
{
	FUNC_ENTER();
	int ret;
	struct v4l2_output output;
	memset(&output, 0, sizeof(struct v4l2_output));
	output.index = index;
	ret = ioctl_VIDIOC_ENUMOUTPUT(dev->dev_fd, &output, 0);
	if (!ret)
	{
		LOG("Output \"%s\" available\n", output.name);
	}
	FUNC_LEAVE();
	return ret;
}

int has_input(v4l2_dev_data *dev, int index)
{
	FUNC_ENTER();
	int ret;
	struct v4l2_input input;
	memset(&input, 0, sizeof(struct v4l2_input));
	input.index = index;
	ret = ioctl_VIDIOC_ENUMINPUT(dev->dev_fd, &input, 0);
	if (!ret)
	{
		LOG("Input \"%s\" available\n", input.name);
	}
	FUNC_LEAVE();
	return ret;
}

int try_output(v4l2_dev_data *dev, int index)
{
	FUNC_ENTER();
	int ret;
	ret = ioctl_VIDIOC_S_OUTPUT(dev->dev_fd, &index, 0);
	FUNC_LEAVE();
	return ret;
}

int get_output(v4l2_dev_data *dev, int *index)
{
	FUNC_ENTER();
	int ret;
	ret = ioctl_VIDIOC_G_OUTPUT(dev->dev_fd, index, 0);
	FUNC_LEAVE();
	return ret;
}

int try_input(v4l2_dev_data *dev, int index)
{
	FUNC_ENTER();
	int ret;
	ret = ioctl_VIDIOC_S_INPUT(dev->dev_fd, &index, 0);
	FUNC_LEAVE();
	return ret;
}

int get_input(v4l2_dev_data *dev, int *index)
{
	FUNC_ENTER();
	int ret;
	ret = ioctl_VIDIOC_G_INPUT(dev->dev_fd, index, 0);
	FUNC_LEAVE();
	return ret;
}


boolean do_get_standards(v4l2_dev_data *dev, const int check_ret, const int check_errno, struct v4l2_input* input)
{
	v4l2_std_id std_id;
	int get_ret;
	int get_errno;

	memset(&std_id, 0, sizeof(std_id));
	get_ret = ioctl_VIDIOC_G_STD(dev->dev_fd, &std_id, 0);
	get_errno = errno;

	LOG("\t\tGet standard %llx - ret %d - errno %d\n", std_id, get_ret, get_errno);

	if (check_ret == 0)
	{
		/* Driver shall set the std field of struct v4l2_input to zero when standards not available */
		if (!input->std)
		{
			LOG("\n\nCannot get standard IDs from HW\n");
			return TRUE;
		}
		else
		{
			/* No errors - get standard success */
			if (!get_ret)
			{
				//TODO check id validity
				return TRUE;
			}
			else
			{
				return(get_ret == -1 && get_errno == EINVAL)?TRUE:FALSE;
			}
		}
	}

	/* EINVAL is expected value here */
	if(check_ret != -1 || check_errno != EINVAL)
	{
		LOGERR("unexpected error values %d %d!\n", check_ret, check_errno);
		return FALSE;
	}


	return TRUE;
}

boolean set_video_standard(v4l2_dev_data *dev, v4l2_std_id id, const int check_ret, const int check_errno, struct v4l2_input* input)
{
	int set_ret;
	int set_errno;
	int get_ret;
	int get_errno;
	v4l2_std_id std_id = id;

	/* Try to set a given standard by ID */
	errno = 0;
	set_ret = ioctl_VIDIOC_S_STD(dev->dev_fd, &std_id, 0);
	set_errno = errno;
	LOG("\t\tSet standard %llx - ret %d - errno %d\n", std_id, set_ret, set_errno);

	/* Get current standard ID */
	errno = 0;
	memset(&std_id, 0, sizeof(std_id));
	get_ret = ioctl_VIDIOC_G_STD(dev->dev_fd, &std_id, 0);
	get_errno = errno;
	LOG("\t\tGet standard %llx - ret %d - errno %d\n", std_id, get_ret, get_errno);

	if (check_ret == 0)
	{
		/* Driver shall set the std field of struct v4l2_input to zero when standards not available */
		if (!input->std)
		{
			LOG("\t\tCannot get standard IDs from HW\n");
			return TRUE;
		}
		else
		{
			/* No errors - set standard success */
			if (set_ret == 0)
			{
				//TODO check id validity
				return (get_ret == 0)?TRUE:FALSE;
			}
			else
			{
				return(set_ret == -1 && set_errno == EINVAL)?TRUE:FALSE;
			}
		}
	}

	/* EINVAL is expected value here */
	if(check_ret != -1 || check_errno != EINVAL)
	{
		LOGERR("unexpected error values %d %d!\n", check_ret, check_errno);
		return FALSE;
	}

	return TRUE;
}
boolean do_set_standards(v4l2_dev_data *dev, const int check_ret, const int check_errno, struct v4l2_input* input)
{
	int orig_ret;
	int orig_errno;
	int enum_ret;
	int enum_errno;

	v4l2_std_id sensed_id;
	v4l2_std_id original_id;
	struct v4l2_standard std;

	int index = 0;
	int failed = 0;

	memset(&sensed_id, 0, sizeof(sensed_id));
	memset(&original_id, 0, sizeof(original_id));

	/* Try to sense the video standard received by the current input if driver
	 * supports this, but do not fail the test case here */
	if (!ioctl_VIDIOC_QUERYSTD(dev->dev_fd, &sensed_id, 0))
	{
		LOG("\tSensed standard from device driver %llx\n", sensed_id);
	}

	/* Save the original standard if possible */
	errno = 0;
	orig_ret = ioctl_VIDIOC_G_STD(dev->dev_fd, &original_id, 0);
	orig_errno = errno;

	LOG("\tGet original standard %llx - ret %d - errno %d\n", original_id, orig_ret, orig_errno);
	do
	{
		errno = 0;
		memset(&std, 0, sizeof(std));
		std.index = index;
		enum_ret = ioctl_VIDIOC_ENUMSTD(dev->dev_fd, &std, 0);
		enum_errno = errno;

		LOG("\tEnum standard index %d - ret %d - errno %d\n", index, enum_ret, enum_errno);

		if (enum_ret == 0)
		{
			if(!set_video_standard(dev, std.id, check_ret, check_errno, input))
			{
				LOGERR("Failed to set video standard: %llx\n", std.id);
				failed++;
			}
		}

		index++;

	} while (enum_ret == 0 && index != 0);

	if (orig_ret == 0)
	{
		/* Restore the original standard */
		if (input->std)
		{
			if(!set_video_standard(dev, original_id, check_ret, check_errno, input))
			{
				LOGERR("Failed to restore original standard: %llx\n", original_id);
				failed++;
			}
			else
			{
				LOG("\tOriginal standard %llx restored\n", original_id);
			}
		}
		else
		{
			LOG("\tNo Original standard ID from HW available\n");
		}
	}
	else
	{
		/* EINVAL is expected value here */
		if(orig_ret != -1 || orig_errno != EINVAL)
		{
			LOGERR("Unexpected error values %d %d!\n", orig_ret, orig_errno);
			failed++;
		}
	}

	if(failed > 0)
	{
		LOGERR("Test failed (%d failures)\n", failed);
		FUNC_LEAVE()
		return FALSE;
	}

	FUNC_LEAVE()
	return TRUE;
}

boolean loop_through_video_inputs(v4l2_dev_data *dev, V4L2VideoInputFunc fp)
{
	FUNC_ENTER()

	int enum_ret;
	int enum_errno;

	int first_round = 1;
	int original_index = 0;
	int index = 0;
	int failed = 0;

	struct v4l2_input input;

	/* Save the original input index */
	if (-1 == ioctl_VIDIOC_G_INPUT(dev->dev_fd, &original_index, 0) )
	{
		LOGERR("Save the original input index failed!\n");
		FUNC_LEAVE()
		return FALSE;
	}

	LOG("Original input index %d\n", original_index);

	do
	{
		errno = 0;
		memset(&input, 0, sizeof(input));
		input.index = index;

		enum_ret = ioctl_VIDIOC_ENUMINPUT(dev->dev_fd, &input, 0);
		enum_errno = errno;

		LOG("\tEnum input index %d - ret %d - errno %d\n", index, enum_ret, enum_errno);

		if (enum_ret == 0)
		{
			if (-1 == ioctl_VIDIOC_S_INPUT(dev->dev_fd, (int*) &input.index, 0) )
			{
				LOGERR("\tSet input index %d failed!\n", input.index);
				failed++;
				goto cleanup;
			}
			LOG("\tTest input index %d\n", input.index);
		}

		/* Test get/set standard ioctl at least once */
		if (first_round || enum_ret == 0)
		{
			if(!fp(dev, enum_ret, enum_errno, &input))
				failed++;

			first_round = 0;
		}

		index++;

	} while (enum_ret == 0 && index != 0);

cleanup:
	/* Restore the original input index */
	if (-1 == ioctl_VIDIOC_S_INPUT(dev->dev_fd, &original_index, 0) )
	{
		LOGERR("Failed to restore original input\n");
		failed++;
	}
	else
	{
		LOG("Original input index %d restored\n", original_index);
	}

	if(failed > 0)
	{
		LOGERR("Test failed (%d failures)\n", failed);
		FUNC_LEAVE()
		return FALSE;
	}
	FUNC_LEAVE()
	return TRUE;
}
