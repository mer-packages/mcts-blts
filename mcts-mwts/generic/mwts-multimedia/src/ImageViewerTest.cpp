/* ImageViewerTest.cpp
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

#include "ImageViewerTest.h"
#include "stable.h"
#include <qmediaplaylist.h>
#include <QDir>

ImageViewerTest::ImageViewerTest() 
{
    //MwtsApp::EnableUI(true);

    MWTS_ENTER;
    MWTS_LEAVE;
}

ImageViewerTest::~ImageViewerTest()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

void ImageViewerTest::OnInitialize()
{
    MWTS_ENTER;

    duration = g_pConfig->value("VIEWER/duration").toInt();

    MWTS_LEAVE;
}

void ImageViewerTest::OnUninitialize()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

void ImageViewerTest::PlayImageViewer(QString path)
{
    MWTS_ENTER;
    tmppath=path;
    QTimer::singleShot(100, this, SLOT(startPlayImageViewer()));
    g_pTest->Start();

    MWTS_LEAVE;
}

void ImageViewerTest::openDirectory(QString path)
{
    MWTS_ENTER;
    QFileInfo fileInfo (path);
    if(fileInfo.isDir())
    {
            QDir dir(path);
            qDebug() << "Directory path" << path;
            if(!playlist->isEmpty())
            {
                playlist->clear();
            }
            foreach (const QString &fileName, dir.entryList(QDir::Files))
            {
                playlist->addMedia(QUrl::fromLocalFile(dir.absoluteFilePath(fileName)));
                qDebug () << "Added file: " << dir.absoluteFilePath(fileName);
            }
        }
    else if(fileInfo.isFile())
    {
            QFile file (path);
            qDebug() << "File path " << path;

            if(!playlist->isEmpty())
            {
                playlist->clear();
            }

            playlist->addMedia(QUrl::fromLocalFile(path));
            qDebug() << "Added file:" << path;
    }
    else
    {
        qDebug() << "This directory/file path: " << path << "is not accessable.";
        return;
    }
}

void ImageViewerTest::SetImageTimeout(int milliseconds)
{
    MWTS_ENTER;

    duration = milliseconds;

    MWTS_LEAVE;
}

void ImageViewerTest::stateChanged(QMediaImageViewer::State state)
{
    switch(state)
    {
        case QMediaImageViewer::StoppedState:
            qDebug() << "Stopped state";
            g_pTest->Stop();
            //g_pResult->Write("Succesfully shown the image!");
            //g_pResult->StepPassed("show", true);
            break;
        case QMediaImageViewer::PlayingState:
            qDebug() << "Playing state";
            break;
        case QMediaImageViewer::PausedState:
            qDebug() << "Paused state";
            break;
    }
}

void ImageViewerTest::statusChanged(QMediaImageViewer::MediaStatus status)
{
    switch(status) {
        case QMediaImageViewer::NoMedia:
            qDebug() << "No image" << playlist->mediaCount();
            break;
        case QMediaImageViewer::LoadingMedia:
            qDebug() << "Loading image" << playlist->currentIndex() << " of " << playlist->mediaCount();
            break;
        case QMediaImageViewer::LoadedMedia:
            qDebug() << "Loaded image" << playlist->currentIndex() << " of " << playlist->mediaCount();
            break;
        case QMediaImageViewer::InvalidMedia:
            qCritical() << "Invalid image " << playlist->currentIndex() << " of " << playlist->mediaCount();
            g_pTest->Stop();
            g_pResult->Write("Invalid image!");
            g_pResult->StepPassed("Invalid Image, step failed (GIF, AnimGIF)", false);
            return;
        default:
            break;
    }
}

void ImageViewerTest::elapsedTimeChanged(int time)
{
    const int remaining = (viewer->timeout() - time) / 1000;
    qDebug() << "Time remaining" << remaining;

}

void ImageViewerTest::currentMediaChanged(const QMediaContent &content)
{
    qDebug() << "Playlist item: " << content.canonicalUrl();
}

void ImageViewerTest::startPlayImageViewer()
{
    viewer = new QMediaImageViewer(this);
    display = new QVideoWidget(g_pMainWindow);
    playlist = new QMediaPlaylist(viewer);

    viewer->bind(display);
    viewer->bind(playlist);

    viewer->setTimeout(duration);
    qDebug() << "Image displayed duration time:" << viewer->timeout();

    g_pMainWindow->setCentralWidget(display);
    g_pMainWindow->setWindowTitle(tr("Image Viewer"));
    g_pMainWindow->showFullScreen();

    connect(viewer, SIGNAL(stateChanged(QMediaImageViewer::State)),
            this, SLOT(stateChanged(QMediaImageViewer::State)));
    connect(viewer, SIGNAL(mediaStatusChanged(QMediaImageViewer::MediaStatus)),
            this, SLOT(statusChanged(QMediaImageViewer::MediaStatus)));
    connect(viewer, SIGNAL(elapsedTimeChanged(int)),
            this, SLOT(elapsedTimeChanged(int)));    
    connect(playlist, SIGNAL(currentMediaChanged(QMediaContent)),
            this, SLOT(currentMediaChanged(QMediaContent)));

    if (tmppath.isEmpty())
    {
        qDebug() << "Using default directory/file path";
        openDirectory(g_pConfig->value("VIEWER/path").toString());
    }
    else
    {
        qDebug() << "Using script with give path";
        openDirectory(tmppath);
    }

    qDebug () << "g_PMainWindow is visible:" << g_pMainWindow->isVisible();
    viewer->play();
}
