/* AudioRecorderTest.h
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

#ifndef _INCLUDED_AUDIO_RECORDER_TEST_H
#define _INCLUDED_AUDIO_RECORDER_TEST_H

#include <MwtsCommon>
#include <qmediarecorder.h>

QT_BEGIN_NAMESPACE
    class QAudioCaptureSource;
    class QMediaRecorder;
QT_END_NAMESPACE

QT_USE_NAMESPACE

/**
 *  This class provide functionality for recording audio files.
 */
class AudioRecorderTest : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor for template test class
     */
    AudioRecorderTest();

    /**
     * Destructor for AudioRecorderTest class
     */
    virtual ~AudioRecorderTest();

    /**
     * Function for MultiMediaTest class
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * Function for MultiMediaTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /**
     * Record audio file
     * @return true or false depending on whether the recording was succesful or not
     */
    bool Record();

    /**
     * Sets the duration of the recording
     * @param recording duration in milliseconds
     */
    void SetRecordingDuration(int millisec);

    /**
    * Sets the used codec of the recording
    * @param codec name
    * @return true or false depends the audio codec is available or not
    */
    bool SetCodec(QString codec);

    /**
    * Sets the container of the recording audio
    * @param container name
    * @return true or false depends the container is available or not
    */
    bool SetContainer(QString container);

    /**
    * Sets the default container for given codec
    * @param container name
    * If codec is not supported constainer is set to an ampty string ""
    */
    void SetDefaultContainer(QString codec);

    /**
     * Sets the used input device of the recording
     * @param device name
     * @return true or false depends the input device is available or not
     */
    bool SetDevice(QString device);

    /**
     * Sets the recording quality
     * quality: very_high, high, normal, low, normal_low
     * @param quality name
     * @return true of false depends the quality is given accurately
     */
    bool SetQuality(QString quality);

    /**
     * Sets the bitrate in bit/sec.
     * @param bit rate
     */
    void SetBitRate(int bitRate);

    /**
     * Sets the sample rate (frequency) in Hertz.
     * @param frequency Hz
     */
    void SetSampleRate(int freq);

    /**
     * Sets the channel number.
     * @param number
     */
    void SetChannelCount(int channelNumber);

    /**
    * Sets the path of the recording output file(s).
    * @param path name
    */
    void SetPath(QString path);

    /**
    * MWTS_DEBUG out the available codecs.
    *
    */
    void PrintAvailableCodecs();

    /**
    * MWTS_DEBUG out the available audio input devices.
    *
    */
    void PrintAvailableDevices();

    /**
     * Gets full path of recently recorded file. It is used later when plaing back it.
     * @return fullRecordedFilePath
     */
    QString FullRecordedFilePath() const;

    /**
     * Shows supported codecs and containers. They are printed in debug message.
     * @return
     */
    void ShowSupportedCodecsAndContainers() const;

private:

    /**
    * Generates an incremental filename????, which helps to avoid overwriting
    * previously saved audio filenames.
    * @return next available filename
    * example 1. audio0000.ogg, after this functin call 2. audio0001.ogg...
    */
    QString NextOutputFilename();

    /**
    * Gives a file container extension according to the codec type
    * @param codec
    * @return file extension
    * example audio/vorbis codec --> .ogg
    */
    QString FileExtension(QString codec);

    /**
    * Gives a file container extension according to the container name
    * @param container
    * @return file extension
    * example matroska container --> .mkv
    */
    QString ContainerFileExtension(QString container);

private slots:

    /**
    * Displays the emited error
    * @param error
    */
    void displayErrorMessage(QMediaRecorder::Error error);

    /**
    * Displays the current QMediaRecorder state
    * @param state
    */
    void displayState(QMediaRecorder::State state);

    /**
    * Displays the elapsed recording time
    * @param codec
    */
    void updateProgress(qint64 pos);

private:
    //The main object used for recording
    QMediaRecorder* recorder;
    //Object used for setting up the audio source input:
    //for example: "device=alsa:front:CARD=Intel,DEV=0" on Eee PC
    QAudioCaptureSource* audioSource;
    //String holds recording directory's path
    QString recordingPath;
    //String holds recording file's name
    //for example: "recorded_audio????", the question marks mean the sequence number
    QString recordingFilename;
    //String holds recording file's extension is the return value of  FileExtension(codec) private method
    QString recordingExtension;
    int duration;
    //full path to recorded audio file, example: /home/meego/recorded_audio0001.wav
    QString fullRecordedFilePath;
    //names of codecs and containers, those are loaded from conf file.
    QStringList codecs;
    QStringList containers;

};

#endif //#ifndef _INCLUDED_AUDIO_RECORDER_TEST_H
