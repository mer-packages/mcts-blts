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

#ifndef _INCLUDED_GSTREAMER_TEST_H
#define _INCLUDED_GSTREAMER_TEST_H

#include <MwtsCommon>
#include <glib-2.0/glib-object.h>
#include <dbus-1.0/dbus/dbus-glib.h>
#include <dbus-1.0/dbus/dbus-glib-lowlevel.h>
#include <gstreamer-0.10/gst/gst.h>


enum gstreamer_event_type 
{
    MWTS_GST_EVENT_PAUSE = 0,
    MWTS_GST_EVENT_CONTINUE,
    MWTS_GST_EVENT_SEEK,
    MWTS_GST_EVENT_SEEK_TO_END
};

/**
 * GstreamerTest class
 * Test class for testing gstreamer functionality.
 * Audio and Video playbacks for local and streaming.
 *
*/
class GstreamerTest : public MwtsTest
{
	Q_OBJECT
public:

    /**
     * Constructor for template test class
     */
    GstreamerTest();

    /**
     * Destructor for template test class
     */
    virtual ~GstreamerTest();


    /**
     * Overridden functions for MwtsTest class
     * OnInitialize is called before test execution
     */

    void OnInitialize();

    /**
     * Overridden functions for MwtsTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();



    void stop_test_internal();
    gboolean fail_timeout_cb();
    gboolean duration_timeout_cb();
    gboolean seeking_timeout_cb();
    //gboolean event_timeout_cb();
    gboolean event_timeout_cb();

    gint create_pipeline();
    gint set_source_uri(const char* protocol, const char* uri);
    gint set_source_file(const char* filename);
    gint set_source_http(const char* filename);
    gint set_source_rtsp(const char* filename);
    void set_seek_position(gint64 seek_position);
    void set_seek_timeout(gint seek_timeout);
    //gint get_position (gint64* playback_position);
    gboolean get_duration (gint64* playback_duration);
    gint set_change_volume_level(bool mode=TRUE);
    gboolean set_volume_level_cb();

    gboolean run_pipeline(gboolean flush_on_complete);

    void debug_print_state_change(int newstate);
    gboolean bus_cb (GstBus *bus, GstMessage *msg, gpointer data);
    gboolean sink_data_cb();

    void set_fail_timeout(gint new_timeout_seconds);
    void set_duration(gint new_duration_seconds);

    void set_timeouts();
    void remove_timeouts();
    void set_event_timeout(gint event_type, gint event_timeout_seconds);
    void set_second_event_timeout(gint second_event_type, gint second_event_timeout_seconds);

    void log_errors();
    gboolean second_event_timeout_cb();
    void set_enable_framerate_measurement ();

    void EnableFPSPerformance();
private:

    GstBus* local_bus;
    GstElement* local_pipeline;
    GstElement* local_playbin;
    GMainLoop* local_mainloop;
    bool local_fail_timeout_occurred;
    bool local_duration_timeout_occurred;
    bool local_seeking_enabled;
    bool local_event_enabled;
    bool local_second_event_enabled;
    bool local_framerate_measurement_enabled;
    bool local_change_volume_level;

    guint local_fail_timeout_source;
    guint local_duration_timeout_source;
    guint local_seeking_timeout_source;
    guint local_event_timeout_source;
    guint local_second_event_timeout_source;
    guint local_change_volume_timeout_source;
    gint64 local_seeking_position;
    gint local_bus_watch_source;
    GTimer* local_timer;
    int local_frame_count;
    int local_elapsed_time;
    gint local_event_type;
    gint local_second_event_type;


    /* For controlling volume level during playback */
    gfloat local_volume_level;
    bool local_volume_level_decreasing;

    /* These are user-given settings for timeouts with default values */
    gint local_fail_timeout_setting;
    gint local_duration_timeout_setting;
    gint local_seeking_timeout_setting; /* make seek after 10 sec */
    gint local_event_timeout_setting;
    gint local_second_event_timeout_setting;

    bool m_bMeasureFpsPerformance;
};

#endif //#ifndef _INCLUDED_GSTREAMER_TEST_H

