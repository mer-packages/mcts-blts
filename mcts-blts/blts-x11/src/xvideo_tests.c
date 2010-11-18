/* xvideo_tests.c -- Tests for X Video extension

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

#include <sys/shm.h>
#include <ctype.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include "xvideo_tests.h"
#include "blts_x11_util.h"

int xvideo_init_test(double execution_time)
{
	int ret = 0;
	unsigned int ver, rev, eventB, reqB, errorB;
	unsigned int i, j, n;
	unsigned int nencode, nadaptors, nscreens;
	int nattr, nimages;
	XvAdaptorInfo *ainfo = NULL;
	XvAttribute *attributes;
	XvEncodingInfo *encodings;
	XvFormat *format;
	XvImageFormatValues *formats;
	window_struct params;
	int xv_port = -1;
	XvImage* yuv_image = NULL;
	int image_encodings = 0;
	unsigned int guid = 0;
	unsigned int bpp = 0;

	if(create_window(&params, "Test Window"))
	{
		ret = -1;
		goto cleanup;
	}

	if(XvQueryExtension(params.display, &ver, &rev, &reqB, &eventB,
		&errorB) != Success)
	{
		BLTS_ERROR("X-Video extension not found\n");
		ret = -1;
		goto cleanup;
	}

	BLTS_DEBUG("XVideo Extension version %i.%i\n", ver, rev);

	nscreens = ScreenCount(params.display);

	for(i = 0; i < nscreens; i++)
	{
		BLTS_DEBUG("screen #%i\n", i);
		XvQueryAdaptors(params.display, RootWindow(params.display, i),
			&nadaptors, &ainfo);

		if(!nadaptors)
		{
			BLTS_DEBUG(" no adaptors present\n");
			continue;
		}

		for(j = 0; j < nadaptors; j++)
		{
			BLTS_DEBUG("  Adaptor #%i: \"%s\"\n", j, ainfo[j].name);
			BLTS_DEBUG("    number of ports: %li\n", ainfo[j].num_ports);
			BLTS_DEBUG("    port base: %li\n", ainfo[j].base_id);
			BLTS_DEBUG("    operations supported: ");

			switch(ainfo[j].type & (XvInputMask | XvOutputMask))
			{
				case XvInputMask:
					if(ainfo[j].type & XvVideoMask)
					{
						BLTS_DEBUG("PutVideo ");
					}
					if(ainfo[j].type & XvStillMask)
					{
						BLTS_DEBUG("PutStill ");
					}
					if(ainfo[j].type & XvImageMask)
					{
						BLTS_DEBUG("PutImage ");
						/* Use the first found adaptor with PutImage capability
						for XvShmPutImage test */
						if(xv_port == -1 && i == (unsigned int)params.screen)
						{
							xv_port = ainfo[j].base_id;
						}
					}
					break;

				case XvOutputMask:
					if(ainfo[j].type & XvVideoMask)
					{
						BLTS_DEBUG("GetVideo ");
					}
					if(ainfo[j].type & XvStillMask)
					{
						BLTS_DEBUG("GetStill ");
					}
					break;

				default:
					BLTS_DEBUG("none ");
					break;
			}
			BLTS_DEBUG("\n");

			format = ainfo[j].formats;
			attributes = XvQueryPortAttributes(params.display,
				ainfo[j].base_id, &nattr);

			XvQueryEncodings(params.display, ainfo[j].base_id, &nencode,
				&encodings);
			if(!encodings || !nencode)
			{
				continue;
			}

			image_encodings = 0;

			for(n = 0; n < nencode; n++)
			{
				if(!strcmp(encodings[n].name, "XV_IMAGE"))
				{
					image_encodings++;
				}
			}

			if(nencode - image_encodings)
			{
				BLTS_DEBUG("    number of encodings: %i\n", nencode - image_encodings);

				for(n = 0; n < nencode; n++)
				{
					if(strcmp(encodings[n].name, "XV_IMAGE"))
					{
						BLTS_DEBUG("      encoding ID #%li: \"%s\"\n",
							encodings[n].encoding_id, encodings[n].name);
						BLTS_DEBUG("        size: %li x %li\n", encodings[n].width,
							encodings[n].height);
						BLTS_DEBUG("        rate: %f\n",
							(float)encodings[n].rate.numerator /
							(float)encodings[n].rate.denominator);
					}
				}
			}

			if(image_encodings && (ainfo[j].type & XvImageMask))
			{
				char imageName[5];

				for(n = 0; n < nencode; n++)
				{
					if(!strcmp(encodings[n].name, "XV_IMAGE"))
					{
						BLTS_DEBUG("    maximum XvImage size: %li x %li\n",
							encodings[n].width, encodings[n].height);
						break;
					}
				}

				formats = XvListImageFormats(params.display,
					ainfo[j].base_id, &nimages);
				BLTS_DEBUG("    Number of image formats: %i\n", nimages);

				for(n = 0; n < (unsigned int)nimages; n++)
				{
					sprintf(imageName, "%c%c%c%c", formats[n].id & 0xff,
						(formats[n].id >> 8) & 0xff,
						(formats[n].id >> 16) & 0xff,
						(formats[n].id >> 24) & 0xff);
					BLTS_DEBUG("      id: 0x%x", formats[n].id);
					/* Use the first found format for XvShmPutImage test */
					if(!guid && i == (unsigned int)params.screen)
					{
						guid = formats[n].id;
						bpp = formats[n].bits_per_pixel;
					}
					if(isprint(imageName[0]) && isprint(imageName[1]) &&
						isprint(imageName[2]) && isprint(imageName[3]))
					{
						BLTS_DEBUG(" (%s)\n", imageName);
					}
					else
					{
						BLTS_DEBUG("\n");
					}
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

					if(formats[n].type == XvRGB)
					{
						BLTS_DEBUG("        depth: %i\n", formats[n].depth);

						BLTS_DEBUG("        red, green, blue masks: 0x%x, 0x%x, 0x%x\n",
							formats[n].red_mask,
							formats[n].green_mask,
							formats[n].blue_mask);
					}
				}
				if(formats) XFree(formats);
			}

			XvFreeEncodingInfo(encodings);
		}
	}

	if(xv_port != -1 && guid != 0)
	{
		XShmSegmentInfo	yuv_shminfo;
		Window root_ret;
		int x_ret, y_ret;
		unsigned int w_ret, h_ret, bw_ret, depth_ret, len;
		int yuv_width = 320;
		int yuv_height = 200;
		long frames = 0;

		yuv_image = (XvImage*)XvShmCreateImage(params.display, xv_port,
			guid, 0, yuv_width, yuv_height, &yuv_shminfo);

		len = (yuv_image->height * yuv_image->width * bpp) / 8;

		yuv_shminfo.shmid = shmget(IPC_PRIVATE, yuv_image->data_size,
			IPC_CREAT | 0777);
		yuv_shminfo.shmaddr = yuv_image->data = shmat(yuv_shminfo.shmid, 0, 0);
		yuv_shminfo.readOnly = False;

		if(!XShmAttach(params.display, &yuv_shminfo))
		{
			BLTS_ERROR("XShmAttach failed!\n");
			ret = -1;
			goto cleanup;
		}

		XGetGeometry(params.display, params.window, &root_ret, &x_ret,
			&y_ret, &w_ret, &h_ret, &bw_ret, &depth_ret);

		timing_start();

		while(timing_elapsed() < execution_time)
		{
			for(i = 0; i < len; i++)
			{
				yuv_image->data[i] = random();
			}

			XvShmPutImage(params.display, xv_port, params.window, params.gc,
				yuv_image,
				0, 0, yuv_image->width, yuv_image->height,
				0, 0, w_ret, h_ret, True);

			XFlush(params.display);
			frames++;
		}

		timing_stop();
	}
	else
	{
		BLTS_DEBUG("No suitable adaptor with PutImage operation found.\n");
	}

cleanup:
	if(ainfo)
	{
		XvFreeAdaptorInfo(ainfo);
	}

	if(yuv_image)
	{
		XFree(yuv_image);
	}

	close_window(&params);

	return ret;
}

