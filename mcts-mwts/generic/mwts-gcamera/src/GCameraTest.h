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
#include <QDir>
#include <string.h>

/* GLib includes */
#include <glib.h>
#include <glib-2.0/glib-object.h>
#include <glib/gstdio.h>
#include <dbus-1.0/dbus/dbus-glib.h>
#include <dbus-1.0/dbus/dbus-glib-lowlevel.h>

/* Gstreamer includes */
#include <gstreamer-0.10/gst/gst.h>
#include <gstreamer-0.10/gst/interfaces/xoverlay.h>
#include <gstreamer-0.10/gst/interfaces/colorbalance.h>

#define GST_USE_UNSTABLE_API
#include <gst/interfaces/photography.h>
#include <gst/interfaces/xoverlay.h>
#include <gst/interfaces/colorbalance.h>

/* Names of default elements */

//the identity does nothing, but demonstrate the postprocessing
#define CAMERA_APP_IMAGE_POSTPROC "identity"
#define CAMERA_APP_VIDEO_POSTPROC "identity"

#define IMG_CAPTURE_TIMEOUT		60
#define CAPTURE_START_AFTER		1
//important to set fourcc for the sake v4l2src on netbook
#define CAMERA_FILTER_CAPS "video/x-raw-yuv, format=(fourcc)I420, width = (int) 320, height = (int) 240"

#define DEFAULT_VF_CAPS \
"video/x-raw-yuv, width = (int) 320, height = (int) 240, framerate = (fraction) 1496/100;" \
"video/x-raw-yuv, width = (int) 640, height = (int) 480, framerate = (fraction) 1494/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 2503/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 2988/100;" \
"video/x-raw-yuv, width = (int) 800, height = (int) 480, framerate = (fraction) 1494/100;" \
"video/x-raw-yuv, width = (int) 720, height = (int) 480, framerate = (fraction) 1494/100"

typedef enum
{
    GST_CAMERABIN_FLAG_SOURCE_RESIZE = 0x00000001,
    //when video format of the driver and the camerabin do not match, it has to be set
    //it is expensive regarding the CPU cost
    GST_CAMERABIN_FLAG_SOURCE_COLORSPACE_CONVERSION  = 0x00000002,
    GST_CAMERABIN_FLAG_VIEWFINDER_COLORSPACE_CONVERSION = 0x00000004,
    GST_CAMERABIN_FLAG_VIEWFINDER_SCALE = 0x00000008,
    GST_CAMERABIN_FLAG_AUDIO_CONVERSION = 0x00000010,
    GST_CAMERABIN_FLAG_DISABLE_AUDIO = 0x00000020,
    GST_CAMERABIN_IMAGE_COLORSPACE_CONVERSION = 0x00000040
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
     * Set video resolution
     * it can also setup fps_h and fps_l
     * real fps = fps_h / fps_l
     *
     * @param x		video width
     * @param y		video height
     * @param fps_h fps 'high' value
     * @param fps_l fps 'low' value
     */
    void set_video_resolution(gint x, gint y, gint fps_h, gint fps_l);

    /**
     * Set image resolution
     *
     * @param x		image width
     * @param y		image height
     */
    void set_image_resolution(gint x, gint y);


    /**
     * Push codecs to pipeline
     * @param gchar * videocodec	video codec to use
     * @param ghcar * muxer			muxer to use
     * @param ghcar * audiosrc		audio source
     * @param ghcar * audiocodec	audio codec
     * @param ghcar * extension		extension of file
     * @return success/failure
     */
    gboolean setup_codecs(const gchar *videocodec, const gchar *muxer, const gchar *audiosrc, const gchar *audiocodec, const gchar *extension);

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
    gboolean set_image_pp();

    /**
     * Set video Post Processing on
     * @return gboolean success/failure
     *
     */
    gboolean set_video_pp();


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

    gboolean set_flags (GstCameraBinFlags flags);

private:
    /**
    * Generates an incremental filename????, which helps to avoid overwriting
    * previously saved video/image filenames.
    * @return next available filename
    * @example 1. video0000.ogg, after this functin call 2. video0001.ogg...
    */
    QString next_output_filename(QString recordingPath, QString recordingFilename, QString recordingExtension);

public:    
    /* Main objects for video/image capture*/
    GstElement *gst_camera_bin;
    GstElement *gst_videosrc;   
    GstCaps *gst_filtercaps;
    GstBus *local_bus;
    GMainLoop* local_mainloop;
    gint local_bus_watch_source;


    /* Variables for customization */
    GList *video_caps_list;
    GString *capture_resolution;

    guint flag_capture_done;
    guint flag_fps_on;
    guint flag_picture_done;
    guint flag_autofocus;
    bool  flag_take_video;

    struct
    {
        guint width;
        guint height;
        guint fps_h;
        guint fps_l;
    } current_resolution;

    /* Output recording variables */
    //String holds recording directory's path
    QString recordingVideoDir;
    QString recordingImageDir;
    //String holds recording file's name
    //for example: "recorded_audio????", the question marks mean the sequence number
    QString recordingVideoFilename;
    QString recordingImageFilename;
    QString recordingFileExtension;    
    GString *capture_filename_extension;    
    QString model;
};

#endif //#ifndef _INCLUDED_G_CAMERA_TEST_H

