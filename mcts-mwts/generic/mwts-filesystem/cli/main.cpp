/* main.cpp
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
#include "FilesystemTest.h"

int main( int argc, char** argv )
{
	qDebug("new FilesystemTest()");
	FilesystemTest* test = new FilesystemTest();
	qDebug("done new FilesystemTest()");

	//	mwts Init
	qDebug("test->Initialize()");
    test->Initialize();

	//mwts SelectSource ROOT
	qDebug("test->SelectSourceDevice(DEV_ROOT)");
    test->SelectSourceDevice(DEV_ROOT);

    //mwts SelectTarget USBMSD_MMC
	qDebug("test->SelectTargetDevice(DEV_USBMSD_MMC)");
    test->SelectTargetDevice(DEV_USBMSD_MMC);

    //mwts SetBlockSize 1024
 	qDebug("test->SetBlockSize(1024)");
    test->SetBlockSize(1024);

    //mwts SetBlockCount 100000
 	qDebug("test->SetBlockCount(100000)");
    test->SetBlockCount(100000);

	//mwts CopySpeed
 	qDebug("test->MeasureCopySpeed()");
    test->MeasureCopySpeed();

	//mwts Close
//	qDebug("test->Close()");
//    test->Close();

	qDebug("delete test");
    delete test;

	qDebug("return 0");
	return 0;
}
