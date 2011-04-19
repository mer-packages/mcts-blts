/* MediaPlayerTest.cpp
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
#include "MediaPlayerTest.h"
//#include <QDir>
#include <QTimer>


MediaPlayerTest::MediaPlayerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

MediaPlayerTest::~MediaPlayerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void MediaPlayerTest::OnInitialize()
{
    MWTS_ENTER;

    //PrintSupportedMimeTypes();

    player = new QMediaPlayer(this);
    //this->SetVolume(g_pConfig->value("PLAYER/volume").toInt());

    //default values for timeouts
    stopTimeout = 0;
    pauseTimeout = 0;
    seekTimeout = 0;

    //default durations
    pauseDuration = 6;
    seekPosition = 10;

    //default stop behavior
    stopEventBehaviour = StopNormal;

    //video widget is not allocated by default
    videoOutput = NULL;

    connect(player, SIGNAL(audioAvailableChanged(bool)), this, SLOT(onAudioAvailableChanged(bool)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(onBufferStatusChanged(int)));
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(onError(QMediaPlayer::Error)));
    connect(player, SIGNAL(mediaChanged(const QMediaContent&)), this, SLOT(onMediaChanged(const QMediaContent&)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(mutedChanged(bool)), this, SLOT(onMutedChanged(bool)));
    connect(player, SIGNAL(playbackRateChanged(qreal)), this, SLOT(onPlaybackRateChanged(qreal)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));
    connect(player, SIGNAL(seekableChanged(bool)), this, SLOT(onSeekableChanged(bool)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onStateChanged(QMediaPlayer::State)));
    connect(player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(onVideoAvailableChanged(bool)));
    connect(player, SIGNAL(volumeChanged(int)), this, SLOT(onVolumeChanged(int)));

    //slots connected to signals which are emitted in stop/pause/seek timeouts
    connect(this, SIGNAL(stop()), this, SLOT(onStop()));
    connect(this, SIGNAL(pause(int)), this, SLOT(onPause(int)));
    connect(this, SIGNAL(seek(int)), this, SLOT(onSeek(int)));

    //timer checking if stop/pause/seek events are timeout
    QTimer* t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(checkTimeouts()));
    t->start(250);

    MWTS_LEAVE;
}

void MediaPlayerTest::OnUninitialize()
{
    MWTS_ENTER;

    if (player)
    {
        delete player;
        player = NULL;
    }
    if (videoOutput)
    {
        delete videoOutput;
        videoOutput = NULL;
    }

    MWTS_LEAVE;
}

void MediaPlayerTest::Play()
{
    MWTS_ENTER;

    qDebug() << "Attempt to play " << player->media().canonicalUrl().toString() << "(" << player->duration() << "ms)" << ", with volume " << player->volume();

    player->play();
    if (player->isMuted())
        player->setMuted(false);

    g_pTime->start();
    g_pTest->Start();

    MWTS_LEAVE;
}

void MediaPlayerTest::SetStopTimeout(int seconds)
{
    MWTS_ENTER;

    qDebug() << "Stop Timeout set to: " << seconds;
    stopTimeout = seconds;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetPauseTimeout(int seconds)
{
    MWTS_ENTER;

    qDebug() << "Stop Timeout set to: " << seconds;
    pauseTimeout = seconds;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetPauseDuration(int seconds)
{
    MWTS_ENTER;

    qDebug() << "Pause duration set to: " << seconds;
    pauseDuration = seconds;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetSeekTimeout(int seconds)
{
    MWTS_ENTER;

    qDebug() << "Seek Timeout set to: " << seconds;
    seekTimeout = seconds;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetSeekPosition(int position)
{
    MWTS_ENTER;

    qDebug() << "Seek position set to: " << position;
    seekPosition = position;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetStopEventBehaviour(MediaPlayerTest::StopEventBehaviour behaviour)
{
    MWTS_ENTER;

    switch (behaviour)
    {
    case MediaPlayerTest::StopNormal:
        qDebug() << "Playback will be stopped normally";
        break;
    case MediaPlayerTest::StopAndPlay:
        qDebug() << "Playback will be stopped and then played from beginning";
        break;
    default:
        break;
    }
    stopEventBehaviour = behaviour;

    MWTS_LEAVE;
}

void MediaPlayerTest::SetMedia(const QString& filePath)
{
    MWTS_ENTER;

    player->setMedia(QUrl::fromLocalFile(filePath));
    qDebug() << "Set media file: " << filePath;

    MWTS_LEAVE;
}


void MediaPlayerTest::SetVolume(int volume)
{
    MWTS_ENTER;
    if (volume > 100 || volume < 0)
    {
        qCritical() << "Volume has to be number between 0 and 100";
    }
    player->setVolume(volume);
    qDebug() << "Set volume: " << volume;
    MWTS_LEAVE;
}

void MediaPlayerTest::SetPlaybackDuration(int msec)
{
    MWTS_ENTER;
    if (msec <= 0)
    {
        qCritical() << "Duration should be greater than 0. Recommended value is at least 2000 ms";
    }
    playbackDuration = msec;
    qDebug() << "Set playback duration: " << msec;
    MWTS_LEAVE;
}

void MediaPlayerTest::PrintSupportedMimeTypes()
{
    MWTS_ENTER;

    qDebug() << "This player supports following MIME types:";
    foreach(QString s, QMediaPlayer::supportedMimeTypes())
    {
        qDebug() << s;
    }
    MWTS_LEAVE;
}

void MediaPlayerTest::InitVideo()
{
    MWTS_ENTER;

    //videoOutput = new QVideoWidget(g_pMainWindow);
    videoOutput = new QVideoWidget();
    player->setVideoOutput(videoOutput);


    g_pMainWindow->setCentralWidget(videoOutput);
    //g_pMainWindow->setWindowTitle(tr("Image Viewer"));
    g_pMainWindow->showFullScreen();
    //g_pMainWindow->show();

    MWTS_LEAVE;
}

/* private slots */

void MediaPlayerTest::onError(QMediaPlayer::Error error)
{
    qCritical() << "Error has occured:" << player->errorString() << ", error code " << error;
    g_pTest->Stop();
}


void MediaPlayerTest::onStateChanged(QMediaPlayer::State state)
{
    MWTS_ENTER;
    switch (state)
    {
        case QMediaPlayer::PlayingState:
            qDebug() << "Playing " << player->media().canonicalUrl().toString() << " audio file";
            break;
         case QMediaPlayer::PausedState:
            qDebug() << "Paused " << player->media().canonicalUrl().toString() << " audio file";
            break;
        case QMediaPlayer::StoppedState:
            qDebug() << "Stopped " << player->media().canonicalUrl().toString() << " audio file";
            break;
        default:
            qDebug() << "Unknown player state";
            break;
    }
    MWTS_LEAVE;
}

void MediaPlayerTest::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    MWTS_ENTER;

    switch (status)
    {
    case QMediaPlayer::UnknownMediaStatus:
        qDebug() << "Media status changed to: UnknownMediaStatus - " << status;
        break;
    case QMediaPlayer::NoMedia:
        qDebug() << "Media status changed to: NoMedia - " << status;
        break;
    case QMediaPlayer::LoadingMedia:
        qDebug() << "Media status changed to: LoadingMedia - " << status;
        break;
    case QMediaPlayer::LoadedMedia:
        qDebug() << "Media status changed to: LoadedMedia - " << status;
        break;
    case QMediaPlayer::StalledMedia:
        qDebug() << "Media status changed to: StalledMedia - " << status;
        break;
    case QMediaPlayer::BufferingMedia:
        qDebug() << "Media status changed to: BufferingMedia - " << status;
        break;
    case QMediaPlayer::BufferedMedia:
        qDebug() << "Media status changed to: BufferedMedia - " << status;
        break;
    case QMediaPlayer::EndOfMedia:
        g_pTest->Stop();
        qDebug() << "Media status changed to: EndOfMedia - " << status;
        break;
    case QMediaPlayer::InvalidMedia:
        qDebug() << "Media status changed to: InvalidMedia - " << status;
        break;
    default:
        qDebug() << "Unknown status";
        break;
    }

    MWTS_LEAVE;
}

void MediaPlayerTest::onAudioAvailableChanged(bool available)
{
    if (available)
    {
        qDebug() << "Audio available to play";

    }
    else
    {
        qDebug() << "Audio not available to play";
    }
}

void MediaPlayerTest::onBufferStatusChanged(int percentFilled)
{
    MWTS_ENTER;
    qDebug() << "Buffer filled in: " << percentFilled << "%";
}

void MediaPlayerTest::onDurationChanged(qint64 duration)
{
    MWTS_ENTER;
    qDebug() << "Duration change to: " << duration;
}

void MediaPlayerTest::onMediaChanged(const QMediaContent& media)
{
    MWTS_ENTER;
    qDebug() << "Media changed to " << media.canonicalUrl().toString();
}

void MediaPlayerTest::onMutedChanged(bool muted)
{
    if (muted)
    {
        qDebug() << "Media file muted";

    }
    else
    {
        qDebug() << "Media file not muted";
    }
}

void MediaPlayerTest::onPlaybackRateChanged(qreal rate)
{
    MWTS_ENTER;
    qDebug() << "Playback rate changed to: " << rate;
}

void MediaPlayerTest::onPositionChanged (qint64 position)
{
    //MWTS_ENTER;
    qDebug() << "Position changed to: " << position << "/" << player->duration() << "ms";
    qDebug() << "State: " << player->state();
}

void MediaPlayerTest::onSeekableChanged(bool seekable)
{
    if (seekable)
    {
        qDebug() << "Media file seekable";

    }
    else
    {
        qDebug() << "Media file not seekable";
    }
}

void MediaPlayerTest::onVideoAvailableChanged (bool videoAvailable)
{
    if (videoAvailable)
    {
        qDebug() << "Video available to play";

    }
    else
    {
        qDebug() << "Video not available to play";
    }
}

void MediaPlayerTest::onVolumeChanged (int volume)
{
    MWTS_ENTER;
    qDebug() << "Volume changed to: " << volume;
}

//

void MediaPlayerTest::onPlay()
{
    MWTS_ENTER;
    qDebug() << "Playing/Resuming " << player->media().canonicalUrl().toString();
    player->play();
    if (player->isMuted())
        player->setMuted(false);
}

void MediaPlayerTest::onStop()
{
    MWTS_ENTER;
    stopTimeout = 0;
    qDebug() << "Stopping playback " << player->media().canonicalUrl().toString();
    player->stop();
    if (stopEventBehaviour == StopAndPlay)
    {
        qDebug() << "Play from beggining after 5 sec";
        QTimer::singleShot(5000, this, SLOT(onPlay()));
    }
    else
    {
        g_pTest->Stop();
    }
}

void MediaPlayerTest::onPause(int duration)
{
    MWTS_ENTER;
    pauseTimeout = 0;
    qDebug() << "Pausing playback " << player->media().canonicalUrl().toString() << "for" << duration << "ms";
    player->pause();
    QTimer::singleShot(duration, this, SLOT(onPlay()));
}

void MediaPlayerTest::onSeek(int position)
{
    MWTS_ENTER;
    seekTimeout = 0;
    qDebug() << "Seeking position " << player->media().canonicalUrl().toString();
    player->setPosition(position);
}

void MediaPlayerTest::checkTimeouts()
{
    //MWTS_ENTER;
    int elapsed = g_pTime->elapsed();
    if (stopTimeout != 0 && elapsed >= (stopTimeout * 1000)) {
        emit stop();
    }
    if (pauseTimeout != 0 && elapsed >= (pauseTimeout * 1000)) {
        emit pause(pauseDuration*1000);
    }
    if (seekTimeout != 0 && elapsed >= (seekTimeout * 1000)) {
        emit seek(seekPosition*1000);
    }
}
