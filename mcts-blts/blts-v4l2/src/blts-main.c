/* blts-main -- command line interface

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

//TODO: few exit() bugs remaining. Use blts-v4l2-cli binary instead if
//      no information from device is needed

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <blts_log.h>
#include <blts_dep_check.h>

#include "timing.h"				/* Timing functions as in peripheral tests */

#include "camera.h"

#define	print_result(test, name) { if(test) printf("%s, PASSED\n", name); else printf("%s, FAILED\n", name); }


io_method      	io              = IO_METHOD_MMAP;
static boolean 			smoke = FALSE;

/**
 * describe usage to user
 */
static void usage (FILE * fp, int argc, char ** argv)
{
    fprintf (fp,
             "Usage: %s [options]\n\n"
             "Options:\n"
             "-d | --device name   Video device name [/dev/video]\n"
    		 "-x | --xres          Set width\n"
    		 "-y | --yres          Set height\n"
    		 "-a | --snap          Take snapshot as snap.jpg\n"
    		 "-t | --stream        Stream data to screen while runnign\n"
             "-m | --mmap          Use memory mapped buffers\n"
             "-r | --read          Use read() calls\n"
             "-u | --userp         Use application allocated buffers\n"
             "-p | --progress      Show progress\n"
    		 "-s | --smoke         Run SMOKE set\n"
             "-h | --help          Print this message\n"
             "",
             argv[0]);
}

static const char short_options [] = "d:hmrupsx:y:ta";

                                     static const struct option
                                                 long_options [] =
{
    { "device",     required_argument,      NULL,           'd'
    },
    { "help",       no_argument,            NULL,           'h' },
    { "mmap",       no_argument,            NULL,           'm' },
    { "read",       no_argument,            NULL,           'r' },
    { "userp",      no_argument,            NULL,           'u' },
    { "progress",   no_argument,            NULL,           'p' },
    { "smoke",      no_argument,            NULL,           's' },
    { "xres",		required_argument,		NULL,           'x' },
    { "yres",		required_argument,		NULL,           'y' },
    { "snap",		no_argument,            NULL,           'a' },
    { "stream",		no_argument,            NULL,           't' },
    { 0, 0, 0, 0 }
};

int
main(int argc, char **argv)
{
    int max_width = 500000;
    int max_height = 500000;
// framebuffer  stuff
    int screen_height = 0;
    int screen_width = 0;
    int fd_framebuffer = -1;
    void *fb_data = 0;
    int retval = 2;


    for (;;)
    {
        int index;
        int c;

        c = getopt_long (argc, argv,
                         short_options, long_options,
                         &index);

        if (-1 == c)
            break;

        switch (c)
        {
        case 0: /* getopt_long() flag */
            break;

        case 'd':
   //         dev_name = optarg;
            break;

        case 'h':
            usage (stdout, argc, argv);
            exit (EXIT_SUCCESS);

        case 'm':
            io = IO_METHOD_MMAP;
            break;

        case 'r':
        	io = IO_METHOD_READ;
            break;

        case 'u':
        	io = IO_METHOD_USERPTR;
            break;

        case 'p':
        	//progress = TRUE;
            break;

        case 's':
        	//smoke = TRUE;
            break;

        case 't':
        	//stream = TRUE;
            break;

        case 'a':
        	//snapshot = TRUE;
            break;

        case 'x':
        	//xresolution=atoi(optarg);
            break;

        case 'y':
        	//yresolution=atoi(optarg);
            break;

        default:
            usage (stderr, argc, argv);
            exit (EXIT_FAILURE);
        }
    }

#if 0
    //*************************************************
    // smoke tests
    if(smoke)
    {
    	printf("Check V4L2 software\n");
    	printf("-------------------\n");
    	if(!depcheck("/usr/lib/tests/blts-v4l2-tests/blts-v4l2-req_file.cfg",1))
    	{
    		printf("V4L2 software check PASSED\n");
    	}
    	else
    	{
    		printf("V4L2 software check FAILED.\n");
    	}
    	printf("-------------------\n");

    	progress = TRUE;
        log_open("./results.csv", 0);
        if( open_device () &&
        	enum_controls(FALSE) &&
        	enum_inputs(FALSE) &&
        	query_capabilites(FALSE) &&
        	enum_formats(FALSE))
        {
        	if(init_device (&max_width, &max_height) &&
        			start_capturing () &&
        			mainloop (10, FALSE) &&
        			stop_capturing () &&
        			uninit_device ())
        	{
        		close_device ();
        		printf("\nSMOKE test passed!\n");
        		return 0;
        	}
        	else
        	{
        		retval = -1;
        		printf("Could not read frames from driver\n");
        	}
        }
        else
        {
        	retval = -2;
        	printf("Could not read information from driver\n");
        }
        log_close();
        close_device ();
        printf("\nSMOKE test FAILED!\n");
        exit (EXIT_FAILURE);
    }
    // *************************************************
    // snapshot
    if(snapshot)
    {
	    if(xresolution > 0 || yresolution > 0 )
	    {
	    	if(xresolution > 0 && yresolution > 0)
	    	{
	    		printf("Trying in %ix%i\n", xresolution, yresolution);
	    		log_open("./results.csv", 0);
	    	    if(!open_device ())
	    	    	retval = -1;

	    	    if(init_device (&xresolution, &yresolution))
	    	    {
	    		    printf("Real resolution is: %ix%i\n", xresolution, yresolution);
	    		    screen_width = xresolution;
	    		    screen_height = yresolution;

	    		    log_print("%ix%i,", screen_width, screen_height);
	    		    print_aspect_ratio(xresolution,yresolution);
	    		    start_capturing ();
	    		    mainloop (50, TRUE);
	    		    do_snapshot (TRUE, xresolution, yresolution, "snap.jpg");
	    		    stop_capturing ();
	    		    uninit_device ();

	    		    if(stream)
	    		    	framebuffer_close(fd_framebuffer, fb_data, screen_width, screen_height);
	    	    }
	    	    log_close();
	    	    close_device ();
	    	    exit (EXIT_SUCCESS);
	    	    return 0;
	    	}
	    	else
	    	{
	    		printf("Both width and height must be given at same time\n");
	    		retval = -1;
	    		exit(EXIT_FAILURE);
	    	}
	    }
	    else
	    {
	    	// take snapshot with maximum resolution...
	    }
    }
    // *************************************************
    // resolution dependable streaming
    if(xresolution > 0 || yresolution > 0 )
    {
    	if(xresolution > 0 && yresolution > 0)
    	{
    		printf("Trying in %ix%i\n", xresolution, yresolution);
    		log_open("./results.csv", 0);
    		log_print("resolution, aspect ratio, loops, time taken, FPS\n");
    	    if(!open_device ())
    	    	retval = -1;

    	    if(init_device (&xresolution, &yresolution))
    	    {
    		    printf("Real resolution is: %ix%i\n", xresolution, yresolution);
    		    screen_width = xresolution;
    		    screen_height = yresolution;

    		    if(stream)
    		    {
    		    	framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
    		    	printf("Framebuffer: %ix%i\n", screen_width, screen_width);

    		    }

    		    log_print("%ix%i,", screen_width, screen_height);
    		    print_aspect_ratio(xresolution,yresolution);
    		    start_capturing ();

    		    if(stream)
    		    	mainloop_stream (LOOPS, TRUE, xresolution, yresolution, screen_width, screen_height, fb_data);
    		    else
    		    	mainloop (LOOPS, TRUE);

    		    stop_capturing ();
    		    uninit_device ();

    		    if(stream)
    		    	framebuffer_close(fd_framebuffer, fb_data, screen_width, screen_height);
    	    }
    	    log_close();
    	    close_device ();
    	    exit (EXIT_SUCCESS);
    	    return 0;
    	}
    	else
    	{
    		printf("Both width and height must be given at same time\n");
    		exit(EXIT_FAILURE);
    		retval = -4;
    	}
    }
    // *************************************************
    // normal testing with different resolutions
    log_open("./results.csv", 0);
    log_print("resolution, aspect ratio, loops, time taken, FPS\n");
    if(!open_device ())
    	retval = -1;

    printf("-----------------------\n");
    printf("Device controls:\n");
    print_result(enum_controls(TRUE), "Camera-Check controls");
    printf("-----------------------\n");
    printf("Device inputs:\n");
    enum_inputs(TRUE);
    printf("-----------------------\n");
    printf("Device capabilities:\n");
    print_result(query_capabilites(TRUE), "Camera-Check capabilites");
    printf("-----------------------\n");
    printf("Device supported formats:\n");
    print_result(enum_formats(TRUE), "Camera-Check formats");
    printf("-----------------------\n");
    printf("Maximum resolution FPS measure\n");
    printf("-----------------------\n");
    if(init_device (&max_width, &max_height))
    {


	    printf("Real resolution is: %ix%i\n", max_width, max_height);
	    screen_width = max_width;
	    screen_height = max_height;

	    if(stream)
	    {
	    	framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
	    	printf("Framebuffer: %ix%i\n", max_width, max_height);
	    }
	    log_print("%ix%i,", max_width, max_height);
	    print_aspect_ratio(max_width,max_height);
	    start_capturing ();

	    if(stream)
	    	mainloop_stream (LOOPS, TRUE, max_width, max_height, screen_width, screen_height, fb_data);
	    else
	    	mainloop (LOOPS, TRUE);

	    stop_capturing ();
	    uninit_device ();

	    if(stream)
	    	framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);

    }
    else
    {
    	retval = -1;
    	printf("Could not open device\n");
    }

    close_device ();
    printf("Custom resolutions: stepping down resolutions and calculating FPS\n");

    int step_size_x, step_size_y, i, x, y;


    step_size_x = max_width / 10;
    step_size_y = max_height / 10;

    for(i=1; i<8; i++)
    {
    	if(stream)
    		framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
    	x = max_width - step_size_x * i;
    	y = max_height - step_size_y * i;
        printf("-----------------------\n");
        printf("%ix%i resolution\n", x, y);
        printf("-----------------------\n");
        if(!open_device ())
        	retval = -1;

        if(init_device (&x, &y))
        {
        	printf("Real resolution is: %ix%i\n", x, y);
        	log_print("%ix%i,", x, y);
        	print_aspect_ratio(x,y);
        	start_capturing ();

        	if(stream)
        		mainloop_stream (LOOPS, TRUE, x, y, screen_width, screen_height, fb_data);
        	else
        		mainloop (LOOPS, TRUE);

        	stop_capturing ();
        	uninit_device ();

        	if(stream)
        		framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);
    	}
        else
        {
        	retval = -1;
        	printf("Could not open device\n");
        }
        close_device ();
    }

    printf("Best known resolutions: gather FPS information from best known resolutions\n");
    if(!open_device ())
    	retval = -1;

    printf("-----------------------\n");
    printf("320x240 FPS measure\n");
    printf("-----------------------\n");
    x = 320;
    y = 240;
    if(init_device (&x, &y))
    {
    	if(stream)
    		framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
	    printf("Real resolution is: %ix%i\n", x, y);
	    log_print("%ix%i,", x, y);
	    print_aspect_ratio(x,y);
	    start_capturing ();
	    if(stream)
	    	mainloop_stream (LOOPS, TRUE, x, y, screen_width, screen_height, fb_data);
	    else
	    	mainloop (LOOPS, TRUE);
	    stop_capturing ();
	    uninit_device ();
	    if(stream)
	    	framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);
    }
    else
    {
    	printf("Could not open device\n");
    }

    close_device ();

    if(!open_device ())
    	retval = -1;

    printf("-----------------------\n");
    printf("848x480 FPS measure\n");
    printf("-----------------------\n");
    x = 848;
    y = 480;
    if(init_device (&x, &y))
    {
    	if(stream)
    		framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
	    printf("Real resolution is: %ix%i\n", x, y);
	    log_print("%ix%i,", x, y);
	    print_aspect_ratio(x,y);
	    start_capturing ();
	    if(stream)
	    	mainloop_stream (LOOPS, TRUE, x, y, screen_width, screen_height, fb_data);
	    else
	    	mainloop (LOOPS, TRUE);
	    stop_capturing ();
	    uninit_device ();
	    if(stream)
	    	framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);
    }
    else
    {
    	printf("Could not open device\n");
    }

    close_device ();

    if(!open_device ())
    	retval = -1;

    printf("-----------------------\n");
    printf("1280x960 FPS measure\n");
    printf("-----------------------\n");
    x = 1280;
    y = 960;
    if(init_device (&x, &y))
    {
    	if(stream)
    		framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
	    log_print("%ix%i,", x, y);
	    print_aspect_ratio(x,y);
	    start_capturing ();
	    if(stream)
	    	mainloop_stream (LOOPS, TRUE, x, y, screen_width, screen_height, fb_data);
	    else
	    	mainloop (LOOPS, TRUE);
	    stop_capturing ();
	    uninit_device ();
	    if(stream)
	    	framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);
    }
    else
    {
    	printf("Could not open device\n");
    }

    close_device ();

    if(!open_device ())
    	retval = -1;
    printf("-----------------------\n");
    printf("2048x1536 FPS measure\n");
    printf("-----------------------\n");
    x = 2048;
    y = 1536;
    if(init_device (&x, &y))
    {
    	if(stream)
    		framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height);
	    log_print("%ix%i,", x, y);
	    printf("Real resolution is: %ix%i\n", x, y);
	    print_aspect_ratio(x,y);
	    start_capturing ();
	    if(stream)
	    	mainloop_stream (LOOPS, TRUE, x, y, screen_width, screen_height, fb_data);
	    else
	    	mainloop (LOOPS, TRUE);
	    stop_capturing ();
	    uninit_device ();
	    if(stream)
	    	framebuffer_close(fd_framebuffer, fb_data, max_width, max_width);
    }
    else
    {
    	retval = -1;
    	printf("Could not open device\n");
    }

    close_device ();
#endif

    log_close();

    exit (EXIT_SUCCESS);
    return retval;
}
