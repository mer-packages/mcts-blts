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

#include "stable.h"
#include "GCameraTest.h"

#define MWTS_GSTREAMER_CONF_DEBUG

static GCameraTest* global_gcamera= NULL;

static guint fail_timeout;
static guint video_done_timeout_source;
static guint fps_timeout_source;
static guint capture_start_timeout_source;

static guint sourcepad_probe;
static GstPad* sourcepad;


static GTimer* latency_timer;
static GTimer* fps_timer;
static guint fps_frame_count;
static guint average_last_frames;

static double min_frame_interval;
static double max_frame_interval;
static double fps_elapsed_time;

static guint burst_mode;

static guint my_zoom;

/* Wrappers to enable access to callbacks outside of GCameraTest class */
gboolean GCameraTest_image_capture_done_cb_wrapper(GstElement * camera, GString * fname, gpointer user_data)
{
    return global_gcamera->image_capture_done(camera, fname, user_data);
}

gboolean GstreamerTest_bus_cb_wrapper(GstBus *bus, GstMessage *msg, gpointer data)
{
    return global_gcamera->bus_cb(bus,msg,data);
}

gboolean GstreamerTest_source_buffer_cb_wrapper(GstPad *pad, GstBuffer *buffer, gpointer u_data)
{
    return global_gcamera->source_buffer_cb(pad, buffer, u_data);
}

void GCameraTest_fail_timeout_cb_wrapper()
{
    global_gcamera->fail_timeout_cb();
}

void GCameraTest_video_done_timeout_cb_wrapper()
{
    global_gcamera->video_done_timeout_cb();
}

void GCameraTest_photo_capture_start_cb_wrapper()
{
    global_gcamera->photo_capture_start_cb();
}

GCameraTest::GCameraTest()
{
    MWTS_ENTER;

    flag_capture_done           = 0;
    flag_autofocus              = 0;
    flag_fps_on                 = 0;

    gst_camera_bin              = NULL;
    gst_videosrc                = NULL;
    video_caps_list             = NULL;    
    capture_resolution          = NULL;
    capture_filename_extension  = NULL;    

    local_mainloop              =NULL;
    fail_timeout                =0;
    video_done_timeout_source   =0;
    fps_timeout_source          =0;
    capture_start_timeout_source=0;

    sourcepad_probe             = 0;
    sourcepad                   = NULL;

    latency_timer               = 0;
    fps_timer                   = 0;
    fps_frame_count             = 0;
    average_last_frames         = 0;

    min_frame_interval          =100000;
    max_frame_interval          =0;
    fps_elapsed_time            =0;

    burst_mode                  =0;

    my_zoom                     =100;

    MWTS_LEAVE;
}


GCameraTest::~GCameraTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void GCameraTest::OnInitialize()
{
    MWTS_ENTER;

    recordingVideoFilename = g_pConfig->value("OUTPUT/video_filename").toString();
    recordingImageFilename = g_pConfig->value("OUTPUT/image_filename").toString();
    recordingVideoDir = g_pConfig->value("OUTPUT/video_dir").toString();
    recordingImageDir = g_pConfig->value("OUTPUT/image_dir").toString();
    #ifdef MWTS_GSTREAMER_CONF_DEBUG
        //taking recording output setting from the .conf file
        MWTS_DEBUG("Conf filename location:" + g_pConfig->fileName());
        MWTS_DEBUG("Recording video filename: " + recordingVideoFilename);
        MWTS_DEBUG("Recording image filename: " + recordingImageFilename);
        MWTS_DEBUG("Recording video dir: " + recordingVideoDir);
        MWTS_DEBUG("Recording image dir: " + recordingImageDir);
    #endif
    global_gcamera = this;
    qDebug("Initialized mwts-gcamera");
    g_type_init();
    //gst_init();

    /* initialization */
    GError* err;
    if (FALSE == gst_init_check (NULL, NULL, &err))
    {
        qCritical("gst_init_check failed :%s", err->message);
        g_error_free(err);
        MWTS_LEAVE;
        g_pResult->StepPassed("Initialize", FALSE);
        return;
    }

    cleanup_pipeline();

    capture_resolution = g_string_new("");
    g_string_printf(capture_resolution, "xxx");

    capture_filename_extension = g_string_new("");
    g_string_printf(capture_filename_extension, "xxx");

    /* create camerabin */
    gst_camera_bin = gst_element_factory_make ("camerabin", NULL);

    if (NULL == gst_camera_bin)
    {
        qCritical("gst_camerabin (camerabin) is null");
        MWTS_LEAVE;
        g_pResult->StepPassed("Initialize", FALSE);
        return;
    }
    else
    {
        MWTS_LEAVE;
        g_pResult->StepPassed("Initialize", TRUE);
        return;
    }

}

void GCameraTest::OnUninitialize()
{
    MWTS_ENTER;    

    cleanup_pipeline ();

    MWTS_LEAVE;
}


/**
 * helper function to write current action to latency file
 * calculates time to action
 * @param gchar action 		action what has happened as a pointer
 * @return void
 */
void GCameraTest::latency(gchar* action)
{
    double elapsed;
    elapsed = g_timer_elapsed(latency_timer, NULL);
    //ldx_test_measure(action, elapsed, "seconds");
}


/**
 * Signal handler
 * When image capture happens, this function will be called
 */
gboolean GCameraTest::image_capture_done (GstElement * camera, GString * fname, gpointer user_data)
{
    MWTS_ENTER;
    Q_UNUSED(camera)
    Q_UNUSED(fname)
    Q_UNUSED(user_data)
    latency("Shot2Shot_Latency");
    qDebug("picture taken! Hooray!");
    flag_capture_done = 1;
    gst_element_set_state (gst_camera_bin, GST_STATE_PAUSED);   

    g_main_loop_quit(local_mainloop);
    MWTS_LEAVE;
    if(burst_mode)
        return TRUE;
    else
        return FALSE;
}

/**
 * Camera buffer update callback
 * used to calculate FPS and burst FPS
 * @param GstPad pad	pad in question
 * @param GstBuffer buffer	pointer to current pad buffer
 * @param u_data	user data
 */
gboolean GCameraTest::source_buffer_cb(GstPad *pad, GstBuffer *buffer, gpointer u_data)
{
    MWTS_ENTER;
    Q_UNUSED(pad)
    Q_UNUSED(buffer)
    Q_UNUSED(u_data)
    static double elapsed;
    static double last_time, now;

    if(fps_frame_count == 0)
    {        
        g_timer_start(fps_timer);
        last_time=0;
        qDebug("This is first frame", elapsed);
    }

    else
    {
        now = g_timer_elapsed(fps_timer, NULL);
        elapsed = now-last_time;
        qDebug("Elapsed time between frames: %.3lf sec", elapsed);
        if(elapsed < min_frame_interval)
            min_frame_interval=elapsed;
        if(elapsed > max_frame_interval)
            max_frame_interval=elapsed;

        last_time=now;
        fps_elapsed_time=now;
    }

    fps_frame_count++;

    MWTS_LEAVE;
    return TRUE;
}


/**
 * Stop all, this is failure
 * This function is a callback and callback time is set elsewhere
 */
gboolean GCameraTest::fail_timeout_cb()
{
    MWTS_ENTER;
    MWTS_ERROR("Fail timeout occurred");
    // stop playback
    gst_element_set_state (gst_camera_bin, GST_STATE_NULL);
    g_main_loop_quit(local_mainloop);
    return FALSE;
}

gboolean GCameraTest::photo_capture_start_cb()
{
    g_timer_start(latency_timer);
    g_signal_emit_by_name (gst_camera_bin, "capture-start", 0);
    if(!burst_mode)
    {
        // in single shot mode capture-stop should be emitted at once.
        g_signal_emit_by_name (gst_camera_bin, "capture-stop", 0);
    }

    // this should be single shot timer
    if(capture_start_timeout_source)
    {
        g_source_remove(capture_start_timeout_source);
        capture_start_timeout_source=0;
    }
}


/**
 * Helper for stopping recording
 * timeout for this callback is set elswhere
 */
gboolean GCameraTest::video_done_timeout_cb()
{
    MWTS_ENTER;
    qDebug("Video taken! Hooray!");
    g_signal_emit_by_name (gst_camera_bin, "capture-stop", 0);
    gst_element_set_state (gst_camera_bin, GST_STATE_PAUSED);
    g_main_loop_quit(local_mainloop);
    flag_capture_done = 1;
    MWTS_LEAVE;
    return FALSE;
}

/**
 * Clean up after tests
 * No other functions should be called after this
 */
void GCameraTest::cleanup_pipeline ()
{
    MWTS_ENTER;

    if (gst_camera_bin)
    {
        gst_element_set_state (gst_camera_bin, GST_STATE_NULL);
        gst_element_get_state (gst_camera_bin, NULL, NULL, GST_CLOCK_TIME_NONE);

        if(fps_timer)
        {
            g_timer_destroy (fps_timer);
            fps_timer = NULL;
        }
        if(latency_timer)
        {
            g_timer_destroy (latency_timer);
            latency_timer = NULL;
        }


        g_main_loop_unref (local_mainloop);
        local_mainloop = NULL;
        if(sourcepad_probe)
        {
            gst_pad_remove_data_probe(sourcepad, sourcepad_probe);
            sourcepad_probe=0;
        }

        if(capture_filename_extension)
            g_string_free(capture_filename_extension, TRUE);
        if(capture_resolution)
            g_string_free(capture_resolution, TRUE);
        gst_object_unref (gst_camera_bin);
        gst_camera_bin = NULL;

        g_list_foreach (video_caps_list, (GFunc) gst_caps_unref, NULL);
        g_list_free (video_caps_list);
        video_caps_list = NULL;
    }

    MWTS_LEAVE;
}

/**
 * Helper for reducing state change debug spam
 *
 */
void GCameraTest::debug_print_state_change(int newstate)
{
    static int oldstate = -1;
    if(newstate != oldstate)
    {
        oldstate = newstate;
        switch(newstate)
        {
        case GST_STATE_VOID_PENDING:
            qDebug("State changed to GST_STATE_VOID_PENDING");
            break;
        case GST_STATE_NULL:
            qDebug("State changed to GST_STATE_NULL");
            break;
        case GST_STATE_READY:
            qDebug("State changed to GST_STATE_READY");
            break;
        case GST_STATE_PAUSED:
            qDebug("State changed to GST_STATE_PAUSED");
            break;
        case GST_STATE_PLAYING:
            qDebug("State changed to GST_STATE_PLAYING");
            break;
        default:
            break;
        }
    }
}


/**
 * Parses error messages from message structure
 * @param msg message
 * @return TRUE if success
 */
gboolean GCameraTest::bus_cb (GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus)
    Q_UNUSED(data)
    gchar *debug = NULL;
    GError *err = NULL;
    GstState oldstate = GST_STATE_NULL;
    GstState newstate = GST_STATE_NULL;
    GstState pendingstate = GST_STATE_NULL;

    switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_EOS:
        qDebug("End of stream");
        g_main_loop_quit(local_mainloop);
        break;

    case GST_MESSAGE_ERROR:
        qDebug("GST_MESSAGE_ERROR");
        gst_message_parse_error (msg, &err, &debug);
        qCritical("Gst : %s", err->message);
        qDebug("Error in pipeline %s",debug);
        g_main_loop_quit(local_mainloop);        
        break;

    case GST_MESSAGE_WARNING:
        qDebug("GST_MESSAGE_WARNING");
        gst_message_parse_warning (msg, &err, &debug);
        qCritical("Gst : %s", err->message);
        break;

    case GST_MESSAGE_INFO:
        qDebug("GST_MESSAGE_INFO");        
        gst_message_parse_info (msg, &err, &debug);
        qDebug("Gst info : %s", err->message);
        break;

    case GST_MESSAGE_STATE_CHANGED:
        gst_message_parse_state_changed (msg, &oldstate, &newstate, &pendingstate);
        debug_print_state_change(newstate);
        break;

    case GST_MESSAGE_BUFFERING:
        qDebug("GST_MESSAGE_BUFFERING");
        break;

    default:
        break;
    }

    if(debug)
    {
        g_free (debug);
    }
    if(err)
    {
        g_error_free(err);
    }

    /* always return true or watch is removed */
    return TRUE;
}


/**
 * Sets file metadata for pictures
 *
 */
void GCameraTest::set_metadata (void)
{
    MWTS_ENTER;
    /* for more information about image metadata tags, see:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/tests/icles/metadata_editor.c
     * and for the mapping:
     * http://webcvs.freedesktop.org/gstreamer/gst-plugins-bad/ext/metadata/metadata_mapping.htm?view=co
     */
    //GstTagSetter *setter = GST_TAG_SETTER (gst_camera_bin);
    GTimeVal time = { 0, 0 };
    gchar *date_str, *desc_str;

    g_get_current_time (&time);
    date_str = g_time_val_to_iso8601 (&time);     /* this is UTC */
    desc_str = g_strdup_printf ("picture taken by %s", g_get_real_name ());
    qDebug("METADATA: [%s] [%s]", date_str, desc_str);    

    g_free (date_str);
    g_free (desc_str);
    MWTS_LEAVE;
}

/**
 * First things first
 * And this function should be the very first.
 * Sets up pipeline.
 * @return gboolean pipeline ready / not
 */
gboolean GCameraTest::setup_pipeline ()
{
    MWTS_ENTER;
    GstBus *bus;
    GstCaps *gst_filtercaps;
    latency_timer = g_timer_new();

    //setup_codecs("theoraenc", "oggmux", "pulsesrc", "vorbisenc", "ogg");
////////////////////////////////////
    /* create main loop */
    local_mainloop = g_main_loop_new (NULL, FALSE);
    /*connect image-done to image_capture*/
    g_signal_connect(gst_camera_bin, "image-done", (GCallback) GCameraTest_image_capture_done_cb_wrapper, NULL);
    /*get bus from pipeline*/
    bus = gst_pipeline_get_bus (GST_PIPELINE (gst_camera_bin));
    /*adding watch to bus*/
    local_bus_watch_source = gst_bus_add_watch (bus, GstreamerTest_bus_cb_wrapper, NULL);
    if(!local_bus_watch_source)
    {
        qCritical("Failed to add gst bus watch");
        return FALSE;
    }

    //gst_bus_set_sync_handler (bus, bus_sync_callback, NULL);
    gst_object_unref (bus);

//////////////////set basic properties ///////////////////
    /* create video source GstElement  */
    gst_videosrc = gst_element_factory_make ((const char*)g_pConfig->value("OUTPUT/video_src").toString().toLatin1(), "source");
    //gst_videosrc = gst_element_factory_make ("v4l2src", "source");

        /*check NULL, then give the CAMERA_APP_VIDEOSRC value to video-src*/
    if (gst_videosrc)
    {        
        //g_object_set (G_OBJECT (gst_videosrc), "device", MAIN_CAMERA, NULL);
        g_object_set (G_OBJECT (gst_camera_bin), "video-source", gst_videosrc, NULL);    
        qDebug() << "gst-videosrc is NOT NULL";
    }
    else
    {
        g_object_get (G_OBJECT (gst_camera_bin), "video-source", &gst_videosrc, NULL);
        if (!gst_camera_bin)
            qDebug() << "gst_videosrc is NULL";
    }

////
    /* create filter caps GstCaps  */
    //gst_filtercaps = gst_element_factory_make (CAMERA_FILTER_CAPS, NULL);
    gst_filtercaps = gst_caps_from_string (CAMERA_FILTER_CAPS);

    /*check NULL, then give the CAMERA_FILTER_CAPS value to filter-caps*/
    if (gst_filtercaps)
    {
        g_object_set (G_OBJECT (gst_camera_bin), "filter-caps", gst_filtercaps, NULL);
        qDebug() << "gst_filtercaps is NOT NULL";
    }
    else
    {
        g_object_get (G_OBJECT (gst_camera_bin), "filter-caps", &gst_filtercaps, NULL);
        if (!gst_filtercaps)
            qDebug() << "gst_filtercaps is NULL";
    }

////////////////////////////////////

    if (GST_STATE_CHANGE_FAILURE ==
        gst_element_set_state (gst_camera_bin, GST_STATE_READY))
    {
        qCritical("gst_camera_bin STATE CHANGE --> READY failure");
        goto fail;
    }


    if (GST_STATE_CHANGE_FAILURE ==
        gst_element_set_state (gst_camera_bin, GST_STATE_PAUSED))
    {
        qCritical("gst_camera_bin STATE CHANGE --> PAUSED failure");
        goto fail;
    }
    else
    {
        gst_element_get_state (gst_camera_bin, NULL, NULL, GST_CLOCK_TIME_NONE);
    }

    if (GST_STATE_CHANGE_FAILURE ==
        gst_element_set_state (gst_camera_bin, GST_STATE_PLAYING))
    {
        qCritical("gst_camera_bin STATE CHANGE --> PLAYING failure");
        goto fail;
    }    
    else
    {
        gst_element_get_state (gst_camera_bin, NULL, NULL, GST_CLOCK_TIME_NONE);
    }
    MWTS_LEAVE;
    return TRUE;

fail:
    MWTS_ERROR("Could not create pipeline");
    cleanup_pipeline ();
    MWTS_LEAVE;
    return FALSE;
}

/**
 * Set image Post Processing on
 * @return gboolean success/failure
 *
 */
gboolean GCameraTest::set_pp()
{
    MWTS_ENTER;
    /* Use  image postprocessing element */
    GstElement *ipp = gst_element_factory_make (CAMERA_APP_IMAGE_POSTPROC, NULL);

    if (ipp)
    {
        g_object_set (G_OBJECT (gst_camera_bin), "imagepp", ipp, NULL);
        MWTS_LEAVE;
        return TRUE;
    }
    MWTS_ERROR("Could not create post processing element");
    MWTS_LEAVE;
    return FALSE;
}


/**
 * Set FPS measurements on
 * @return gboolean whether the pad has been hooked or not
 *
 */
gboolean GCameraTest::set_fps()
{
    GstElement* camerasrc = NULL;
    gint i = 0;
    for(i=0; i<4 && camerasrc==NULL; i++)
    {
        sleep(1);
        //camerasrc = gst_bin_get_by_name((GstBin*)gst_camera_bin, "source");
        g_object_get(G_OBJECT (gst_camera_bin), "video-source", &camerasrc, NULL);
    }

    if(!camerasrc)
    {
        qDebug("No source found to measure FPS from");
        return FALSE;

    }
    else
    {
        fps_timer=g_timer_new();
        sourcepad = gst_element_get_pad (camerasrc, "src");
        sourcepad_probe = gst_pad_add_buffer_probe (sourcepad, G_CALLBACK (GstreamerTest_source_buffer_cb_wrapper), NULL);
        flag_fps_on = 1;
        qDebug ("Flag_fps switched on");
    }
    return TRUE;
}

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
void GCameraTest::set_resolution(gint x, gint y, gint fps_h, gint fps_l)
{
    MWTS_ENTER;
    if(fps_l != 0)
        qDebug("setting resolution to h=%i and l=%i, fps_h=%i, fps_l=%i, fps=%.2f", x, y, fps_h, fps_l, (float)(fps_h/fps_l));
    else
        qDebug("setting resolution to h=%i and l=%i", x, y);
    /*g_signal_emit_by_name (gst_camera_bin, "set-video-resolution-fps",
                            image_resolution_label_map[i].width,
                            image_resolution_label_map[i].height, 0);*/
    //or

    g_signal_emit_by_name(gst_camera_bin, "set-video-resolution-fps", x, y, fps_h, fps_l);

    if(capture_resolution != NULL)
    {
        g_string_free(capture_resolution, TRUE);
        capture_resolution = NULL;
    }
    capture_resolution = g_string_new("");
    g_string_printf(capture_resolution, "%ix%i", x, y);
    qDebug("capture resolution set as %s", capture_resolution->str);
    MWTS_LEAVE;
}

/**
 * Take picture
 * Before calling, everything else needs to be set up
 * @param gboolean consecutive whether to use in burst mode or not
 * @return success/failure
 */
gboolean GCameraTest::take_picture(gboolean consecutive)
{
    guint fail_timeout_source = 0;
    if(consecutive)
        burst_mode=1;
    else
        burst_mode=0;

    MWTS_ENTER;
    gst_element_set_state (gst_camera_bin, GST_STATE_PLAYING);

    flag_capture_done = 0;

    ////video file operations
    /*GString * actual_filename = NULL;
    qDebug("creating filename");
    actual_filename=g_string_new("");
    g_string_printf (actual_filename, "%simage_%s_%04u.jpg",
                                    IMAGE_DIR,
                                    capture_resolution->str,
                                    file_nro);
    qDebug("Actual filename is... %s", actual_filename->str);
    file_nro++;
    qDebug("writing file as %s", actual_filename->str);
    */
    QString f = next_output_filename(recordingImageDir,
                                     recordingImageFilename,
                                     ".jpg");
    g_object_set (G_OBJECT (gst_camera_bin), "filename", (const char*) f.toLatin1(), NULL);
    //choosing image recording mode
    g_object_set (gst_camera_bin, "mode", 0, NULL);

    capture_start_timeout_source = g_timeout_add_seconds(CAPTURE_START_AFTER, (gboolean(*)(void*))GCameraTest_photo_capture_start_cb_wrapper, NULL);
    fail_timeout_source = g_timeout_add_seconds(IMG_CAPTURE_TIMEOUT, (gboolean(*)(void*))GCameraTest_fail_timeout_cb_wrapper, NULL);

    g_main_loop_run (local_mainloop);

    if(fail_timeout_source)
    {
        // remove timeouts
        g_source_remove(fail_timeout_source);
        fail_timeout_source=0;
    }

    if(capture_start_timeout_source )
    {
        g_source_remove(capture_start_timeout_source);
        capture_start_timeout_source=0;
    }


    MWTS_LEAVE;
    if(flag_capture_done)
        return TRUE;
    else
        return FALSE;
}

/**
 * Take video
 * Before calling, everything else needs to be set up
 * @param guint video_lenght time in seconds to take video
 * @return success / failure
 */
gboolean GCameraTest::take_video(guint video_length)
{
    guint fail_timeout_source = 0;

    min_frame_interval=100000;
    max_frame_interval=0;
    fps_frame_count=0;

    MWTS_ENTER;
    gst_element_set_state (gst_camera_bin, GST_STATE_PLAYING);
    flag_capture_done = 0;    
    QString f = next_output_filename(recordingVideoDir,
                                     recordingVideoFilename,
                                     QString().fromLatin1(capture_filename_extension->str));
    g_object_set (G_OBJECT (gst_camera_bin), "filename", (const char*) f.toLatin1(), NULL);
    //choosing video recording mode
    g_object_set (gst_camera_bin, "mode", 1, NULL);
    g_signal_emit_by_name (gst_camera_bin, "capture-start", 0);

    video_done_timeout_source = g_timeout_add_seconds(video_length, (gboolean(*)(void*))GCameraTest_video_done_timeout_cb_wrapper, NULL);
    fail_timeout_source = g_timeout_add_seconds(video_length + 20, (gboolean(*)(void*))GCameraTest_fail_timeout_cb_wrapper, NULL);

    g_main_loop_run (local_mainloop);

    if(fail_timeout_source)
    {
        g_source_remove(fail_timeout_source);
        fail_timeout_source=0;
    }
    if(video_done_timeout_source)
    {
        g_source_remove(video_done_timeout_source);
        video_done_timeout_source=0;
    }
    if(flag_fps_on)
    {
        if(fps_timeout_source)
        {
            g_source_remove(fps_timeout_source);
            fps_timeout_source = 0;
        }

        /* elapsed time is measured between first and last frame
         so frame count is actually framecount-1 */

        qDebug("FrameRate ", (double)(fps_frame_count-1)/(double)fps_elapsed_time, "FPS");
        qDebug("MaxFrameInterval ", max_frame_interval*1000.0, "ms");
        qDebug("MinFrameInterval ", min_frame_interval*1000.0, "ms");

    }

    MWTS_LEAVE;
    if(flag_capture_done)
    {
        //gst_object_unref(gst_filename);
        return TRUE;
    }
    else
    {
        //gst_object_unref(gst_filename);
        return FALSE;
    }

}

QString GCameraTest::next_output_filename(QString recordingPath, QString recordingFilename, QString recordingExtension)
{
    int i = recordingFilename.indexOf("?");
    if(i == -1)
    {
        return recordingFilename + recordingExtension;
    }
    else
    {
        int j = recordingFilename.lastIndexOf("?")+1;

        QDir dir(recordingPath);
        QString prefix = recordingFilename.left( i );
        QString postfix = recordingFilename.right( recordingFilename.length() - j );
        QStringList nameFilters;
        nameFilters << ( recordingFilename + recordingExtension );
        QStringList filenames = dir.entryList ( nameFilters );

        int max =- 1;
        foreach(QString filename, filenames)
        {
            QString s = filename.left(j).right( j - i);
            bool ok;
            int n = s.toInt(&ok);

            if( ok  && ( max < n ))
            {
                max = n;
            }
        }

        QString r;
        r.setNum(max+1);

        if(r.length() > ( j - i ))
        {
            r = r.right( j - i );
        }
        else
        {
            while(r.length() < ( j - i ))
            {
                r = "0" + r;
            }
        }
        qDebug() << __PRETTY_FUNCTION__ << "result:" << (recordingPath+prefix + r + postfix + recordingExtension);
        return recordingPath+prefix + r + postfix + recordingExtension;
    }
}

/**
 * Push codecs to pipeline
 * @param gchar * videocodec	video codec to use
 * @param ghcar * muxer			muxer to use
 * @param ghcar * audiosrc		audio source
 * @param ghcar * audiocodec	audio codec
 * @param ghcar * extension		extension of file
 * @return success/failure
 */
gboolean GCameraTest::setup_codecs(gchar *videocodec, gchar *muxer, gchar *audiosrc, gchar *audiocodec, gchar *extension)
{
    MWTS_ENTER;

    qDebug("setting video codecs as videoenc=%s, videomux=%s, audiosrc=%s, audioenc=%s",
                videocodec, muxer, audiosrc, audiocodec);

    GstElement* videoencoder_element = gst_element_factory_make (videocodec, NULL);
    GstElement* audioencoder_element = gst_element_factory_make (audiocodec, NULL);
    GstElement* videomuxer_element = gst_element_factory_make (muxer, NULL);
    GstElement* audiosrc_element = gst_element_factory_make (audiosrc, NULL);    


    qDebug("elements: videoenc %d / audioenc %d / muxer %d / audiosrc %d",
            videoencoder_element,
            audioencoder_element,
            videomuxer_element,
            audiosrc_element);

    if(!videoencoder_element)
    {
        qCritical("Failed to create video encoder element : %s", videocodec);
        return false;
    }

    if(!audioencoder_element)
    {
        qCritical("Failed to create audio encoder element : %s", audiocodec);
        return false;
    }

    if(!videomuxer_element)
    {
        qCritical("Failed to create video muxer element : %s", muxer);
        return false;
    }

    if(!audiosrc_element)
    {
        qCritical("Failed to create audio source element : %s", audiosrc);
        return false;
    }


    g_object_set (G_OBJECT (gst_camera_bin), "video-encoder", videoencoder_element, NULL);
    g_object_set (G_OBJECT (gst_camera_bin), "audio-encoder", audioencoder_element, NULL);
    g_object_set (G_OBJECT (gst_camera_bin), "video-muxer", videomuxer_element, NULL);
    g_object_set (G_OBJECT (gst_camera_bin), "audio-source", audiosrc_element, NULL);



    if(capture_filename_extension != NULL)
    {
        g_string_free(capture_filename_extension, TRUE);
        capture_filename_extension=NULL;
    }
    capture_filename_extension = g_string_new (extension);
    MWTS_LEAVE;
    return TRUE;
}

gboolean GCameraTest::set_flags(GstCameraBinFlags flags)
{
    MWTS_ENTER;
    if (flags!=NULL || flags!=0)
    {
        gint def_flags;
        g_object_get(G_OBJECT (gst_camera_bin), "flags", &def_flags, NULL);
        qDebug() << "DEFAULT FLAGS:" << def_flags;
        /* v4l2 does not have the format what the theoraenc video-encoder
        supports, colorspace conversion is needed */
        def_flags |= flags;
        //flags |= GST_CAMERABIN_FLAG_VIEWFINDER_COLORSPACE_CONVERSION;
        g_object_set (G_OBJECT (gst_camera_bin), "flags", flags, NULL);
        qDebug() << "CHANGED FLAGS:" << flags;
        //g_pResult->StepPassed(__PRETTY_FUNCTION__, TRUE);
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qCritical("The given parameter is not okay.");
        //g_pResult->StepPassed(__PRETTY_FUNCTION__, FALSE);
        MWTS_LEAVE;
        return FALSE;
    }
}

/************************************************************
 *
 * GStreamer photography interface
 *
 */

/**
 * Call autofocus and set it running
 */
void GCameraTest::set_autofocus(void)
{
    gst_photography_set_autofocus(GST_PHOTOGRAPHY(gst_camera_bin), true);
}

/**
 * Set amount of zoom
 * zoom value can be from 100 to 1000
 *
 */
gboolean GCameraTest::set_zoom(guint zoom)
{
    qDebug("setting zoom=%d", zoom);
//	return gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_camera_bin), zoom);
    gboolean ret = gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_videosrc), zoom);
    qDebug("setting zoom=%d returned %d", zoom, ret);
    my_zoom=zoom;			// remember zoom factor
    return ret;
}

gboolean GCameraTest::increase_zoom(guint zoom)
{
    qDebug("increase zoom=%d", zoom);
    zoom += my_zoom;
//	return gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_camera_bin), zoom);
    gboolean ret = gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_videosrc), zoom);
    qDebug("setting zoom=%d returned %d", zoom, ret);
    my_zoom=zoom;			// remember zoom factor
    return ret;
}

gboolean GCameraTest::decrease_zoom(guint zoom)
{
    qDebug("decrease zoom=%d", zoom);
    zoom = my_zoom - zoom;
//	return gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_camera_bin), zoom);
    gboolean ret = gst_photography_set_zoom(GST_PHOTOGRAPHY(gst_videosrc), zoom);
    qDebug("setting zoom=%d returned %d", zoom, ret);
    my_zoom=zoom;			// remember zoom factor
    return ret;
}
/**
 * Set flash mode
 * GST_PHOTOGRAPHY_FLASH_MODE_AUTO = 0,
 * GST_PHOTOGRAPHY_FLASH_MODE_OFF,
 * GST_PHOTOGRAPHY_FLASH_MODE_ON,
 * GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN,
 * GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE
 */
gboolean GCameraTest::set_flashmode(GstFlashMode mode)
{
    qDebug("setting flash mode=%d", mode);
//	return gst_photography_set_flash_mode(GST_PHOTOGRAPHY(gst_camera_bin), mode);
    gboolean ret = gst_photography_set_flash_mode(GST_PHOTOGRAPHY(gst_videosrc), mode);
    qDebug("setting flash mode=%d returned %d", mode, ret);
    return ret;
}

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
gboolean GCameraTest::set_tone_mode(GstColourToneMode tone_mode)
{
    return gst_photography_set_colour_tone_mode(GST_PHOTOGRAPHY(gst_videosrc), tone_mode);
}

/**
 * Set ISO speed
 *
 */
gboolean GCameraTest::set_iso_speed(guint speed)
{
    return gst_photography_set_iso_speed (GST_PHOTOGRAPHY(gst_videosrc), speed);
}

/**
 * Camera white balance mode
 *  GST_PHOTOGRAPHY_WB_MODE_AUTO = 0,
 *  GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT,
 *  GST_PHOTOGRAPHY_WB_MODE_CLOUDY,
 *  GST_PHOTOGRAPHY_WB_MODE_SUNSET,
 *  GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN,
 *  GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT
 */
gboolean GCameraTest::set_wb_mode(GstWhiteBalanceMode mode)
{
    //qDebug("setting white balance=%d", mode);
    //return gst_photography_set_white_balance_mode (GST_PHOTOGRAPHY(gst_camera_bin), mode);
    gboolean ret = gst_photography_set_white_balance_mode (GST_PHOTOGRAPHY(gst_videosrc), mode);
    qDebug("setting  white balance=%d returned %d", mode, ret);
    return ret;
}

/**
 * Set exposure
 *
 */
gboolean GCameraTest::set_exposure(guint exposure)
{
    return gst_photography_set_exposure (GST_PHOTOGRAPHY(gst_videosrc), exposure);
}

/**
 * Set aperture
 *
 */
gboolean GCameraTest::set_aperture(guint aperture)
{
    return  gst_photography_set_aperture (GST_PHOTOGRAPHY(gst_videosrc), aperture);
}

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
gboolean GCameraTest::all_in_one_with_picture()
{
// SetResolution 1280 960
// SetResolution 2048 1536
// SetResolution 2592 1944
 /*   gboolean ret = FALSE;
    int i_resolution_cases = 0;
    for (i_resolution_cases = 0; i_resolution_cases < 3; i_resolution_cases++)
    {
        ret = TRUE;
        switch (i_resolution_cases)
        {
            case 0:
                set_resolution(1280, 960, 0, 0);
                break;
            case 1:
                set_resolution(2048, 1536, 0, 0);
                break;
            case 2:
                set_resolution(2592, 1944, 0, 0);
                break;
            default:
                ret = FALSE;
        }
        if (!ret)
            return ret;


        int i_whitebalence_cases = 0;
        for (i_whitebalence_cases = 0; i_whitebalence_cases < 6; i_whitebalence_cases++)
        {
            ret = set_wb_mode(i_whitebalence_cases);
            if (!ret)
                return ret;


            int i_tone_cases = 0;
            for (i_tone_cases = 0; i_tone_cases < 9; i_tone_cases++)
            {
                ret = set_tone_mode(i_tone_cases);
                if (!ret)
                    return ret;

                int i_flash_cases = 0;
                for (i_flash_cases = 0; i_flash_cases < 5; i_flash_cases++)
                {
                    ret = set_flashmode(i_flash_cases);
                    if (!ret)
                        return ret;

                    int i_zoom_cases = 0;
                    for (i_zoom_cases = 100; i_zoom_cases < 300; i_zoom_cases+=30)
                    {
                        ret = set_flashmode(i_zoom_cases);
                        if (!ret)
                            return ret;
                        // TODO here we can take pictures
                        take_picture(FALSE);
                    }
                }
            }
        }
    }*/
}

