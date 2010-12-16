/* AudioPlayerTest.h
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

#ifndef AUDIOPLAYERTEST_H
#define AUDIOPLAYERTEST_H

#include <QObject>
#include <QMediaPlayer>
#include <QUrl>

/**
 *  This class provide functionality for playing audio files.
 */
class AudioPlayerTest : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor for AudioPlayerTest class
     */
    AudioPlayerTest();

    /**
     * Destructor for AudioPlayerTest class
     */
    virtual ~AudioPlayerTest();

    /**
     * Function for AudioPlayerTest class
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * Function for AudioPlayerTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /**
     * Starts playing media audio file. Media file should be set before.
     * Currently playing fail only if error occurs.
     */
    void play();

    /**
     * Sets media file to play.
     * @param filePath path to media file
     */
    void SetMedia(const QString& filePath);

    /**
     * Sets playback volume.
     * @param volume value of volume between 0 and 100
     */
    void SetVolume(int volume);

    /**
     * Sets playback duration
     * @param msec duration in miliseconds
     */
    void SetPlaybackDuration(int msec);

private slots:

    /**
    * Displays the emited error information.
    * If error occurs test will fails, then check error information.
    * @param error QMediaPlayer::Error value
    */
    void onError(QMediaPlayer::Error error);

    /**
    * Displays the current QMediaPlayer state
    * @param state QMediaPlayer::State value
    */
    void onStateChanged(QMediaPlayer::State state);

    /**
     * Displays if audio media file can be played
     * @param available true id can be played otherwise false
     */
    void onAudioAvailableChanged(bool available);

    /**
     * Displays status message if media status changed
     * @param QMediaPlayer::MediaStatus value
     */
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);


private:
    /**
     * Object used for playback feature.
     */
    QMediaPlayer* player;
    //playback time
    int playbackDuration;

};

#endif // AUDIOPLAYERTEST_H
