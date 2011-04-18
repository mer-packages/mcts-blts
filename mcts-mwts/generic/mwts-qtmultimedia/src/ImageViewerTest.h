/* ImageViewerTest.h
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

#ifndef IMAGE_VIEWER_TEST_H
#define IMAGE_VIEWER_TEST_H

#include <QMainWindow>
#include <QPrinter>
#include <QtGui>
#include <MwtsCommon>
#include <qmediaimageviewer.h>
#include <qvideowidget.h>

QT_BEGIN_NAMESPACE
class QMediaPlaylist;
//class QVideoWidget;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class ImageViewerTest : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor forImageViewerTest class
     */
    ImageViewerTest();

    /**
     * Destructor for ImageViewerTest class
     */
    virtual ~ImageViewerTest();

    /**
     * Function for MultiMediaTest class
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * Function for MultiMediaTest class
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();


    /**
     * Displays image/s which were set with SetImageTimeout method
     */
    void PlayImageViewer();

    /**
     * Sets the timeout of the image playing
     * @param image playing timeout in milliseconds
     */
     void SetImageTimeout(int milliseconds);

     /**
      * Sets directory where images are of path to image file
      * @param path directory/file path
      */
     void SetImageViewerPath(QString path);

private:
    //The main object used for image viewing
    QMediaImageViewer *viewer;
    //Object for playlist setting up, later binding to viewer
    QMediaPlaylist *playlist;
    //Object for playlist setting up, later binding to viewer
    QVideoWidget *display;

    /**
     * Gets directory/filename path as a  paramater
     * and add the image(s) to the playlist
     *
     *
     * @param dir/file path to the images(s)
     */
    void openDirectory(QString directory);

private slots:

    /**
    * Displays the current QMediaImageViewer state (played, stopped, paused)
    * @param state
    */

    void stateChanged(QMediaImageViewer::State state);
    /**
    * Displays the current QMediaImageViewer status (loaded, loading, invalid, no media)
    * @param status
    */

    void statusChanged(QMediaImageViewer::MediaStatus status);
    /**
    * Displays the elapsed time per image
    * @param time
    */
    void elapsedTimeChanged(int time);

    /**
    * Displays the current playlist media
    * @param content
    */
    void currentMediaChanged(const QMediaContent &content);

};

#endif //#ifndef _IMAGE_VIEWER_TEST_H

