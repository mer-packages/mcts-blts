/*
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
#include "gstreamer.h"


#define VOLUME_CHANGE_TIMEOUT 2
#define VOLUME_CHANGE_STEP 0.05
#define MWTS_GSTREAMER_CONF_DEBUG

static GstreamerTest* global_gstreamer= NULL;

/* Wrappers to enable access to callbacks outside of GstreamerTest class */
gboolean GstreamerTest_set_volume_level_cb_wrapper()
{
    return global_gstreamer->set_volume_level_cb();
}

void GstreamerTest_fail_timeout_cb_wrapper()
{
    qDebug() << "GstreamerTest_fail_timeout_cb_wrapper";
    global_gstreamer->fail_timeout_cb();
}

void GstreamerTest_duration_timeout_cb_wrapper()
{
    global_gstreamer->duration_timeout_cb();
}

void GstreamerTest_seeking_timeout_cb_wrapper()
{
    global_gstreamer->seeking_timeout_cb();
}

void GstreamerTest_event_timeout_cb_wrapper()
{
    global_gstreamer->event_timeout_cb();
}

void GstreamerTest_second_event_timeout_cb_wrapper()
{
    global_gstreamer->second_event_timeout_cb();
}

gboolean GstreamerTest_bus_cb_wrapper(GstBus *bus, GstMessage *msg, gpointer data)
{
    return global_gstreamer->bus_cb(bus,msg,data);
}

void GstreamerTest_sink_data_cb_wrapper(/* GstPad *pad, GstBuffer *buffer, gpointer u_data */)
{
    global_gstreamer->sink_data_cb(/*bus,buffer,u_data*/);
}

/**
 * Constructor for template test class
 */
GstreamerTest::GstreamerTest() 
{
    MWTS_ENTER;
    local_bus = NULL;
    local_pipeline = NULL;
    local_playbin = NULL;
    local_mainloop = NULL;
    local_fail_timeout_occurred=FALSE;
    local_duration_timeout_occurred=FALSE;
    local_seeking_enabled=FALSE;
    local_event_enabled=FALSE;
    local_second_event_enabled=FALSE;
    local_framerate_measurement_enabled=FALSE;
    local_change_volume_level=FALSE;

    local_fail_timeout_source=0;
    local_duration_timeout_source=0;
    local_seeking_timeout_source=0;
    local_event_timeout_source=0;
    local_second_event_timeout_source=0;
    local_change_volume_timeout_source=0;
    local_seeking_position=0;
    local_bus_watch_source=0;
    local_timer = NULL;
    local_frame_count = 0;
    local_elapsed_time = 0;
    local_event_type = 0;
    local_second_event_type = 0;
    m_bMeasureFpsPerformance = FALSE;    

    /* These are user-given settings for timeouts with default values */
    local_fail_timeout_setting = 0;
    local_duration_timeout_setting = 0;
    local_seeking_timeout_setting = 10; /* make seek after 10 sec */
    local_event_timeout_setting = 15;
    local_second_event_timeout_setting = 20;

    MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
GstreamerTest::~GstreamerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void GstreamerTest::OnInitialize()
{
    MWTS_ENTER;

    local_bus=NULL;
    local_playbin=NULL;
    g_type_init();

    remove_timeouts();
    global_gstreamer = this;

    if(TRUE != create_pipeline())
    {
        qCritical() << "Failed to create pipeline";
    }
    else
    {
        qDebug() << "Pipeline created" ;
    }
    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void GstreamerTest::OnUninitialize()
{
    MWTS_ENTER;

    gst_element_set_state (local_pipeline, GST_STATE_NULL);
    gst_object_unref (local_bus);
    local_bus=NULL;
    gst_object_unref (local_pipeline);
    local_pipeline=NULL;

    g_main_loop_unref (local_mainloop);
    local_mainloop=NULL;
    remove_timeouts();


    if(local_timer)
    {
        g_timer_destroy (local_timer);
        local_timer=NULL;
    }

    MWTS_LEAVE;
}


void GstreamerTest::EnableFPSPerformance()
{
    m_bMeasureFpsPerformance=TRUE;
}

/** helper for quitting mainloop */
void GstreamerTest::stop_test_internal()
{
    g_main_loop_quit(local_mainloop);
}

/** helper for stopping playback */
gboolean GstreamerTest::fail_timeout_cb()
{
    MWTS_ENTER;
    qDebug("Fail timeout occurred");
    /* stop playback */
    gst_element_set_state (local_pipeline, GST_STATE_NULL);

    stop_test_internal();
    local_fail_timeout_occurred=TRUE;
    return FALSE;
}

/** helper for stopping playback */
gboolean GstreamerTest::duration_timeout_cb()
{
    MWTS_ENTER;
    qDebug("Duration timeout occurred");
    /* stop playback */
    gst_element_set_state (local_pipeline, GST_STATE_NULL);

    stop_test_internal();
    local_duration_timeout_occurred=TRUE;
    return FALSE;
}

/**
  Callback function performing seeking.
  @return TRUE if success, FALSE if not
*/
gboolean GstreamerTest::seeking_timeout_cb()
{	
    MWTS_ENTER;
    qDebug("Seeking timeout occurred");
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current_position = 0;
    GstElement* sink = NULL;
    sink = gst_bin_get_by_name((GstBin*)local_playbin, "videosink");
    if(!sink)
    {
        qDebug("No videosink found, using audiosink");
        sink = gst_bin_get_by_name((GstBin*)local_playbin, "audiosink");
        if(!sink)
        {
            qDebug("No audiosink found");
            return FALSE;
        }
    }

    qDebug("Using seek position %lld", local_seeking_position );
    if (!gst_element_seek_simple (sink, GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH, local_seeking_position))
    {
        qCritical("Failed to seek");
        MWTS_LEAVE;
        return FALSE;
    }
    if (gst_element_query_position (sink, &fmt, &current_position))
    {
        qDebug("Failed to query position");
    }
    else
    {
        qDebug ("Current position: %lld",current_position);
    }

    if(local_seeking_timeout_source)
    {
        qDebug ("Removing timeout");
        g_source_remove(local_seeking_timeout_source);
        local_seeking_timeout_source=0;
    }
    MWTS_LEAVE;

    return TRUE;
}

/**
  Callback function performing (first) predetermined event,
  pause, continue, seek and seeking to end of file
  @return TRUE if success, FALSE if not
*/
gboolean GstreamerTest::event_timeout_cb()
{	
    MWTS_ENTER;
    qDebug("Event timeout occurred");

    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current_position = 0;
    GstElement* sink = NULL;
    sink = gst_bin_get_by_name((GstBin*)local_playbin, "videosink");
    if(!sink)
    {
        qDebug("No videosink found, using audiosink");
        sink = gst_bin_get_by_name((GstBin*)local_playbin, "audiosink");
        if(!sink)
        {
            qDebug("No audiosink found");
            return FALSE;
        }
    }

    if ( MWTS_GST_EVENT_PAUSE == local_event_type)
    {
        qDebug("Setting state GST_STATE_PAUSED");
        gst_element_set_state (sink, GST_STATE_PAUSED);
    }

    if ( MWTS_GST_EVENT_CONTINUE == local_event_type)
    {
        qDebug("Setting state GST_STATE_PLAYING");
        gst_element_set_state (sink, GST_STATE_PLAYING);
    }

    if (MWTS_GST_EVENT_SEEK == local_event_type)
    {
        qDebug("Using seek position %lld",local_seeking_position );
        if (!gst_element_seek_simple (sink,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,local_seeking_position))
        {
            qCritical("Failed to seek");
            MWTS_LEAVE;
            return FALSE;
        }
    }

    if (MWTS_GST_EVENT_SEEK_TO_END == local_event_type)
    {
        gint64 end_position = 0;
        if (!gst_element_query_duration (sink, &fmt, &end_position))
        {
            qCritical("Unable to find end position");
            MWTS_LEAVE;
            return FALSE;
        }
        qDebug("Using seek to end position %lld",end_position );

        if (!gst_element_seek_simple (sink,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,end_position))
        {
            qCritical("Failed to seek to end");
            MWTS_LEAVE;
            return FALSE;
        }
    }

    if (gst_element_query_position (sink, &fmt, &current_position))
    {
        qDebug("Failed to query position");
    }
    else
    {
        qDebug ("Current position: %lld",current_position);
    }

    if(local_event_timeout_source)
    {
        g_source_remove(local_event_timeout_source);
        local_event_timeout_source=0;
    }

    MWTS_LEAVE;
    return TRUE;
}

/**
  Callback function performing second predetermined event,
  pause, continue, seek.  
  @return TRUE if success, FALSE if not
*/
gboolean GstreamerTest::second_event_timeout_cb()
{	
    MWTS_ENTER;
    qDebug("Second event timeout occurred");

    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current_position = 0;

    GstElement* sink = NULL;
    sink = gst_bin_get_by_name((GstBin*)local_playbin, "videosink");
    if(!sink)
    {
        qDebug("No videosink found, using audiosink");
        sink = gst_bin_get_by_name((GstBin*)local_playbin, "audiosink");
        if(!sink)
        {
            qDebug("No audiosink found");
            return FALSE;
        }
    }
    if (MWTS_GST_EVENT_PAUSE == local_second_event_type)
    {
        qDebug("Setting state GST_STATE_PAUSED");
        gst_element_set_state (sink, GST_STATE_PAUSED);
    }

    if (MWTS_GST_EVENT_CONTINUE == local_second_event_type)
    {
        qDebug("Setting state GST_STATE_PLAYING");
        gst_element_set_state (sink, GST_STATE_PLAYING);
    }

    if (MWTS_GST_EVENT_SEEK == local_second_event_type)
    {
        qDebug("Using seek position %lld",local_seeking_position );
        if (!gst_element_seek_simple (sink,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,local_seeking_position))
        {
            qCritical("Failed to seek");
            MWTS_LEAVE;
            return FALSE;
        }
    }

    if (gst_element_query_position (sink, &fmt, &current_position))
    {
        qDebug ("Failed to query position");
    }
    else
    {
        qDebug ("Current position: %lld",current_position);
    }

    if(local_second_event_timeout_source)
    {
        g_source_remove(local_second_event_timeout_source);
        local_second_event_timeout_source=0;
    }
    MWTS_LEAVE;
    return TRUE;
}

/**
  Callback for setting volume level during playback

  Function is called when timeout for changing volume has passed 
  and sets itself new timeout. Volume level (values between 1.0 - 0.0) is adjusted
  first to decrease to zero, then to increase to full 1.0 and then exit playback.
  @return TRUE if success, FALSE if not
*/

gboolean GstreamerTest::set_volume_level_cb()
{
    MWTS_ENTER;

    /* Remove timeout first, in case if playback is terminated */
    if(local_change_volume_timeout_source)
    {
        g_source_remove(local_change_volume_timeout_source);
        local_change_volume_timeout_source=0;
    }

    /* When volume level gets to zero, time to increase volume level */
    if (local_volume_level < 0.0)
    {
        local_volume_level = 0.0;
        local_volume_level_decreasing = FALSE;
    }

    /* when volume over 100% the test has run its course */
    if (local_volume_level > 1 )
    {
        stop_test_internal();
    }

    qDebug("Setting volume level %f", local_volume_level);

    /* manipulating volume element */
    g_object_set (G_OBJECT (local_playbin), "volume", local_volume_level , NULL);

    /* increases and decreases volume level depending which way it should go */
    if (local_volume_level_decreasing)
    {
        local_volume_level = local_volume_level - VOLUME_CHANGE_STEP;
    }
    else
    {
        local_volume_level = local_volume_level + VOLUME_CHANGE_STEP;
    }

    /* Create new timeout */
    local_change_volume_timeout_source = g_timeout_add_seconds_full(G_PRIORITY_HIGH, VOLUME_CHANGE_TIMEOUT , (gboolean(*)(void*))GstreamerTest_set_volume_level_cb_wrapper, NULL, NULL);

    MWTS_LEAVE;
    return FALSE;
}



/** helper for lessening state change debug spam */
void GstreamerTest::debug_print_state_change(int newstate)
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
  Parses error messages from message structure
  @param msg message
  @return TRUE if success
*/
gboolean GstreamerTest::bus_cb (__attribute__((unused)) GstBus *bus, GstMessage *msg, __attribute__((unused)) gpointer data)
{
    //GMainLoop *loop = (GMainLoop*)data;
    gchar *debug = NULL;
    GError *err = NULL;
    GstState oldstate = GST_STATE_NULL;
    GstState newstate = GST_STATE_NULL;
    GstState pendingstate = GST_STATE_NULL;

    switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_EOS:
        qDebug("End of stream");
        stop_test_internal();
        break;

    case GST_MESSAGE_ERROR:
        qDebug("GST_MESSAGE_ERROR");
        gst_message_parse_error (msg, &err, &debug);
        qCritical("Gst : %s", err->message);
        qDebug("%s",debug);
        stop_test_internal();
        break;

    case GST_MESSAGE_WARNING:
        qDebug("GST_MESSAGE_WARNING");
        gst_message_parse_warning (msg, &err, &debug);
        qDebug("Gst : %s", err->message);
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

/** callback called when new frame in sink pad
used for framerate measurement
*/

gboolean GstreamerTest::sink_data_cb()
{
    static double last_time, now;

    if(local_frame_count == 0)
    {
        //this is first frame
        g_timer_start(local_timer);
        last_time=0;
    }
    else
    {
        now=g_timer_elapsed(local_timer, NULL);
        qDebug("Elapsed time between frames: %.3lf sec", now-last_time);
        last_time=now;
        local_elapsed_time=now;
    }
    local_frame_count++;

    return TRUE;
}


/** Adjust failure timeout setting (actual timers are only touched in run_pipeline()
  @param new_timeout_seconds timeout for testcase failure.
  @return 
*/
void GstreamerTest::set_fail_timeout(gint new_timeout_seconds)
{
    MWTS_ENTER;;
    local_fail_timeout_setting = new_timeout_seconds;
    MWTS_LEAVE;
    return;
}

/** 
  Adjust duration timeout setting (actual timers are only touched in run_pipeline()
  @param new_duration_seconds duration of testcase.
  @return 
*/
void GstreamerTest::set_duration(gint new_duration_seconds)
{
    MWTS_ENTER;
    local_duration_timeout_setting = new_duration_seconds;
    MWTS_LEAVE;
    return;
}

/**
  Sets playback to given position.
  @param seek_position desired position given in nanoseconds.
  @return 
*/
void GstreamerTest::set_seek_position (gint64 seek_position)
{
    MWTS_ENTER;
    local_seeking_position = seek_position;
    local_seeking_enabled = TRUE;
    MWTS_LEAVE;
    return;
}

/**
  Sets Timeout when seeking is done
  @param seek_timeout_seconds seek timeout
  @return
*/
void GstreamerTest::set_seek_timeout (gint seek_timeout_seconds)
{
    MWTS_ENTER;
    local_seeking_timeout_setting = seek_timeout_seconds;
    MWTS_LEAVE;
    return;
}

/**
  Sets flag on for framerate measurements
  @return
*/
void GstreamerTest::set_enable_framerate_measurement ()
{
    MWTS_ENTER;
    local_framerate_measurement_enabled = TRUE;
    MWTS_LEAVE;
    return;
}

/**
  Sets values for playback manipulation event.
  @param event_type what kind of event is performed to playback
  @param event_timeout_seconds timestamp when event occurs
  @return
*/
void GstreamerTest::set_event_timeout(gint event_type, gint event_timeout_seconds)
{
    MWTS_ENTER;
    local_event_timeout_setting = event_timeout_seconds;
    local_event_type = event_type;
    qDebug("Local_event_type = %d", local_event_type );
    local_event_enabled = TRUE;
    MWTS_LEAVE;
    return;
}


/**
  Sets values for second playback manipulation event.
  @param second_event_type what kind of event is performed to playback
  @param second_event_timeout_seconds timestamp when event occurs
  @return
*/
void GstreamerTest::set_second_event_timeout(gint second_event_type, gint second_event_timeout_seconds)
{
    MWTS_ENTER;
    local_second_event_timeout_setting = second_event_timeout_seconds;
    local_second_event_type = second_event_type;
    local_second_event_enabled = TRUE;
    MWTS_LEAVE;
    return;
}

/** Sets timeouts to g_timer

*/
void GstreamerTest::set_timeouts()
{
    remove_timeouts();
    if(local_fail_timeout_setting > 0)
    {
        qDebug() << "SETTING LOCAL FAIL TIMEOUT" << local_fail_timeout_setting;
        local_fail_timeout_source = g_timeout_add_seconds_full(G_PRIORITY_HIGH, local_fail_timeout_setting,  (gboolean(*)(void*))GstreamerTest_fail_timeout_cb_wrapper, NULL, NULL);
    }
    if(local_duration_timeout_setting > 0)
    {
        local_duration_timeout_source = g_timeout_add_seconds(local_duration_timeout_setting,  (gboolean(*)(void*))GstreamerTest_duration_timeout_cb_wrapper, NULL);
    }
    if(local_seeking_enabled == TRUE)
    {
        local_seeking_timeout_source = g_timeout_add_seconds(local_seeking_timeout_setting,  (gboolean(*)(void*))GstreamerTest_seeking_timeout_cb_wrapper, NULL);
    };
    if(local_event_enabled == TRUE)
    {
        local_event_timeout_source = g_timeout_add_seconds(local_event_timeout_setting,  (gboolean(*)(void*))GstreamerTest_event_timeout_cb_wrapper, NULL);
    }
    if(local_second_event_enabled == TRUE)
    {
        local_second_event_timeout_source = g_timeout_add_seconds(local_second_event_timeout_setting,  (gboolean(*)(void*))GstreamerTest_second_event_timeout_cb_wrapper, NULL);
    }
    return;
}

/** Cleans timeout settings

*/
void GstreamerTest::remove_timeouts()
{
    if(local_fail_timeout_source)
    {
        g_source_remove(local_fail_timeout_source);
        local_fail_timeout_source=0;
    }
    if(local_duration_timeout_source)
    {
        g_source_remove(local_duration_timeout_source);
        local_duration_timeout_source=0;
    }

    if(local_seeking_timeout_source)
    {
        g_source_remove(local_seeking_timeout_source);
        local_seeking_timeout_source=0;
    }

    if(local_event_timeout_source)
    {
        g_source_remove(local_event_timeout_source);
        local_event_timeout_source=0;
    }
    if(local_second_event_timeout_source)
    {
        g_source_remove(local_second_event_timeout_source);
        local_second_event_timeout_source=0;
    }
    return;
}


/**
* Writes to log any errors or warnings in gst log cache
* @return 
*/

void GstreamerTest::log_errors()
{
    MWTS_ENTER;
    GstMessage *msg;
    GError *err = NULL;
    /* check if there is an error message with details on the bus */

    while(NULL != (msg = gst_bus_pop_filtered(local_bus, GST_MESSAGE_ERROR /* \ GST_MESSAGE_WARNING */ )))
    {
        if(GST_MESSAGE_ERROR == GST_MESSAGE_TYPE(msg))
        {
            gst_message_parse_error(msg, &err, NULL);
            qCritical("%s",err->message);
        }
        if(GST_MESSAGE_WARNING == GST_MESSAGE_TYPE(msg))
        {
            gst_message_parse_warning(msg, &err, NULL);
            qDebug("%s",err->message);
        }
        g_error_free (err);
        gst_message_unref (msg);
    }
    return;
}



/**
* Creates gstreamer pipeline with playbin2 element in it
* @return TRUE if success, FALSE if not
*/
gint GstreamerTest::create_pipeline()
{
    MWTS_ENTER;
    GError* err;

    /* initialization */
    if (FALSE == gst_init_check (NULL, NULL, &err))
    {
        qDebug("gst_init_check failed :%s", err->message);
        g_error_free(err);
        return FALSE;
    }

    /* create main loop */
    local_mainloop = g_main_loop_new (NULL, FALSE);

    /* create elements */
    local_pipeline = gst_pipeline_new ("test_pipeline");
    if(!local_pipeline)
    {
        qCritical("Unable to create gstreamer pipeline");
        return FALSE;
    }
    qDebug("pipeline created");

    local_bus = gst_pipeline_get_bus (GST_PIPELINE (local_pipeline));

    local_bus_watch_source = gst_bus_add_watch (local_bus,GstreamerTest_bus_cb_wrapper, local_mainloop);
    if(!local_bus_watch_source)
    {
        qCritical("Failed to add gst bus watch");
        return FALSE;
    }

    local_playbin  = gst_element_factory_make ("playbin2", "playbin");

    if (!local_playbin)
    {
        qCritical("Could not create element playbin2");
        return FALSE;
    }
    qDebug("playbin2 created");

    /* add playbin to pipeline */
    gst_bin_add (GST_BIN (local_pipeline), local_playbin);

    local_timer=g_timer_new();
    if(!local_pipeline)
    {
        qCritical("gstreamer pipeline disappeared");
        return FALSE;
    }
    return TRUE;

}

/**
  Helper function for giving media file location to playbin element
  @param uri 
  @return TRUE if success
*/
gboolean GstreamerTest::set_source_uri(const char* protocol, const char* uri)
{
    MWTS_ENTER;
    if(!local_pipeline)
    {
        qCritical("Not found local pipeline");
        return FALSE;
    }

    if (protocol == NULL)
    {
        qCritical("No protocol set");
        return FALSE;
    }
\
    QString protocolPath = g_pConfig->value("Data/" + QString(protocol)).toString();
    #ifdef MWTS_GSTREAMER_CONF_DEBUG
        qDebug() << "[protocol]  " << protocol;
        qDebug() << "Config file path: " << g_pConfig->fileName();
        qDebug() << "Data/file " << g_pConfig->value("Data/file").toString();
        qDebug() << "Data/rtsp " << g_pConfig->value("Data/rstp").toString();
        qDebug() << "Data/http " << g_pConfig->value("Data/http").toString();
        qDebug() << "[protocolPath]  " << protocolPath;
    #endif

    if (protocolPath.size() <= 0)
    {
        qCritical() << "Invalid protocol path defined in the config for " << protocol;
        return FALSE;
    }

    // Check if there isn't / mark in the end
    if ( protocolPath[protocolPath.size()-1] != '/' )
    {
        protocolPath += "/";
    }

    protocolPath += uri;

    qDebug() << "Uri: " << protocolPath;

    g_object_set (G_OBJECT (local_playbin), "uri",(const char*) protocolPath.toLatin1(), NULL);
    return TRUE;
}


/**
  Sets up gstreamer pipeline and the element structure in it. 
  @param flush_on_complete sets GST_STATE_NULL after playback 
  @return TRUE if success, FALSE if fails
*/
gboolean GstreamerTest::run_pipeline(gboolean flush_on_complete)
{
    MWTS_ENTER;
    if (!(local_bus||local_pipeline))
    {
        qCritical("No bus or pipeline available!");
        MWTS_LEAVE;
        return FALSE;
    }

    GstElement* videosink = NULL;
    GstElement* audiosink = NULL;
    GstPad* videosinkpad = NULL;
    guint videosinkpad_probe=0;

    /* run */
    GstStateChangeReturn ret;
    int i;
    local_frame_count = 0;
    qDebug("Starting pipeline");



    /* Set state first to paused - now playbin2 creates the internal pipeline */
    /* Creating pad into videosink and callback to measure framerate */
    if (local_framerate_measurement_enabled)
    {

        if (m_bMeasureFpsPerformance)
        {
            // Use fakesink instead of xvsink to measure performance
            videosink=gst_element_factory_make("fakesink", NULL);
            qDebug() << "videosink:" << videosink;
            audiosink=gst_element_factory_make("fakesink", NULL);
            qDebug() << "audiosink:" << audiosink;
            g_object_set(G_OBJECT(local_playbin), "video-sink", videosink, NULL);
            g_object_set(G_OBJECT(local_playbin), "audio-sink", audiosink,	NULL);
        }

        qDebug("GST_STATE_PLAYING ->");
        ret = gst_element_set_state (local_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            qCritical("Failed to set pipeline to playing state!");
            log_errors();
            MWTS_LEAVE;
            return FALSE;
        }
        sleep(1);
        qDebug("GST_STATE_PAUSED ->");
        ret = gst_element_set_state (local_pipeline, GST_STATE_PAUSED);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            qCritical("Failed to set pipeline to paused state!");
            log_errors();
            MWTS_LEAVE;
            return FALSE;
        }

        qDebug("Attaching probe to sinkpad");

        /* Attach pad probe to video sink pad (if found) */
        for(i=0; i<10 && videosink==NULL; i++)
        {
            sleep(1);
            videosink = gst_bin_get_by_name((GstBin*)local_playbin, "videosink");
        }

        if(!videosink)
        {
            qCritical("No video sink found");
            return FALSE;
        }
        else
        {
            if (m_bMeasureFpsPerformance)
            {
                // disable videosink sync to play video as quickly as possible
                g_object_set(G_OBJECT(videosink), "sync", FALSE , NULL);
            }
            // add sinkpad probe to get callback for every frame
            videosinkpad=gst_element_get_pad (videosink, "sink");
            qDebug("sink = %p found after %d seconds", videosink, i);
            qDebug("sinkpad = %p", videosinkpad);
            videosinkpad_probe = gst_pad_add_buffer_probe (videosinkpad, GstreamerTest_sink_data_cb_wrapper, NULL);
        }

    }
    /* Start playing */
    qDebug("GST_STATE_PLAYING ->");
    ret = gst_element_set_state (local_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        qCritical("Failed to set pipeline to playing state!");
        log_errors();
        MWTS_LEAVE;
        return FALSE;
    }

    /* Set the timers */
    set_timeouts();

    g_main_loop_run (local_mainloop);
    log_errors();

    /* mainloop is automatically stopped when stream ends
     or fail/duration timeouts occur */
    remove_timeouts();

    if(videosinkpad_probe)
    {
        gst_pad_remove_data_probe(videosinkpad, videosinkpad_probe);
        videosinkpad_probe=0;
    }
    if(videosinkpad)
    {
        gst_object_unref(videosinkpad);
        videosinkpad=NULL;
    }

    if(flush_on_complete)
    {
        ret = gst_element_set_state (local_pipeline, GST_STATE_NULL);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            qCritical("Failed to flush pipeline!");
            log_errors();
        }
    }

    if(local_fail_timeout_occurred)
    {
        qCritical("Fail timeout occurred during the test");
        MWTS_LEAVE;
        return FALSE;
    }
    if(local_frame_count > 0)
    {
        /* elapsed time is measured between first and last frame
         so frame count is actually framecount-1 */
        qDebug("local_frame_count: %d", local_frame_count);
        qDebug("local_elapsed_time: %d", local_elapsed_time);
		double framerate = (double)(local_frame_count-1)/(double)local_elapsed_time;
		g_pResult->AddMeasure("Framerate", framerate, "FPS");
		if (framerate < 15)
		{
			qCritical() << "Measured framerate critically low (" << framerate << ")";
		}
    }

    MWTS_LEAVE;
    return TRUE;

}

/**
  Queries the whole duration of media file or stream
  @param duration Length of media file in nanoseconds.
  @return TRUE if success, FALSE if fails
*/

gboolean GstreamerTest::get_duration (gint64* duration)
{
    GstFormat fmt = GST_FORMAT_TIME;

    if (gst_element_query_duration (local_pipeline, &fmt, duration))
    {
        duration = 0;
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}

/**
  Sets first timeout for changing volume level.
  @return TRUE if success
*/

gint GstreamerTest::set_change_volume_level(bool mode)
{
    MWTS_ENTER;
    //local_change_volume_level = TRUE;
    if(local_change_volume_timeout_source)
    {
        // remove timeouts
        g_source_remove(local_change_volume_timeout_source);
        local_change_volume_timeout_source=0;
    }

    local_volume_level_decreasing=mode;
    if (local_volume_level_decreasing==TRUE)
    {
        local_volume_level=1.0;
    }
    else
    {
        local_volume_level=0.0;
    }

    local_change_volume_timeout_source = g_timeout_add_seconds_full(G_PRIORITY_HIGH, VOLUME_CHANGE_TIMEOUT, (gboolean(*)(void*))GstreamerTest_set_volume_level_cb_wrapper, NULL, NULL);


    MWTS_LEAVE;
    return TRUE;
}




