/* AudioPlayerTest.cpp
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
#include "AudioPlayerTest.h"
//#include <QDir>


AudioPlayerTest::AudioPlayerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

AudioPlayerTest::~AudioPlayerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void AudioPlayerTest::OnInitialize()
{
    MWTS_ENTER;

    player = new QMediaPlayer();
    this->SetVolume(g_pConfig->value("PLAYER/volume").toInt());

    playbackDuration = 4000;

    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(onError(QMediaPlayer::Error)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onStateChanged(QMediaPlayer::State)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(audioAvailableChanged(bool)), this, SLOT(onAudioAvailableChanged(bool)));

    MWTS_LEAVE;
}

void AudioPlayerTest::OnUninitialize()
{
    MWTS_ENTER;

    if (player)
    {
        delete player;
        player = NULL;
    }

    MWTS_LEAVE;
}

/*
 * Test functions
 */

void AudioPlayerTest::play()
{
    MWTS_ENTER;

    MWTS_DEBUG("Attempt to play " + player->media().canonicalUrl().toString() + "(" + player->duration() + "ms)" + ", with volume " + player->volume());

    player->play();

    QTimer::singleShot(playbackDuration, g_pTest, SLOT(Stop()));
    g_pTest->Start();

    MWTS_LEAVE;
}


void AudioPlayerTest::SetMedia(const QString& filePath)
{
    MWTS_ENTER;

    player->setMedia(QUrl::fromLocalFile(filePath));
    MWTS_DEBUG ("Set media file: " + filePath);

    MWTS_LEAVE;
}


void AudioPlayerTest::SetVolume(int volume)
{
    MWTS_ENTER;
    if (volume > 100 || volume < 0)
    {
        qCritical() << "100 >= Positive number >= 0 is expected.";
    }
    player->setVolume(volume);
    MWTS_DEBUG ("Set volume: " + volume);
    MWTS_LEAVE;
}

void AudioPlayerTest::SetPlaybackDuration(int msec)
{
    MWTS_ENTER;
    if (msec <= 0)
    {
        qCritical() << "Duration should be greater than 0. Recommended value is at least 2000 ms";
    }
    playbackDuration = msec;
    MWTS_DEBUG ("Set playback duration: " + msec);
    MWTS_LEAVE;
}

/* private slots */


void AudioPlayerTest::onError(QMediaPlayer::Error error)
{
    qCritical() << "Error has occured:" + player->errorString() << ",error code " << error;
}


void AudioPlayerTest::onStateChanged(QMediaPlayer::State state)
{
    switch (state)
    {
        case QMediaPlayer::PlayingState:
            MWTS_DEBUG("Playing " + player->media().canonicalUrl().toString() + " audio file");
            break;
         case QMediaPlayer::PausedState:
            MWTS_DEBUG("Paused " + player->media().canonicalUrl().toString() + " audio file");
            break;
        case QMediaPlayer::StoppedState:
            MWTS_DEBUG("Stopped " + player->media().canonicalUrl().toString() + " audio file");
            break;
        default:
            MWTS_DEBUG("Unknown state");
    }
}

void AudioPlayerTest::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    MWTS_DEBUG("Media statud changed to: " + status);

}

void AudioPlayerTest::onAudioAvailableChanged(bool available)
{
    if (available)
    {
        MWTS_DEBUG("audio available to play");

    }
    else
    {
        MWTS_DEBUG("audio not available to play");
    }
}

