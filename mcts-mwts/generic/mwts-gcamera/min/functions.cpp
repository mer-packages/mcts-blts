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
#include "GCameraTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

#define MIPGETNEXTSTRING(ret,item,string,default) \
ret = mip_get_next_string(item,&string);\
if (ret != ENOERR) {	string=default;}

#define MIPGETNEXTINT(ret,item,int,default) \
ret = mip_get_next_int(item,&int);\
if (ret != ENOERR) {	int=default;}

#define FILLREQURED(param,default) \
if (param == INITPTR) {param = default;}

// Camera test class
GCameraTest Test;

LOCAL gint SetPipeline(__attribute__((unused)) MinItemParser *item)
{
    MWTS_ENTER;
    Test.setup_pipeline();
    g_pResult->StepPassed(__FUNCTION__, TRUE);
    MWTS_LEAVE;
    return ENOERR;
}

LOCAL gint TakePicture(MinItemParser *item)
{
    MWTS_ENTER;
    gchar *burst = NULL;
    if(ENOERR != mip_get_next_string(item, &burst))
    {
        if(burst)
        {
            g_free(burst);
            burst = NULL;
        }
        if(Test.take_picture(FALSE))
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            MWTS_LEAVE;
            return ENOERR;
        }
        else
        {
               g_pResult->StepPassed(__FUNCTION__, FALSE);
            MWTS_LEAVE;
            return EINVAL;
        }
    }
    else
    {
        //just assume that if something is written it means Burst mode
        if(Test.take_picture(TRUE))
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            MWTS_LEAVE;
            return ENOERR;
        }
        else
        {
            g_pResult->StepPassed(__FUNCTION__, FALSE);
            MWTS_LEAVE;
            return EINVAL;
        }
    }
}


LOCAL gint TakeVideo(MinItemParser *item)
{
    MWTS_ENTER;
    gint time = NULL;
    if(ENOERR != mip_get_next_int(item, &time))
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("Could not parse amount of time to take video");
        return EINVAL;
    }
    else
    {
        if(Test.take_video(time))
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            MWTS_LEAVE;
            return ENOERR;
        }
        else
        {
            g_pResult->StepPassed(__FUNCTION__, FALSE);
            MWTS_LEAVE;
            return EINVAL;
        }
    }
}


LOCAL gint SetAutoFocus(__attribute__((unused)) MinItemParser *item)
{
    MWTS_ENTER;
    Test.set_autofocus();
    g_pResult->StepPassed(__FUNCTION__, TRUE);
    MWTS_LEAVE;
    return ENOERR;
}

LOCAL gint SetIsoSpeed(MinItemParser *item)
{
    MWTS_ENTER;
    gint  iso_speed = 0;
    if(ENOERR != mip_get_next_int(item, &iso_speed))
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("Could not parse ISO speed");
        return EINVAL;
    }
    if(Test.set_iso_speed((guint)iso_speed))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}


LOCAL gint SetZoom(MinItemParser *item)
{
    MWTS_ENTER;
    gint zoom_value;
    if(ENOERR != mip_get_next_int(item, &zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("Could not parse zoom value");
        return EINVAL;
    }
    if(Test.set_zoom(zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint IncreaseZoom(MinItemParser *item)
{
    MWTS_ENTER;
    gint zoom_value;
    if(ENOERR != mip_get_next_int(item, &zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("Could not parse increase zoom value");
        return EINVAL;
    }
    if(Test.increase_zoom(zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint DecreaseZoom(MinItemParser *item)
{
    MWTS_ENTER;
    gint zoom_value;
    if(ENOERR != mip_get_next_int(item, &zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("Could not parse decrease zoom value");
        return EINVAL;
    }
    if(Test.decrease_zoom(zoom_value))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint SetVideoResolution(MinItemParser *item)
{
    MWTS_ENTER;
    gint x=0, y=0, fps_h=0, fps_l=0;
    if(ENOERR != mip_get_next_int(item, &x))
    {
        MWTS_ERROR("x value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(ENOERR != mip_get_next_int(item, &y))
    {
        MWTS_ERROR("y value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(ENOERR != mip_get_next_int(item, &fps_h))
    {
        fps_h = 0;
        fps_l = 0;
    }
    if(ENOERR != mip_get_next_int(item, &fps_l))
    {
        fps_h = 0;
        fps_l = 0;
    }

    Test.set_video_resolution(x,y,fps_h,fps_l);
    g_pResult->StepPassed(__FUNCTION__, TRUE);
    MWTS_LEAVE;
    return ENOERR;
}


LOCAL gint SetImageResolution(MinItemParser *item)
{
    MWTS_ENTER;
    gint x=0, y=0;
    if(ENOERR != mip_get_next_int(item, &x))
    {
        MWTS_ERROR("x value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(ENOERR != mip_get_next_int(item, &y))
    {
        MWTS_ERROR("y value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }

    Test.set_image_resolution(x,y);
    g_pResult->StepPassed(__FUNCTION__, TRUE);
    MWTS_LEAVE;
    return ENOERR;
}


LOCAL gint SetFPSMeasure(__attribute__((unused)) MinItemParser *item)
{
    Test.set_fps();
    g_pResult->StepPassed(__FUNCTION__, TRUE);
    MWTS_LEAVE;
    return ENOERR;
}

LOCAL gint SetImagePostProcessing(__attribute__((unused)) MinItemParser *item)
{
    if(Test.set_image_pp())
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint SetVideoPostProcessing(__attribute__((unused)) MinItemParser *item)
{
    if(Test.set_video_pp())
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint SetFlashMode(MinItemParser *item)
{
    //Off | On | FillIn | RedEye

    gchar *mode = NULL;
    gint enum_mode = 0;
    if(ENOERR != mip_get_next_string(item, &mode))
    {
        MWTS_ERROR("mode value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(strcmp(mode, "Auto") == 0)
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_AUTO;
    else if(strcmp(mode, "Off") == 0)
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_OFF;
    else if(strcmp(mode, "On") == 0)
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_ON;
    else if(strcmp(mode, "FillIn") == 0)
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN;
    else if(strcmp(mode, "RedEye") == 0)
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE;
    else
    {
        MWTS_ERROR("could not parse correct flash mode. Returning to off mode");
        enum_mode = GST_PHOTOGRAPHY_FLASH_MODE_OFF;
    }


    g_free(mode);

    if(Test.set_flashmode((GstFlashMode)enum_mode))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}


/**
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
 */
LOCAL gint SetTone(MinItemParser *item)
{
    gchar *mode = NULL;
    guint enum_mode = 0;
    if(ENOERR != mip_get_next_string(item, &mode))
    {
        MWTS_ERROR("mode value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(strcmp(mode, "Normal") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NORMAL;
    else if(strcmp(mode, "Sepia") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SEPIA;
    else if(strcmp(mode, "Negative") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NEGATIVE;
    else if(strcmp(mode, "Grayscale") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_GRAYSCALE;
    else if(strcmp(mode, "Natural") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NATURAL;
    else if(strcmp(mode, "Vivid") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_VIVID;
    else if(strcmp(mode, "Colorswap") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_COLORSWAP;
    else if(strcmp(mode, "Solarize") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_SOLARIZE;
    else if(strcmp(mode, "OutOfFocus") == 0)
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_OUT_OF_FOCUS;
    else
    {
        MWTS_ERROR("could not parse correct tone mode. Returning to normal mode");
        enum_mode = GST_PHOTOGRAPHY_COLOUR_TONE_MODE_NORMAL;
    }
    g_free(mode);
    if(Test.set_tone_mode((GstColourToneMode)enum_mode))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }   
}

/*
 *  GST_PHOTOGRAPHY_WB_MODE_AUTO = 0,
 *  GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT,
 *  GST_PHOTOGRAPHY_WB_MODE_CLOUDY,
 *  GST_PHOTOGRAPHY_WB_MODE_SUNSET,
 *  GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN,
 *  GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT
 */
LOCAL gint SetWhiteBalance(MinItemParser *item)
{
    gchar *mode = NULL;
    guint enum_mode = 0;
    if(ENOERR != mip_get_next_string(item, &mode))
    {
        MWTS_ERROR("mode value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
    if(strcmp(mode, "Auto") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_AUTO;
    else if(strcmp(mode, "Daylight") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_DAYLIGHT;
    else if(strcmp(mode, "Cloudy") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_CLOUDY;
    else if(strcmp(mode, "Sunset") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_SUNSET;
    else if(strcmp(mode, "Tugsten") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_TUNGSTEN;
    else if(strcmp(mode, "Fluorscent") == 0)
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_FLUORESCENT;
    else
    {
        MWTS_ERROR("could not parse correct whitebalance mode. Returning to automatic mode");
        enum_mode = GST_PHOTOGRAPHY_WB_MODE_AUTO;
    }


    g_free(mode);

    if(Test.set_wb_mode((GstWhiteBalanceMode)enum_mode))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

/*
 * GST_CAMERABIN_FLAG_SOURCE_RESIZE                    - (0x00000001): source-resize - Enable source crop and scale
 * GST_CAMERABIN_FLAG_SOURCE_COLORSPACE_CONVERSION     - (0x00000002): source-colorspace-conversion - Enable colorspace conversion for video source
 * GST_CAMERABIN_FLAG_VIEWFINDER_COLORSPACE_CONVERSION - (0x00000004): viewfinder-colorspace-conversion - Enable colorspace conversion for viewfinder
 * GST_CAMERABIN_FLAG_VIEWFINDER_SCALE                 - (0x00000008): viewfinder-scale - Enable scale for viewfinder
 * GST_CAMERABIN_FLAG_AUDIO_CONVERSION                 - (0x00000010): audio-conversion - Enable audio conversion for video capture
 * GST_CAMERABIN_FLAG_DISABLE_AUDIO                    - (0x00000020): disable-audio    - Disable audio elements for video capture
 * GST_CAMERABIN_IMAGE_COLORSPACE_CONVERSION           - (0x00000040): image-colorspace-conversion - Enable colorspace conversion for still image
 */

LOCAL gint SetFlags(MinItemParser *item)
{
    gchar *mode = NULL;
    gint enum_mode = 0;

    if(ENOERR != mip_get_next_string(item, &mode))
    {
        MWTS_ERROR("camerabin flags value expected");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }

    if(strcmp(mode, "SourceResize") == 0)
        enum_mode = GST_CAMERABIN_FLAG_SOURCE_RESIZE;
        else if(strcmp(mode, "SourceColorspaceConversion") == 0)
        enum_mode = GST_CAMERABIN_FLAG_SOURCE_COLORSPACE_CONVERSION;
    else if(strcmp(mode, "ViewfinderColorspaceConversion") == 0)
        enum_mode = GST_CAMERABIN_FLAG_VIEWFINDER_COLORSPACE_CONVERSION;    
    else
    {
        MWTS_ERROR("Could not parse correct flags, returning with default.");
        enum_mode = 0;
    }

    g_free(mode);

    if(Test.set_flags((GstCameraBinFlags)enum_mode))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {        
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_ERROR("The given parameter is not okay.");
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint SetAperture(MinItemParser *item)
{
    gint amount = 0;
    if(ENOERR != mip_get_next_int(item, &amount))
    {
        MWTS_ERROR("value expected");
        return EINVAL;
    }

    if(Test.set_aperture(amount))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}

LOCAL gint SetExposure(MinItemParser *item)
{
    gint amount = 0;
    if(ENOERR != mip_get_next_int(item, &amount))
    {
        MWTS_ERROR("value expected");
        return EINVAL;
    }

    if(Test.set_exposure(amount))
    {
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        MWTS_LEAVE;
        return ENOERR;
    }
    else
    {
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return EINVAL;
    }
}


/**
  Test function list for MIN
  @return ENOERR
*/
gint ts_get_test_cases( DLList** list )
{
    // declare common functions like Init, Close..
    MwtsMin::DeclareFunctions(list);

    ENTRYTC(*list,"SetPipeline", SetPipeline);
    ENTRYTC(*list,"SetFlags", SetFlags);

    ENTRYTC(*list,"TakePicture", TakePicture);
    ENTRYTC(*list,"TakeVideo", TakeVideo);

    ENTRYTC(*list,"SetImageResolution", SetImageResolution);
    ENTRYTC(*list,"SetVideoResolution", SetVideoResolution);
    ENTRYTC(*list,"SetFPSMeasure", SetFPSMeasure);
    ENTRYTC(*list,"SetImagePostProcessing", SetImagePostProcessing);
    ENTRYTC(*list,"SetVideoPostProcessing", SetVideoPostProcessing);

    /************************************************************
     *
     * GStreamer photography interface
     * interfaces/photography
     */
    ENTRYTC(*list,"SetAutoFocus", SetAutoFocus);
    ENTRYTC(*list,"SetIsoSpeed", SetIsoSpeed);
    ENTRYTC(*list,"SetZoom", SetZoom);
    ENTRYTC(*list,"IncreaseZoom", IncreaseZoom);
    ENTRYTC(*list,"DecreaseZoom", DecreaseZoom);
    ENTRYTC(*list,"SetAperture", SetAperture);
    ENTRYTC(*list,"SetExposure", SetExposure);
    ENTRYTC(*list,"SetFlashMode", SetFlashMode);
    ENTRYTC(*list,"SetTone", SetTone);
    ENTRYTC(*list,"SetWhiteBalance", SetWhiteBalance);

    return ENOERR;

}
