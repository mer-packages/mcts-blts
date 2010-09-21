/* v4l2-cases.c -- MIN case file

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

/* ------------------------------------------------------------------------- */
/* INCLUDE FILES */
#include "v4l2-min.h"
#include "../camera.h"
#include <blts_log.h>

#include <fcntl.h>		/* low-level i/o */
#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

io_method      	io              = IO_METHOD_MMAP;
/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
char *module_date = __DATE__;
char *module_time = __TIME__;

static int camera_width = 0;
static int camera_heigth = 0;

/* ------------------------------------------------------------------------- */
/* EXTERNAL FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* None */
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */
/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* static GLOBAL VARIABLES */
/* None */

/* ------------------------------------------------------------------------- */
/* static CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* static FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* ------------------------------------------------------------------------- */
static int		OpenDevice(MinItemParser * item);
static int		CloseDevice(MinItemParser * item);
static int		Scan(MinItemParser * item);
static int		ReadFrames(MinItemParser * item);
static int		Init(MinItemParser * item);
static int		Close(MinItemParser * item);
static int		Start(MinItemParser * item);
static int		Stop(MinItemParser * item);
static int 		SnapShot (MinItemParser * item);


/* ------------------------------------------------------------------------- */
/* ==================== static FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

/**
 * Init module
 * Opens up log file
 */

static int Init (MinItemParser * item)
{
	log_open("./results.csv", 0);

	return ENOERR;
}

/**
 * Deinit and close
 */

static int Close (MinItemParser * item)
{
    log_close();
	return ENOERR;
}

/**
 * Checks if camera can be opened
 */


static int OpenDevice (MinItemParser * item)
{
	char *device = NULL;
	if(ENOERR != mip_get_next_string(item, &device))
	{
		return EINVAL;
	}

	dev_name = device;

	if(open_device())
		return ENOERR;
	else
		return EINVAL;
}

/**
 * Closes, or tries to, device
 */

static int CloseDevice (MinItemParser * item)
{
	if(close_device())
		return ENOERR;
	else
		return EINVAL;
	return ENOERR;
}

/**
 * Eqvivalent to smoke test
 * Scans controls and capabilities
 */

static int Scan (MinItemParser * item)
{
	if(enum_controls(0) &&
    enum_inputs(0) &&
    query_capabilites(0) &&
    enum_formats(0))
		return ENOERR;
	else
		return EINVAL;
	return ENOERR;
}

/**
 * Actually opens camera
 * in given x and y resolution
 * sets correct x and y values if
 * camera cannot open up with set
 * values
 */

static int Start (MinItemParser * item)
{
	int x, y;
	if(ENOERR != mip_get_next_int(item, &x) ||
	   ENOERR != mip_get_next_int(item, &y))
	{
		return EINVAL;
	}
    if(init_device (&x, &y))
    {
    	camera_heigth = y;
    	camera_width = x;
    	return ENOERR;
    }
    else
    	return EINVAL;
}

/**
 * uninit camera
 */

static int Stop (MinItemParser * item)
{
	if(uninit_device ())
	{
    	camera_heigth = 0;
    	camera_width = 0;

		return ENOERR;
	}
	else
		return EINVAL;
}


/**
 * Read frames from camera
 * either to just to test
 * or to stream them to screen
 */

static int ReadFrames (MinItemParser * item)
{
	int loops;
	char *stream = NULL;
	int ret;
	void *fb_data = 0;
	if(ENOERR != mip_get_next_int(item, &loops))
	{
		return EINVAL;
	}
	ret = mip_get_next_string(item, &stream);
	if(ret != ENOERR)
		stream = NULL;

    if(start_capturing ())
    {
    	if(stream)
    	{
    	    int screen_height = 0;
    	    int screen_width = 0;

    	    int cam_width = camera_width;
    	    int cam_height = camera_heigth;

    	    screen_height = cam_height;
    	    screen_width = cam_width;
    	    log_print("resolution, aspect ratio, loops, time taken, FPS\n");
    	    log_print("%ix%i,", camera_width, camera_heigth);
    	    int fbfd = 0;
	    	if(-1 == (fbfd = framebuffer_open("/dev/fb0", &fb_data, &screen_width, &screen_height)))
	    	{
	    		return EINVAL;
	    	}

	    	mainloop_stream (loops, FALSE, cam_width, cam_height, screen_width, screen_height, fb_data);
	    	stop_capturing ();

	    	framebuffer_close(fbfd, fb_data, screen_width, screen_height);
	    	return ENOERR;

    	}
    	else
    	{
    		log_print("resolution, aspect ratio, loops, time taken, FPS\n");
    		log_print("%ix%i,", camera_width, camera_heigth);

    		if(mainloop (loops, FALSE) &&
    		stop_capturing ())
    			return ENOERR;
    	}
    }
    else
    	return EINVAL;

    return ENOERR;
}


/**
 * Takes picture as JPG
 * picture name must be given as second parameter
 */

static int SnapShot (MinItemParser * item)
{
	char *filename = NULL;
	if(ENOERR != mip_get_next_string(item, &filename))
	{
		return EINVAL;
	}

    if(start_capturing ())
    {
    		log_print("resolution, aspect ratio, loops, time taken, FPS\n");
    		log_print("%ix%i,", camera_width, camera_heigth);

    	    int cam_width = camera_width;
    	    int cam_height =  camera_heigth;
    	    mainloop (50, FALSE);
    	    if(filename!= NULL)
    	    	if(!do_snapshot (FALSE, cam_width, cam_height, filename))
    	    		return EINVAL;
		    if(!stop_capturing ())
		    	return EINVAL;

		    return ENOERR;
    }
    else
    	return EINVAL;
}



static int Controls (MinItemParser * item)
{
	if(enum_controls(FALSE))
		return ENOERR;
	else
		return EINVAL;
}

static int Capabilities (MinItemParser * item)
{
	if(query_capabilites(FALSE))
		return ENOERR;
	else
		return EINVAL;
}

static int Formats (MinItemParser * item)
{
	if(enum_formats(FALSE))
		return ENOERR;
	else
		return EINVAL;
}




/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
int ts_get_test_cases (DLList ** list)
{
        /*
         * Copy this line for every implemented function.
         * * First string is the function name used in TestScripter file.
         * * Second is the actual implementation function.
         */
		ENTRYTC (*list, "Init", Init);
		ENTRYTC (*list, "Close", Close);

        ENTRYTC (*list, "Scan", Scan);
        ENTRYTC (*list, "OpenDevice", OpenDevice);
        ENTRYTC (*list, "CloseDevice", CloseDevice);
        ENTRYTC (*list, "Start", Start);
        ENTRYTC (*list, "Stop", Stop);
        ENTRYTC (*list, "ReadFrames", ReadFrames);
        ENTRYTC (*list, "SnapShot", SnapShot);
        ENTRYTC (*list, "Controls", Controls);
        ENTRYTC (*list, "Capabilities", Capabilities);
        ENTRYTC (*list, "Formats", Formats);
        return ENOERR;

}

/* ------------------------------------------------------------------------- */
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
