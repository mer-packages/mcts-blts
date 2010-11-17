/* main.cpp -- Implementation of command line interface.
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
#include <MwtsCommon>

#include "GCameraTest.h"


/**
 * Main binary to test functionality of camera easily
 * Just compile, upload, install and run ltap-camera
 *
 */

gint main(int argc, char *argv[])
{

    GOptionContext *context;
    GError *error = NULL;

    GCameraTest *test= new GCameraTest;

    test->SetCaseName("mwts-gcamera");
    test->Initialize();
    test->setup_codecs("theoraenc", "oggmux", "pulsesrc", "vorbisenc", "ogg");
    test->take_video(3);
    test->Uninitialize();

    return 0;
}
