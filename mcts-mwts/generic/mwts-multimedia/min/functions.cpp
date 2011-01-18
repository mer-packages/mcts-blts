/* functions.cpp
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
#include "interface.h"
#include "MultimediaTest.h"


#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

MultimediaTest test;

/**
 * Shows debug message of supported codecs and containners on device.
 * @return ENOERR
 */
LOCAL int ShowSupportedCodecsAndContainers(__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;
    test.audioRecorder->ShowSupportedCodecsAndContainers();
    return ENOERR;
}


/**
 * Records audio for [duration] time then stops and saves
 *
 * @return ENOERR
 */

LOCAL int Record(__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;

    bool result = false;

    result = test.audioRecorder->Record();
    g_pResult->StepPassed(__FUNCTION__, result);

    if (!result)
        return 1;

    return ENOERR;
}

/**
  Sets the duration of the recording
  Usage: SetRecordingDuration [milliseconds]
  @param recording  in milliseconds
  @return ENOERR
*/
LOCAL int SetRecordingDuration(MinItemParser * item)
{
    MWTS_ENTER;

    int duration = 0;

    if(mip_get_next_int(item, &duration))
    {
        qCritical() << "Needs parameter as recording duration in milliseconds (>1000)";
    }
    else
    {
        if (duration < 1000)
        {
            qCritical() << "Positive 1000 <= number is expected (milliseconds)";
        }
        else
        {
            test.audioRecorder->SetRecordingDuration(duration);
        }
    }

    return ENOERR;
}

/**
  Sets the used container for the recording audio
  Usage: SetContainer [container]
  @param codec name
  @return ENOERR
*/
LOCAL int SetContainer(MinItemParser * item)
{
    MWTS_ENTER;

    char *container = NULL;
    bool result = false;

    if(mip_get_next_string(item, &container))
    {
        qCritical() << "The name must contain chars.";
    }
    else
    {
        result = test.audioRecorder->SetContainer(container);
        g_pResult->StepPassed(__FUNCTION__, result);
    }
    free(container);

    if (!result)
        return 1;

    return ENOERR;
}

/**
  Sets the used codec of the recording
  Usage: SetCodec [codec]
  @param codec name
  @return ENOERR
*/
LOCAL int SetCodec(MinItemParser * item)
{
    MWTS_ENTER;

    char *codec = NULL;
    bool result = false;

    if(mip_get_next_string(item, &codec))
    {
        qCritical() << "The name must contain chars.";
    }
    else
    {
        result = test.audioRecorder->SetCodec(codec);
        g_pResult->StepPassed(__FUNCTION__, result);
    }
    free(codec);

    if (!result)
        return 1;

    return ENOERR;
}

/**
  Sets the used input device of the recording
  Usage: SetDevice[device]
  @param device name
  @return ENOERR
*/
LOCAL int SetDevice(MinItemParser * item)
{
    MWTS_ENTER;

    char *device = NULL;
    bool result = false;

    if(mip_get_next_string(item, &device))
    {
        qCritical() << "The name must contain chars.";
    }
    else
    {
        result = test.audioRecorder->SetDevice(device);
        g_pResult->StepPassed(__FUNCTION__, result);
    }
    free(device);

    return ENOERR;
}

/**
  Sets the recording quality
  Usage: SetQuality [quality]
  quality: very_high, high, normal, low, normal_low
  @param quality name
  @return ENOERR
*/
LOCAL int SetQuality(MinItemParser * item)
{
    MWTS_ENTER;

    char *quality = NULL;
    bool result = false;

    if(mip_get_next_string(item, &quality))
    {
        qCritical() << "The name must be one of them: high, very_high, low, very_low";
    }
    else
    {
        result = test.audioRecorder->SetQuality(quality);
        g_pResult->StepPassed(__FUNCTION__, result);
    }
    free(quality);

    return ENOERR;
}

/**
  Sets the path of the recording output file.
  Usage: SetPath [path]
  @param path name
  @return ENOERR
*/
LOCAL int SetPath(MinItemParser * item)
{
    MWTS_ENTER;

    char *path = NULL;

    if (mip_get_next_string(item, &path))
    {
        qCritical() << "The name must contain chars.";
    }
    else
    {
        test.audioRecorder->SetPath(path);
    }
    free(path);

    return ENOERR;
}

/**
  Sets the channel number. Mono:1, Stereo:2
  Usage: SetChannelCount [number]
  @param number
  @return ENOERR
*/
LOCAL int SetChannelCount(MinItemParser * item)
{
    MWTS_ENTER;
    int number = 0;

    if(mip_get_next_int(item, &number))
    {
        qCritical() << "This must be a number: stereo 2, mono 1";
    }
    else
    {
        test.audioRecorder->SetChannelCount(number);
    }

    return ENOERR;
}

/**
  Sets the bitrate in bit/sec.
  Usage: SetBitRate [number]
  @param bit rate
  @return ENOERR
*/
LOCAL int SetBitRate(MinItemParser * item)
{
    MWTS_ENTER;

    int bitsec = 0;

    if(mip_get_next_int(item, &bitsec))
    {
        qCritical() << "This must be a number (bit/second)";
    }
    else
    {
        test.audioRecorder->SetBitRate(bitsec);
    }

    return ENOERR;
}

/**
  Sets the sample rate (frequency) in Hertz.
  Usage: SetSampleRate[frequecy]
  @param frequency Hz
  @return ENOERR
*/
LOCAL int SetSampleRate(MinItemParser * item)
{
    MWTS_ENTER;
    int freq=0;
    if(mip_get_next_int(item, &freq))
    {
        qCritical() << "This must be a number (Hz)";
    }

    if (freq < 1000)
    {
        qCritical() << "Positive 1000 <= number is expected.";
    }

    else
    {
        test.audioRecorder->SetSampleRate(freq);
    }

    return ENOERR;
}

/**
  Sets the timeout of the image playing
  Usage: SetImageTimeout [milliseconds]
  @param image playing timeout in milliseconds
  @return ENOERR
*/
LOCAL int SetImageTimeout(MinItemParser * item)
{
    MWTS_ENTER;

    int timeout = 0;

    if(mip_get_next_int(item, &timeout))
    {
        qCritical() << "Needs parameter as image playing timeout in milliseconds";
    }
    else
    {
        if (timeout < 1000)
        {
            qCritical() << "Positive 1000 <= number is expected (milliseconds)";
        }
        else
        {
            test.imageViewer->SetImageTimeout(timeout);
        }
    }

    return ENOERR;
}

/**
  Gets directory/file path as a paramater or no paramater, then
  displays every image(s) from the directory/file or default directory/file path is read from the config/MultimediaTest.conf file
  Usage: PlayImageViewer [ <directory path> | <file path> | no parameter = default from MultimediaTest.conf ]
  @param directory/file path to the images(s)
  @return ENOERR
*/
LOCAL int PlayImageViewer(MinItemParser * item)
{
    MWTS_ENTER;

    char *path = NULL;

    if (mip_get_next_string(item, &path))
    {        
        test.imageViewer->PlayImageViewer(QString(""));
    }
    else
    {
        test.imageViewer->PlayImageViewer(path);
    }
    free(path);

    if (!test.IsPassed())
        return 1;

    return ENOERR;
}

/**
  Sets the frequency in Hertz
  Usage: SetFrequency[frequency]
  @param frequency Hz
  @return ENOERR
*/
LOCAL int SetFrequency (MinItemParser * item)
{
    MWTS_ENTER;

    int freq = 0;

    if(mip_get_next_int(item, &freq))
    {
        qCritical() << "The frequency in Hertz a radio tuner is tuned to.";
    }
    else
    {
        if (freq < 1000)
        {
            qCritical() << "Positive 1000 <= number is expected.";
        }
        else
        {
            test.fmRadio->SetFrequency(freq);
        }
    }
    return ENOERR;
}

/**
  Starts a foreward scan for a signal, starting from the current frequency
  Usage: ScanUp
  @return ENOERR
*/
LOCAL int ScanUp (__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;
    test.fmRadio->ScanUp();
    MWTS_LEAVE;
    return ENOERR;
}

/**
  Starts a backward scan for a signal, starting from the current frequency
  Usage: ScanDown
  @return ENOERR
*/
LOCAL int ScanDown (__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;
    test.fmRadio->ScanDown();
    MWTS_LEAVE;
    return ENOERR;
}

/**
  Starts to play the radio on the set frequency for a certain time
  Usage: PlayRadio
  @return ENOERR
*/
LOCAL int PlayRadio (__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;
    test.fmRadio->PlayRadio();
    if (!test.IsPassed())
        return 1;
    MWTS_LEAVE;
    return ENOERR;
}

/**
  Set the radio volume in percentage
  Usage: SetVolume [volume]
  @param volume  as 0 <= integer number <= 100)
  @return ENOERR
*/
LOCAL int SetVolume (MinItemParser * item)
{
    MWTS_ENTER;

    int volume = 70;

    if(mip_get_next_int(item, &volume))
    {
        qCritical() << "The volume is an integer number.";
    }
    else
    {
        if (volume > 100 || volume < 0)
        {
            qCritical() << "100 >= Positive number >= 0 is expected.";
        }
        else
        {
            test.fmRadio->SetVolume(volume);
        }
    }
    MWTS_LEAVE;
    return ENOERR;
}

/**
  Sets the duration of the radio playing
  Usage: SetRadioDuration [milliseconds]
  @param radio playing in milliseconds
  @return ENOERR
 */
 LOCAL int SetRadioDuration(MinItemParser * item)
 {
    MWTS_ENTER;

    int duration = 0;

    if(mip_get_next_int(item, &duration))
    {
       qCritical() << "Needs parameter as radio playing duration in milliseconds (>1000)";
    }
    else
    {
        if (duration < 1000)
        {
            qCritical() << "Positive 1000 <= number is expected (milliseconds)";
        }
        else
        {
            test.fmRadio->SetRadioDuration(duration);
        }
    }

    return ENOERR;
}

/**
 *  Player functions
 */

/**
  Play recently recorder audio file.
  Can be call anly after calling Record function.
  @return ENOERR if playing pass with any error
 */
LOCAL int PlayRecordedAudio(__attribute__((unused)) MinItemParser * item)
{
     MWTS_ENTER;

     QString filePath = test.audioRecorder->FullRecordedFilePath();
     test.audioPlayer->SetMedia(filePath);
     test.audioPlayer->play();
     return ENOERR;
}

/**
  Sets the path of the file to play.
  Usage: SetMedia [file path]
  @param path name
  @return ENOERR
*/
LOCAL int SetMedia(MinItemParser * item)
{
    MWTS_ENTER;

    char *path = NULL;

    if (mip_get_next_string(item, &path))
    {
        qCritical() << "The name must contain chars.";
    }
    else
    {
        test.audioPlayer->SetMedia(path);
    }
    free(path);

    return ENOERR;
}

/**
  Set the audio player volume in percentage
  Usage: SetPlaybackVolume [volume]
  @param volume  as 0 <= integer number <= 100)
  @return ENOERR
*/
LOCAL int SetPlaybackVolume (MinItemParser * item)
{
    MWTS_ENTER;

    int volume = 70;

    if(mip_get_next_int(item, &volume))
    {
        qCritical() << "The volume is an integer number.";
    }
    else
    {
            test.audioPlayer->SetVolume(volume);
    }
    MWTS_LEAVE;
    return ENOERR;
}

/**
  Set the audio player duration in miliseconds
  Usage: SetPlaybackDuration [duration]
  @param duration > 0
  @return ENOERR
*/
LOCAL int SetPlaybackDuration (MinItemParser * item)
{
    MWTS_ENTER;

    int duration = 4000;

    if(mip_get_next_int(item, &duration))
    {
        qCritical() << "The volume is an integer number.";
    }
    else
    {
            test.audioPlayer->SetPlaybackDuration(duration);
    }
    MWTS_LEAVE;
    return ENOERR;
}

/**
 * Play set audio file.
 * @return ENOERR
 */
LOCAL int Play(__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;
    test.audioPlayer->play();
    return ENOERR;
}


/**
 * Function for MIN to gather our test case functions.
 * @param list	Functio pointer list, out
 * @return ENOERR
 */
int ts_get_test_cases (DLList ** list)
{   //min interface
    MwtsMin::DeclareFunctions(list);

    //multimedia audio recorder
    ENTRYTC (*list, "ShowSupportedCodecsAndContainers", ShowSupportedCodecsAndContainers);
    ENTRYTC (*list, "Record", Record);
    ENTRYTC (*list, "SetDevice", SetDevice);
    ENTRYTC (*list, "SetRecordingDuration", SetRecordingDuration);
    ENTRYTC (*list, "SetContainer", SetContainer);
    ENTRYTC (*list, "SetCodec", SetCodec);
    ENTRYTC (*list, "SetQuality", SetQuality);
    ENTRYTC (*list, "SetChannelCount", SetChannelCount);
    ENTRYTC (*list, "SetBitRate", SetBitRate);
    ENTRYTC (*list, "SetSampleRate", SetSampleRate);
    ENTRYTC (*list, "SetPath", SetPath);

     //multimedia image viewer
    ENTRYTC (*list, "SetImageTimeout", SetImageTimeout);
    ENTRYTC (*list, "PlayImageViewer", PlayImageViewer);

    //multimedia fm radio
    ENTRYTC (*list, "SetFrequency", SetFrequency);
    ENTRYTC (*list, "ScanUp", ScanUp);
    ENTRYTC (*list, "ScanDown", ScanDown);
    ENTRYTC (*list, "SetVolume", SetVolume);
    ENTRYTC (*list, "PlayRadio", PlayRadio);
    ENTRYTC (*list, "SetRadioDuration", SetRadioDuration);

    //multimedia audio player
    ENTRYTC (*list, "PlayRecordedAudio", PlayRecordedAudio);
    ENTRYTC (*list, "SetMedia", SetMedia);
    ENTRYTC (*list, "SetPlaybackVolume", SetPlaybackVolume);
    ENTRYTC (*list, "SetPlaybackDuration", SetPlaybackDuration);
    ENTRYTC (*list, "Play", Play);

    return ENOERR;
}
