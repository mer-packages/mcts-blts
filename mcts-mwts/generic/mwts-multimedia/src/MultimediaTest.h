/* MultimediaTest.h
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

/** @mainpage MWTS-MULTIMEDIA
 *
 * @section links
 *
 * For detailed informations check README file on @link gitorious http://gitorious.org/~tumi/qa-tools/mcts-mwts-dev/trees/master/mcts-mwts/generic/mwts-multimedia @endlink
 */


#ifndef _INCLUDED_MULTIMEDIA_TEST_H
#define _INCLUDED_MULTIMEDIA_TEST_H

#include <MwtsCommon>
#include "AudioRecorderTest.h"
#include "ImageViewerTest.h"
#include "FMRadioTest.h"
#include "AudioPlayerTest.h"

class MultimediaTest : public MwtsTest
{
    Q_OBJECT
public:
    /**
     * Constructor for template test class
     */
    MultimediaTest();

    /**
     * Destructor for template test class
     */
    virtual ~MultimediaTest();

    /**
     * Overridden functions for MwtsTest class
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * Overridden functions for MwtsTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

public:
    ImageViewerTest *imageViewer;
    AudioRecorderTest *audioRecorder;
    FMRadioTest *fmRadio;
    AudioPlayerTest* audioPlayer;
};

#endif //#ifndef _INCLUDED_MULTIMEDIA_TEST_H

