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
#include "interface.h"
#include "gstreamer.h"

#include <MwtsCommon>

#include <glib-object.h>
#include <gst/gst.h>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

#define MIPGETNEXTSTRING(ret,item,string,default) \
ret = mip_get_next_string(item,&string);\
if (ret != ENOERR) {	string=default;}

#define FILLREQURED(param,default) \
if (param == INITPTR) {param = default;}

// Test class
GstreamerTest Test;

/**
  SetFailTimeout2 -function for scripted cases
  Sets time limit how long playback will continue.
  This is for situations where file length is known.
  After timeout playback is stopped normally
  Usage: SetFailTimeout2 [seconds]
  @param seconds desired duration given in seconds
  @return ENOERR
*/
LOCAL gint SetFailTimeout2(MinItemParser * item)
{
    MWTS_ENTER;
    int timeout=0;
    if(ENOERR != mip_get_next_int(item, &timeout))
    {
        qCritical() << "Could not parse timeout parameter for SetFailTimeout2";
        return EINVAL;
    }
    qDebug() << "SETTING FAIL TIMEOUT:" << timeout;
    Test.set_fail_timeout(timeout);
    return ENOERR;
}

/**
  SetDuration -function for scripted cases
  Sets time limit how long clip will be played.
  After that playback is stopped normally
  Usage: SetDuration [seconds]
  @param seconds desired duration given in seconds
  @return ENOERR
*/
LOCAL gint SetDuration(MinItemParser * item)
{
	MWTS_ENTER;
	int duration=0;
	if(ENOERR != mip_get_next_int(item, &duration))
	{
		qCritical() << "Could not parse duration parameter for SetDuration";
		return EINVAL;
	}
	Test.set_duration(duration);
	return ENOERR;
}
/**
  SetEvent -function for scripted cases
  Sets time and event 
  Usage: SetEvent [event] [seconds]
  @param event desired event. [PAUSE|CONTINUE]
  @param seconds desired duration given in seconds
  @return ENOERR
*/
LOCAL gint SetEvent (MinItemParser * item)
{
  
	MWTS_ENTER;
	int timeout=0;
	gchar* event_type = NULL;
	if(ENOERR != mip_get_next_string(item, &event_type))
	{
		qCritical() << "Could not parse source type for SetSource";
		return EINVAL;
	}
	if(strcmp(event_type, "SeekToEnd") == 0)
	{
		if(ENOERR != mip_get_next_int(item, &timeout))
		{
			qCritical() << "Could not parse duration parameter for SetEvent";
			return EINVAL;
		}
		Test.set_event_timeout(MWTS_GST_EVENT_SEEK_TO_END,timeout);
	}
	if(strcmp(event_type, "Pause") == 0)
	{
		if(ENOERR != mip_get_next_int(item, &timeout))
		{
			qCritical() << "Could not parse duration parameter for SetEvent";
			return EINVAL;
		}
		Test.set_event_timeout(MWTS_GST_EVENT_PAUSE,timeout);
	}

	return ENOERR;
}

/**
  SetEvent -function for scripted cases
  Sets time and event 
  Usage: SetEvent [event] [seconds]
  @param event desired event. [PAUSE|CONTINUE]
  @param seconds desired duration given in seconds
  @return ENOERR
*/
LOCAL gint SetSecondEvent (MinItemParser * item)
{
	MWTS_ENTER;
	int timeout=0;
	gchar* event_type = NULL;
	if(ENOERR != mip_get_next_string(item, &event_type))
	{
		qCritical() << "Could not parse source type for SetSource";
		return EINVAL;
	}
	if(strcmp(event_type, "Continue") == 0)
	{
		if(ENOERR != mip_get_next_int(item, &timeout))
		{
			qCritical() << "Could not parse duration parameter for SetEvent";
			return EINVAL;
		}
		Test.set_second_event_timeout(MWTS_GST_EVENT_CONTINUE,timeout);
	}

	return ENOERR;
}


/**
  SetSeekTimeout -function for scripted cases
  Sets time when seeking is done
  Usage: SetSeekTimeout [seconds]
  @param seconds desired duration given in seconds
  @return ENOERR
*/
LOCAL gint SetSeekTimeout(MinItemParser * item)
{
	MWTS_ENTER;
	int timeout=0;
	if(ENOERR != mip_get_next_int(item, &timeout))
	{
		qCritical() << "Could not parse duration parameter for SetSeekTimeout";
		return EINVAL;
	}
	Test.set_seek_timeout(timeout);
	return ENOERR;
}
/**
  SeekPosition -function for scripted cases
  Sets playback position to desired position
  Usage: SeekToEndPosition [seconds]
  @param seconds desired timestamp given in seconds
  @return ENOERR
*/
LOCAL gint SetSeekPosition(MinItemParser * item)
{
	MWTS_ENTER;
	int position = 0;
	gint64 seeking_position = 0;

	if(ENOERR != mip_get_next_int(item, &position))
	{
		qCritical() << "Could not parse position parameter for SetSeekPosition";
		return EINVAL;
	}
	/* gstreamer handles position internally in nanoseconds */
	seeking_position = position * GST_SECOND;
	qDebug() << "Setting seek position " << seeking_position ;
	Test.set_seek_position(seeking_position);
	return ENOERR;
}

/**
  SeekToEndPosition -function for scripted cases
  Sets playback position to the end of file
  Usage: SeekToEndPosition
  @return ENOERR
*/
LOCAL gint SeekToEndPosition(MinItemParser * item)
{
	MWTS_ENTER;
	gint64 duration = 0;
	Test.get_duration (&duration);
	duration = duration - 1;
	qDebug() << "Setting seek position " << duration;
	Test.set_seek_position(duration);
	return ENOERR;
}

/**
  MeasureFramerate -function for scripted cases
  Enables framerate measuring
  Usage: SeekToEndPosition
  @return ENOERR
*/
LOCAL gint MeasureFramerate(MinItemParser * item)
{
	MWTS_ENTER;
	Test.set_enable_framerate_measurement ();
	return ENOERR;
}

/**
  SetChangeVolumeLevel -function for scripted cases
  Changes volume level
  Usage: SetChangeVolumeLevel up, SetChangeVolumeLevel down, SetChangeVolumeLevel
  @param [<up>|<down>|]
  @return ENOERR
*/
LOCAL gint SetChangeVolumeLevel(MinItemParser * item)
{
	MWTS_ENTER;

    gchar* mode = NULL;
    bool ret;

    if(ENOERR != mip_get_next_string(item, &mode))
    {
        ret = Test.set_change_volume_level ();
        g_pResult->StepPassed( __FUNCTION__, ret );
        return ENOERR;
    }

    else if(strcmp(mode, "up") == 0)
    {
        ret = Test.set_change_volume_level (FALSE);
        g_pResult->StepPassed( __FUNCTION__, ret );
        return ENOERR;
    }

    else if(strcmp(mode, "down") == 0)
    {
        ret = Test.set_change_volume_level (TRUE);
        g_pResult->StepPassed( __FUNCTION__, ret);
        return ENOERR;
    }
    else
    {
        qCritical() << "Parameter must be up/down/ or no paramater";
        g_pResult->StepPassed( __FUNCTION__, FALSE );
        return EINVAL;
    }

    g_free(mode);
}

/**
  SetSource -function for scripted cases
  Sets file or stream location to be played 
  Usage: SetSource [File|HTTP|RTSP] path_to_file
  @param path to file
  @return ENOERR
*/
LOCAL gint SetSource(MinItemParser *item)
{
	MWTS_ENTER;
	gchar* source_type = NULL;
	gchar* filename = NULL;
	gchar* uri = NULL;
    gchar* protocol = NULL;

    if (ENOERR != mip_get_next_string( item, &protocol ))
    {
        qCritical() << "could not parse URI for SetSource";
        return EINVAL;
    }

	if (ENOERR != mip_get_next_string( item, &uri ))
	{
		qCritical() << "could not parse URI for SetSource";
		return EINVAL;
	}

	qDebug() << "Using uri source " << uri;
        if(FALSE == Test.set_source_uri(protocol, uri))
	{
		qCritical() << "Unable to use source URI " << uri; 
		g_pResult->StepPassed( __FUNCTION__, FALSE );
		return EINVAL;
	}
	g_free(uri);
	g_free(source_type);
	g_pResult->StepPassed( __FUNCTION__, TRUE );

	return ENOERR;
}


/**
  RunTest -function for scripted cases. Creates pipeline and starts playback.
  Usage: RunTest
  @param: noflush - disables automated pipeline flushing
  @return ENOERR
*/
LOCAL gint RunTest( MinItemParser *item)
{
	MWTS_ENTER;

	gchar *opt = NULL;
	mip_get_next_string(item, &opt);

	/* default: flush the pipeline at end (potentially freeing resources,
	   required for iterative testing without recreating pipe + bin ) */
	gboolean flush_at_complete = opt ? strcmp(opt,"noflush") != 0 : TRUE;
	g_free(opt);
	opt = NULL;

	if( ! flush_at_complete )
	{
		qDebug() << "Skipping pipeline flush at stream completion/timeout";
	}

	if( FALSE == Test.run_pipeline(flush_at_complete) )
	{
		qCritical() << "Pipeline running failure";
			g_pResult->StepPassed( __FUNCTION__,FALSE );
		return EINVAL;
	}
	g_pResult->StepPassed( __FUNCTION__, TRUE );

	return ENOERR;
}


/**
  RunTest -function for scripted cases. Creates pipeline and starts playback.
  Usage: RunTest
  @param: noflush - disables automated pipeline flushing
  @return ENOERR
*/
LOCAL gint EnableFPSPerformance( MinItemParser *item)
{
	MWTS_ENTER;

	Test.EnableFPSPerformance();

	return 0;
}
/**
  Test function list for MIN
  @return ENOERR
*/
gint ts_get_test_cases( DLList** list )
{
  	MwtsMin::DeclareFunctions(list);
	ENTRYTC(*list,"RunTest", RunTest);
	ENTRYTC(*list,"SetSource", SetSource);
    ENTRYTC(*list,"SetFailTimeout2", SetFailTimeout2);
	ENTRYTC(*list,"SetDuration", SetDuration);
	ENTRYTC(*list,"SetSeekTimeout", SetSeekTimeout);
	ENTRYTC(*list,"SetSeekPosition", SetSeekPosition);
	ENTRYTC(*list,"SeekToEndPosition", SeekToEndPosition);
	ENTRYTC(*list,"SetEvent", SetEvent);
	ENTRYTC(*list,"SetSecondEvent", SetSecondEvent);
	ENTRYTC(*list,"MeasureFramerate", MeasureFramerate);
	ENTRYTC(*list,"SetChangeVolumeLevel", SetChangeVolumeLevel);
	ENTRYTC(*list,"EnableFPSPerformance", EnableFPSPerformance);

	return ENOERR;

}


