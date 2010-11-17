/* AudioRecorderTest.cpp
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
#include "AudioRecorderTest.h"
#include <qaudiocapturesource.h>
#include <QDir>
#include <QUrl>

AudioRecorderTest::AudioRecorderTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

AudioRecorderTest::~AudioRecorderTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void AudioRecorderTest::OnInitialize()
{
    MWTS_ENTER;
    audioSource = new QAudioCaptureSource(this);
    audioSource->setAudioInput(g_pConfig->value("RECORDER/device").toString());

    recorder = new QMediaRecorder(audioSource, this);
    MWTS_DEBUG("Conf filename location:" + g_pConfig->fileName());
    duration = g_pConfig->value("RECORDER/duration").toInt();
    recordingFilename = g_pConfig->value("RECORDER/filename").toString();
    MWTS_DEBUG("Recording filename: " + recordingFilename);
    recordingPath = g_pConfig->value("RECORDER/path").toString();
    this->SetCodec(g_pConfig->value("RECORDER/codec").toString());

    qDebug() << "Recording duration: " << duration;

    if(!recorder->isAvailable())
    {
        MWTS_DEBUG ("QMediaRecorder service is not available");
    }

    connect(recorder, SIGNAL(error(QMediaRecorder::Error)), SLOT(displayErrorMessage(QMediaRecorder::Error)));
    connect(recorder, SIGNAL(stateChanged(QMediaRecorder::State)), SLOT(displayState(QMediaRecorder::State)));
    connect(recorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress(qint64)));

    //DEBUG
    PrintAvailableCodecs();
    PrintAvailableDevices();

    MWTS_LEAVE;
}

void AudioRecorderTest::OnUninitialize()
{
    MWTS_ENTER;

    if (recorder)
    {
        delete recorder;
        recorder = NULL;
    }

    if (audioSource)
    {
        delete audioSource;
        audioSource = NULL;
    }

    MWTS_LEAVE;
}

/**
 * Test functions
 */
bool AudioRecorderTest::Record()
{
    MWTS_ENTER;

    bool result = false;
    QString output = recordingPath+QDir::separator()+NextOutputFilename();

    MWTS_DEBUG("RECORDING output: " + output);
    MWTS_DEBUG("Start recording");

    //records for [duration] time
    recorder->setOutputLocation(output);
    recorder->record();

    QTimer::singleShot(duration, g_pTest, SLOT(Stop()));
    g_pTest->Start();
    recorder->stop();
    MWTS_DEBUG("Stop recording");

    QFile file(output);

    if (file.size()>0)
    {
        MWTS_DEBUG("Result recording succeeded");
        result = true;
    }

    else
    {
        MWTS_DEBUG("Result recording failed");
        result = false;
    }

    MWTS_LEAVE;

    return result;
}

void AudioRecorderTest::SetPath (QString path)
{
    MWTS_ENTER;
    recordingPath.clear();
    recordingPath=path;
    MWTS_LEAVE;
}

QString AudioRecorderTest::NextOutputFilename()
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
        qDebug() << __PRETTY_FUNCTION__ << "result:" << (prefix + r + postfix + recordingExtension);
        return prefix + r + postfix + recordingExtension;
    }
}

QString AudioRecorderTest::FileExtension(QString codec)
{
    if(codec == "audio/PCM")
    {
        return ".wav";
    }
    else if(codec == "audio/vorbis")
    {
        return ".ogg";
    }
    else
    {
        return ".out";
    }
}

bool AudioRecorderTest::SetCodec(QString codec)
{
    MWTS_ENTER;
    bool result = false;

    foreach(QString i, recorder->supportedAudioCodecs())
    {
        if (codec == i)
        {
            result = true;
            recorder->audioSettings().setCodec(codec);
            recordingExtension.clear();
            recordingExtension = FileExtension(codec);
            MWTS_DEBUG ("Set codec: " + codec);
        }
    }
    MWTS_LEAVE;

    return result;
}

bool AudioRecorderTest::SetQuality(QString quality){
    MWTS_ENTER;

    if(quality == "high")
    {
        recorder->audioSettings().setQuality(QtMultimediaKit::HighQuality);
        return true;
    }

    else if(quality == "very_high")
    {
        recorder->audioSettings().setQuality(QtMultimediaKit::VeryHighQuality);
        return true;
    }
    else if(quality == "low")
    {
        recorder->audioSettings().setQuality(QtMultimediaKit::LowQuality);
        return true;
    }

    else if(quality == "very_low")
    {
        recorder->audioSettings().setQuality(QtMultimediaKit::VeryLowQuality);
        return true;
    }

    else if(quality == "normal")
    {
        recorder->audioSettings().setQuality(QtMultimediaKit::NormalQuality);
        return true;
    }

    else
    {
        MWTS_DEBUG ("Quality has to be : high, very_high, low, very_low, normal");
        return false;
    }
}

void AudioRecorderTest::SetRecordingDuration(int millisec)
{
    MWTS_ENTER;
    duration = millisec;
    MWTS_LEAVE;
}

bool AudioRecorderTest::SetDevice(QString device)
{
    MWTS_ENTER;
    bool result = false;

    foreach(QString i, audioSource->audioInputs())
    {
        if(device==i)
        {
            result = true;
            audioSource->setAudioInput(device);
            MWTS_DEBUG("Set device: " + device);
        }
    }
    MWTS_LEAVE;

    return result;
}

void AudioRecorderTest::SetBitRate(int bitRate)
{
    MWTS_ENTER;
    recorder->audioSettings().setBitRate(bitRate);
    MWTS_LEAVE;
}

void AudioRecorderTest::SetSampleRate(int freq)
{
    MWTS_ENTER;
    recorder->audioSettings().setSampleRate(freq);
    MWTS_LEAVE;
}

void AudioRecorderTest::SetChannelCount(int channelNumber)
{
    MWTS_ENTER;
    recorder->audioSettings().setChannelCount(channelNumber);
    MWTS_LEAVE;
}

void AudioRecorderTest::PrintAvailableCodecs()
{
    MWTS_DEBUG("Codecs available:");
    foreach(QString i, recorder->supportedAudioCodecs())
    {
        MWTS_DEBUG(i);
    }
}

void AudioRecorderTest::PrintAvailableDevices()
{
    MWTS_DEBUG("Devices available:");

    foreach (QString i, audioSource->audioInputs())
    {
        MWTS_DEBUG(i);
    }
}

void AudioRecorderTest::displayErrorMessage (QMediaRecorder::Error error)
{
    MWTS_DEBUG("Error state:" + recorder->errorString());
}

void AudioRecorderTest::displayState(QMediaRecorder::State state)
{
}

void AudioRecorderTest::updateProgress(qint64 duration)
{
    if (recorder->error() != QMediaRecorder::NoError || duration < 1000)
    {
        qDebug () << "Error (possible that duration is too under 1000 millesecond)" << qRound(duration/1000) <<  " sec";
        return;
    }

    qDebug () << "Recorded " << qRound(duration/1000) <<  " sec";
}
