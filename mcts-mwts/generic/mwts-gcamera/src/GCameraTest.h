/* GCameraTest.h
 *
 * This file is part of MCTS
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com;
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef _INCLUDED_G_CAMERA_TEST_H
#define _INCLUDED_G_CAMERA_TEST_H

#include <MwtsCommon>
#include <glib-2.0/glib-object.h>
#include <dbus-1.0/dbus/dbus-glib.h>
#include <dbus-1.0/dbus/dbus-glib-lowlevel.h>
///////////////////////////
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

/* Gstreamer includes */
#include <gstreamer-0.10/gst/gst.h>
#include <gstreamer-0.10/gst/interfaces/xoverlay.h>
#include <gstreamer-0.10/gst/interfaces/colorbalance.h>

#define GST_USE_UNSTABLE_API
#include <gstreamer-0.10/gst/interfaces/photography.h>

/*Output dir/file settings*/
#define BASE_DIR		 "/home/user/"
#define DEFAULT_IMAGEDIR "/MyDocs/.images/"
#define DEFAULT_VIDEODIR "/MyDocs/.videos/"

#define VIDEO_DIR "/home/user/MyDocs/.videos/"
#define IMAGE_DIR "/home/user/MyDocs/.images/"

#define FPS_FILE "/var/log/tests/camera-fps.csv"
#define LATENCY_FILE "/var/log/tests/camera-latency.csv"

/* Names of default elements */
//#define CAMERA_APP_VIDEOSRC "v4l2camsrc"
//#define CAMERA_APP_VIDEOSRC "v4l2newcamsrc"
#define CAMERA_APP_VIDEOSRC "v4l2src"
#define CAMERA_APP_IMAGE_POSTPROC "ipp"

/* Device names */
//only option on netbook
#define MAIN_CAMERA "/dev/video0"
#define FRONTAL_CAMERA "/dev/video1"

#define IMG_CAPTURE_TIMEOUT		60
#define CAPTURE_START_AFTER		10
#define CAMERA_FILTER_CAPS "video/x-raw-yuv"

#define DEFAULT_VF_CAPS \
"video/x-raw-yuv, width = (int) 320, height = (int) 240, framerate = (fraction) 1496/100;" \
"video/x-raw-yuv, width = (int) 640, height = (int) 480, framerate = (fraction) 1494/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 2503/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 2988/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 1494/100;" \
"video/x-raw-yuv, width = (int) 720, height = (int) 480, framerate = (fraction) 1494/100"

#define PREVIEW_CAPS \
"video/x-raw-rgb, width = (int) 640, height = (int) 480"

typedef enum
{
    GST_CAMERABIN_FLAG_SOURCE_COLORSPACE_CONVERSION  = 0x00000002,
    GST_CAMERABIN_FLAG_VIEWFINDER_COLORSPACE_CONVERSION = 0x00000004
} GstCameraBinFlags;




/**
 * GCameraTest class
 * Test class for testing gstreamer functionality.
 * Audio and Video playbacks for local and streaming.
 *
*/
class GCameraTest : public MwtsTest
{
	Q_OBJECT
public:

    /**
     * Constructor for GCameraTest class
     */
    GCameraTest();

    /**
     * Destructor for GCameraTest class
     */
    virtual ~GCameraTest();


    /**
     * Overridden functions for GCameraTest class
     * OnInitialize is called before test execution
     */

    void OnInitialize();

    /**
     * Overridden functions for GCameraTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /**
     * First things first
     * And this function should be the very first.
     * Sets up pipeline.
     * @return gboolean pipeline ready / not
     */
    gboolean setup_pipeline ();

    /**
     * Clean up after tests
     * No other functions should be called after this
     */
    void cleanup_pipeline ();

    /**
     * Sets file metadata for pictures
     *
     */
    void set_metadata ();

    /**
     * Set image and/video resolution
     * if video mode then also setup fps_h and fps_l
     * real fps = fps_h / fps_l
     *
     * @param x		image width
     * @param y		image height
     * @param fps_h fps 'high' value
     * @param fps_l fps 'low' value
     */
    void set_resolution(gint x, gint y, gint fps_h, gint fps_l);


    /**
     * Push codecs to pipeline
     * @param gchar * videocodec	video codec to use
     * @param ghcar * muxer			muxer to use
     * @param ghcar * audiosrc		audio source
     * @param ghcar * audiocodec	audio codec
     * @param ghcar * extension		extension of file
     * @return success/failure
     */
    gboolean setup_codecs(gchar *videocodec, gchar *muxer, gchar *audiosrc, gchar *audiocodec, gchar *extension);

    /**
     * Take picture
     * Before calling, everything else needs to be set up
     * @param gboolean consecutive whether to use in burst mode or not
     * @return success/failure
     */
    gboolean take_picture(gboolean consecutive);

    /**
     * Take video
     * Before calling, everything else needs to be set up
     * @param guint video_lenght time in seconds to take video
     * @return success / failure
     */
    gboolean take_video(guint video_length);

    /**
     * Set image Post Processing on
     * @return gboolean success/failure
     *
     */
    gboolean set_pp();


    /**
     * Set FPS measurements on
     * @return gboolean whether the pad has been hooked or not
     *
     */
    gboolean set_fps();

    /**
     * helper function to write current action to latency file
     * calculates time to action
     * @param gchar action 		action what has happened as a pointer
     * @return void
     */
    void latency(gchar* action);


    /**
     * Write raw image buffer to file if found from message
     *
     */
    void handle_element_message (GstMessage * msg);

    /**
     * Helper for reducing state change debug spam
     *
     */
    void debug_print_state_change(int newstate);

    /************************************************************
     *
     * Callbacks
     *
     */
    /**
     * Parses error messages from message structure
     * @param msg message
     * @return TRUE if success
     */
    gboolean bus_cb (GstBus *bus, GstMessage *msg, gpointer data);
    /**
     * Camera buffer update callback
     * used to calculate FPS and burst FPS
     * @param GstPad pad	pad in question
     * @param GstBuffer buffer	pointer to current pad buffer
     * @param u_data	user data
     */
    gboolean source_buffer_cb(GstPad *pad, GstBuffer *buffer, gpointer u_data);

    /**
     * Stop all, this is failure
     * This function is a callback and callback time is set elsewhere
     */
    gboolean fail_timeout_cb();

    gboolean photo_capture_start_cb();

    /**
     * Helper for stopping recording
     * timeout for this callback is set elswhere
     */
    gboolean video_done_timeout_cb();

    /**
     * Signal handler
     * When image capture happens, this function will be called
     */
    gboolean image_capture_done (GstElement * camera, GString * fname, gpointer user_data);


    /************************************************************
     *
     * GStreamer photography interface
     * interfaces/photography
     * This is not part of MeeGo SDK, implementation is commented out in .cpp
     */

    /**
     * set tone from selection of
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NORMAL = 0,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SEPIA,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NEGATIVE,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_GRAYSCALE,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NATURAL,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_VIVID,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_COLORSWAP,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SOLARIZE,
     *  GST_PHOTOGRAPHY_COLOUR_TONE_MODE_OUT_OF_FOCUS
     */
    gboolean set_tone_mode(GstColourToneMode tone_mode);

    /**
     * Set flash mode
     * GST_PHOTOGRAPHY_FLASH_MODE_AUTO = 0,
     * GST_PHOTOGRAPHY_FLASH_MODE_OFF,
     * GST_PHOTOGRAPHY_FLASH_MODE_ON,
     * GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN,
     * GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE
     */
    gboolean set_flashmode(GstFlashMode mode);

    /**
     * Camera white balance mode
     *  GST_PHOTOGRAPHY_WB_MODE_AUTO = 0,
     *  GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT,
     *  GST_PHOTOGRAPHY_WB_MODE_CLOUDY,
     *  GST_PHOTOGRAPHY_WB_MODE_SUNSET,
     *  GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN,
     *  GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT
     */
    gboolean set_wb_mode(GstWhiteBalanceMode mode);

    /**
     * all in one with picture (still images)
     *
     * Camera white balance mode
     * GST_PHOTOGRAPHY_WB_MODE_AUTO = 0,
     * GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT,
     * GST_PHOTOGRAPHY_WB_MODE_CLOUDY,
     * GST_PHOTOGRAPHY_WB_MODE_SUNSET,
     * GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN,
     * GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT
     *
     * set tone from selection of
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NORMAL = 0,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SEPIA,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NEGATIVE,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_GRAYSCALE,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NATURAL,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_VIVID,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_COLORSWAP,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SOLARIZE,
     * GST_PHOTOGRAPHY_COLOUR_TONE_MODE_OUT_OF_FOCUS
     *
     * Set flash mode
     * GST_PHOTOGRAPHY_FLASH_MODE_AUTO = 0,
     * GST_PHOTOGRAPHY_FLASH_MODE_OFF,
     * GST_PHOTOGRAPHY_FLASH_MODE_ON,
     * GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN,
     * GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE
     */
    gboolean all_in_one_with_picture();

    /**
     * Set amount of zoom
     * zoom value can be from 100 to 1000
     *
     */
    gboolean set_zoom(guint zoom);

    /**
     * Set ISO speed
     *
     */
    gboolean set_iso_speed(guint speed);

    /**
     * Set exposure
     *
     */
    gboolean set_exposure(guint exposure);

    /**
     * Set aperture
     *
     */
    gboolean set_aperture(guint aperture);

    gboolean increase_zoom(guint zoom);
    gboolean decrease_zoom(guint zoom);

    /**
     * Call autofocus and set it running
     */
    void set_autofocus();

public:
    /* states:
    (image) <---> (video_stopped) <---> (video_recording)
    */
    typedef enum _tag_CaptureState
    {
        CAP_STATE_IMAGE,
        CAP_STATE_VIDEO_STOPPED,
        CAP_STATE_VIDEO_PAUSED,
        CAP_STATE_VIDEO_RECORDING,
    } CaptureState;


    struct
    {
        guint width;
        guint height;
        guint fps_h;
        guint fps_l;
    } current_resolution;


    guint flag_picture_done;
    guint flag_autofocus;

    GstElement *gst_camera_bin;
    GstElement *gst_videosrc;

    GList *video_caps_list;
    GString *capture_filename;
    GString *capture_filename_extension;
    guint file_nro;
    GString *capture_resolution;

    guint flag_capture_done;
    guint flag_fps_on;

    GMainLoop* local_mainloop;

    gint local_bus_watch_source;
};

#endif //#ifndef _INCLUDED_G_CAMERA_TEST_H
