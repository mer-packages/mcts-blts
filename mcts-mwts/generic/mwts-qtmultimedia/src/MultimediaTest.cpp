/* MultimediaTest.cpp
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
#include "MultimediaTest.h"

/**
    sdsdsdsd
*/

MultimediaTest::MultimediaTest()
{        
    MWTS_ENTER;

    //audioRecorder = new AudioRecorderTest;
    //imageViewer = new ImageViewerTest;
    //fmRadio = new FMRadioTest;
    //mediaPlayer = new MediaPlayerTest;

    MWTS_LEAVE;
}

MultimediaTest::~MultimediaTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void MultimediaTest::OnInitialize()
{
    MWTS_ENTER;

    //audioRecorder->OnInitialize();
    //imageViewer->OnInitialize();
    //fmRadio->OnInitialize();
    //mediaPlayer->OnInitialize();

    audioRecorder = NULL;
    imageViewer = NULL;
    fmRadio = NULL;
    mediaPlayer = NULL;

    MWTS_LEAVE;
}

void MultimediaTest::OnUninitialize()
{
    MWTS_ENTER;

    if(imageViewer)
    {
        imageViewer->OnUninitialize();
        delete imageViewer;
        imageViewer = NULL;
    }

    if(audioRecorder)
    {
        audioRecorder->OnUninitialize();
        delete audioRecorder;
        audioRecorder = NULL;
    }

    if(fmRadio)
    {
        fmRadio->OnUninitialize();
        delete fmRadio;
        fmRadio = NULL;
    }

    if (mediaPlayer)
    {
        mediaPlayer->OnUninitialize();
        delete mediaPlayer;
        mediaPlayer = NULL;
    }

    MWTS_LEAVE;
}

void MultimediaTest::EnableFeature(featureName name)
{
    switch (name)
    {
    case audioRecorderFeature:
        audioRecorder = new AudioRecorderTest;
        audioRecorder->OnInitialize();
        break;
    case imageViewerFeature:
        imageViewer = new ImageViewerTest;
        imageViewer->OnInitialize();
        break;
    case radioPlayerFeature:
        fmRadio = new FMRadioTest;
        fmRadio->OnInitialize();
        break;
    case mediaPlayerFeature:
        mediaPlayer = new MediaPlayerTest;
        mediaPlayer->OnInitialize();
        break;
    default:
        break;
    }
}



