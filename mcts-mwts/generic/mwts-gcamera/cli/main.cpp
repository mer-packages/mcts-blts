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
 * Just compile, upload, install and run mwts-gcamera
 *
 */

gint main(int argc, char *argv[])
{

    GCameraTest *test= new GCameraTest;
    gst_init (NULL, NULL);

    test->SetCaseName("mwts-gcamera");
    test->Initialize();
    //test->OnInitialize();
    //test->setup_codecs("theoraenc", "oggmux", "pulsesrc", "vorbisenc", ".ogg");
    test->set_flags(GST_CAMERABIN_FLAG_SOURCE_COLORSPACE_CONVERSION);
    test->setup_pipeline();
    //test->set_fps();
    /*for (int i=0; i<10;i++)
    {
        test->take_video(2);
    }*/
    //test->take_video(5);
    //test->take_video(2);
    test->increase_zoom(500);
    test->take_picture(TRUE);

    test->Uninitialize();
    //test->OnUninitialize();
    delete test;
    test=NULL;
    return 0;
}
