/* MediaPlayerTest.h
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

#ifndef MEDIAPLAYERTEST_H
#define MEDIAPLAYERTEST_H

#include <MwtsCommon>
//#include <QObject>
#include <QMediaPlayer>
#include <QUrl>
#include <QVideoWidget>

/**
 *  This class provide functionality for playing audio files.
 */
class MediaPlayerTest : public QObject
{
    Q_OBJECT

public:

    /**
     *  enum describes behaviour after stopping playback
     */
    enum StopEventBehaviour {StopNormal, StopAndPlay};

    /**
     * Constructor for MediaPlayerTest class
     */
    MediaPlayerTest();

    /**
     * Destructor for MediaPlayerTest class
     */
    virtual ~MediaPlayerTest();

    /**
     * Function for MediaPlayerTest class
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * Function for MediaPlayerTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /**
     * Starts playing media audio file. Media file should be set before.
     * Currently playing fail only if error occurs.
     */
    void Play();

    /**
     *  Sets time of stop event
     *  @param seconds time to set
     */
    void SetStopTimeout(int seconds);

    /**
     *  Sets time of pause event
     *  @param seconds time to set
     */
    void SetPauseTimeout(int seconds);

    /**
     *  Sets duration of pause event
     *  @param seconds time to set
     */
    void SetPauseDuration(int seconds);

    /**
     *  Sets time of seek event
     *  @param seconds time to set
     */
    void SetSeekTimeout(int seconds);

    /**
     *  Sets position of playing media file
     *  @param position of media file
     */
    void SetSeekPosition(int position);

    /**
     *  Sets behaviour after of playback after stopping
     */
    void SetStopEventBehaviour(MediaPlayerTest::StopEventBehaviour behaviour);

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

    /**
     * Print on console supported MIME types
     */
    void PrintSupportedMimeTypes();

    /**
     *  Initialize video output for video media files
     */
    void InitVideo();

private slots:

    /**
     * Displays if audio media file can be played
     * @param available true if can be played otherwise false
     */
    void onAudioAvailableChanged(bool available);

    /**
     * Displays fill percentage of buffer
     * @param percentFilled
     */
    void onBufferStatusChanged(int percentFilled);

    /**
     * Displays when duration of media changed
     * @param duration changed duration
     */
    void onDurationChanged(qint64 duration);

    /**
    * Displays the emited error information.
    * If error occurs test will fails, then check error information.
    * @param error QMediaPlayer::Error value
    */
    void onError(QMediaPlayer::Error error);

    /**
     *  Displays info about current media file
     *  @param media media file object
     */
    void onMediaChanged(const QMediaContent& media);

    /**
     * Displays status message if media status changed
     * @param QMediaPlayer::MediaStatus value
     */
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    /**
     *  Displays if media file is muted or not
     *  @param muted true if muted false otherwise
     */
    void onMutedChanged(bool muted);

    /**
     */
    //void onNetworkConfigurationChanged (const QNetworkConfiguration& configuration);

    /**
     *  Displays info if playback rate chenged
     *  @param rate chenged rate
     */
    void onPlaybackRateChanged(qreal rate);

    /**
     *  Displays position of current playing file
     *  @param position to display
     */
    void onPositionChanged (qint64 position);

    /**
     *  Displays if media file is seekable
     *  @param seekable true if seekable false otherwise
     */
    void onSeekableChanged(bool seekable);

    /**
    * Displays the current QMediaPlayer state
    * @param state QMediaPlayer::State value
    */
    void onStateChanged(QMediaPlayer::State state);

    /**
     * Displays if video media file can be played
     * @param videoAvailable true if can be played otherwise false
     */
    void onVideoAvailableChanged (bool videoAvailable);

    /**
     *  Display if volume has chenged
     *  @param volume to show
     */
    void onVolumeChanged (int volume);

    /**
     *  Starts or resume playback
     */
    void onPlay();

    /**
     *  Stops the playback
     */
    void onStop();

    /**
     *  Pause playback for duration
     *  @param duration of pause
     */
    void onPause(int duration);

    /**
     *  Seeks madia file to position
     *  @param position to set
     */
    void onSeek(int position);

    /**
     *  Checks if stop/pause/seek event reachs its timeout
     */
    void checkTimeouts();


signals:
    void stop();
    void pause(int);
    void seek(int);


private:
    /**
     * Object used for playback feature.
     */
    QMediaPlayer* player;
    //playback time
    int playbackDuration;
    //pause duration
    int pauseDuration;
    //stop timeout in seconds
    int stopTimeout;
    //pause timeout in seconds
    int pauseTimeout;
    //seek timeout in seconds
    int seekTimeout;
    //seek position
    int seekPosition;
    //behaviour of test after stopping playback
    StopEventBehaviour stopEventBehaviour;
    //object used to display video
    QVideoWidget* videoOutput;

};

#endif // MEDIAPLAYERTEST_H
