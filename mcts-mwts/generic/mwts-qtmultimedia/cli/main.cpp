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
#include "MultimediaTest.h"

#include <QApplication>
int main( int argc, char** argv )
{
    qDebug() << "In main function";
    MultimediaTest test;
    MwtsApp::EnableUI(true);
    //test.SetCaseName("MT1");

    test.Initialize();

    test.imageViewer->SetImageTimeout(10000);
    test.imageViewer->SetImageViewerPath("/home/user/MyDocs/testdata/Image/JPEG/JPEG_2304x1296(3MPix)_24BPP_BigBuckBunny_1.2MB.jpg");
    test.imageViewer->PlayImageViewer();

    test.Uninitialize();
    return 0;
}
