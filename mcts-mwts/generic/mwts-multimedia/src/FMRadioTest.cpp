/* FMRadioTest.cpp
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
#include "FMRadioTest.h"
#include <QTimer>
#include <QPair>

FMRadioTest::FMRadioTest()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

FMRadioTest::~FMRadioTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void FMRadioTest::OnInitialize()
{
    MWTS_ENTER;

    radio = new QRadioTuner();

    connect(radio, SIGNAL(bandChanged(QRadioTuner::Band)), this, SLOT(onBandChanged(QRadioTuner::Band)));
    connect(radio, SIGNAL(frequencyChanged(int)), this, SLOT(onFrequencyChanged(int)));
    connect(radio, SIGNAL(stateChanged(QRadioTuner::State)), this, SLOT(onStateChanged(QRadioTuner::State)));
    connect(radio, SIGNAL(mutedChanged(bool)), this, SLOT(onMutedChanged(bool)));
    connect(radio, SIGNAL(error(QRadioTuner::Error)), this, SLOT(onError(QRadioTuner::Error)));
    connect(radio, SIGNAL(signalStrengthChanged(int)), this, SLOT(onSignalStrengthChanged(int)));
    connect(radio, SIGNAL(searchingChanged(bool)), this, SLOT(onSearchingChanged(bool)));
    connect(radio, SIGNAL(stereoStatusChanged(bool)), this, SLOT(onStereoStatusChanged(bool)));
    connect(radio, SIGNAL(volumeChanged(int)), this, SLOT(onVolumeChanged(int)));

    if (radio->isAvailable())
    {
        qDebug() << "Radio available";
    }
    /*else
    {
        qCritical() << "No radio found";
        g_pResult->StepPassed("No radio found", false);
        return;
    }*/

    //this is workaround, because it seems that searchingChanged is not emmited
    QTimer* t = new QTimer();
    connect(t, SIGNAL(timeout()), this, SLOT(checkSearching()));
    t->start(100);


    SetVolume(g_pConfig->value("FMRADIO/volume").toInt());
    //SetFrequency(g_pConfig->value("FMRADIO/frequency").toInt());
    duration = g_pConfig->value("FMRADIO/duration").toInt();;


    MWTS_LEAVE;
}

void FMRadioTest::OnUninitialize()
{
    MWTS_ENTER;

    if(radio)
    {
        delete radio;
        radio = NULL;
    }

    MWTS_LEAVE;
}

void FMRadioTest::SetFrequency(int frequency)
{
    MWTS_ENTER;

    radio->setFrequency(frequency);

    qDebug () << "Freuency set to: " << radio->frequency()/1000 << " kHz";

    MWTS_LEAVE;
}

void FMRadioTest::ScanUp()
{
     MWTS_ENTER;

     radio->searchForward();

     MWTS_LEAVE;
}

void FMRadioTest::ScanDown()
{
    MWTS_ENTER;

    radio->searchBackward();

    MWTS_LEAVE;
}

void FMRadioTest::SetVolume (int volume)
{
    MWTS_ENTER;

    radio->setVolume(volume);

    MWTS_LEAVE;
}

void FMRadioTest::SetRadioDuration (int millisecond)
{
    duration = millisecond;
    qDebug () << "Radio playing duration: " << duration;
}

void FMRadioTest::PlayRadio()
{
    MWTS_ENTER;

    if (!IsRadioAvailable())
        return;

    radio->start();
    QTimer::singleShot(duration, g_pTest, SLOT(Stop()));
    g_pTest->Start();
    radio->stop();

    MWTS_LEAVE;
}

void FMRadioTest::SetBand(QRadioTuner::Band band)
{
    MWTS_ENTER;

    if(radio->isBandSupported(band))
    {
        radio->setBand(band);
        QPair<int, int> pair = radio->frequencyRange (band);
        minFrequency = pair.first;
        maxFrequency = pair.second;
        qDebug () << "Band set to " << band;
    }
    else
    {
        qCritical() << "Band not supported";
        g_pResult->StepPassed("Band not supported", false);
    }

    MWTS_LEAVE;
}

void FMRadioTest::SetScanMode(ScanMode mode) {

    MWTS_ENTER;

    scanMode = mode;
    switch (mode)
    {
    case ContinueMode:
        qDebug () << "ScanMode set to Continue Mode";
        break;
    case StopMode:
        qDebug () << "ScanMode set to Stop Mode";
        break;
    default:
        qDebug() << "ScanMode set to Unknown mode";
        break;
    }


    MWTS_LEAVE;
}

/**
 * Slots for QRadioTunerControl signals
 */

void FMRadioTest::onBandChanged(QRadioTuner::Band band)
{
    MWTS_ENTER;
    switch (band)
    {
    case QRadioTuner::FM:
        qDebug() << "band change to FM";
        break;
    default:
        qDebug() << "band change to different than FM";
        break;
    }

    MWTS_LEAVE;
}

void FMRadioTest::onFrequencyChanged(int freqency)
{
    //MWTS_ENTER;
    //qDebug() << "frequncy changed to " << freqency;
    if (scanMode == ContinueMode && freqency == maxFrequency)
    {
        g_pTest->Stop();
        radio->stop();
    }

    //MWTS_LEAVE;
}

void FMRadioTest::onStateChanged(QRadioTuner::State state)
{
    MWTS_ENTER;
    qDebug() << "radio state changed to: " << state;
    MWTS_LEAVE;
}

void FMRadioTest::onMutedChanged(bool muted)
{
    MWTS_ENTER;

    if (muted)
        qDebug() << "muted";
    else
        qDebug() << "not muted";

    MWTS_LEAVE;
}

void FMRadioTest::onError(QRadioTuner::Error error)
{
    MWTS_ENTER;
    qCritical() << "Error" << radio->errorString();
    g_pResult->StepPassed("Error" + radio->errorString(), false);
    MWTS_LEAVE;
}

void FMRadioTest::onSignalStrengthChanged(int strength)
{
    //MWTS_ENTER;

    //qDebug() << "signal strenght changed: " << strength;

    //MWTS_LEAVE;
}

//this signal is not emitted by Qt M ???
void FMRadioTest::onSearchingChanged(bool searching)
{
    MWTS_ENTER;

    if (searching) {
        qDebug() << "searching";
    }
    else {
        qDebug() << "not searching";
        //radio->searchForward();
    }

    MWTS_LEAVE;
}

void FMRadioTest::onVolumeChanged(int volume)
{
    MWTS_ENTER;
    qDebug() << "volume changed: " << volume;
    MWTS_LEAVE;
}

void FMRadioTest::onStereoStatusChanged(bool stereo)
{
    MWTS_ENTER;

    if (stereo)
        qDebug() << "is stereo";
    else
        qDebug() << "is not stereo";

    MWTS_LEAVE;
}

//for workaround
void FMRadioTest::checkSearching()
{
    //MWTS_ENTER;

    if (radio->isSearching())
    {
        //qDebug() << "searching";
    }
    else
    {
        //qDebug() << "not searching";

        if (scanMode == ContinueMode)
        {
            QString msg = "Station found on " + QString::number((double)radio->frequency() / 1000000.0, 'f', 3) + " MHz, signal strength " + QString::number(radio->signalStrength());
            qDebug() << msg;
            g_pResult->Write(msg);
            ScanUp();
        }
        else if (scanMode == StopMode)
        {

        }
    }

    //MWTS_LEAVE;
}

void FMRadioTest::PerformBandScan()
{
    MWTS_ENTER;

    if (!IsRadioAvailable())
        return;

    if (!g_pResult->IsPassed())
        return;

    QString msg = "Performing band scan, frequency from " + QString::number((double)minFrequency / 1000000) + " MHz to " + QString::number((double)maxFrequency / 1000000) + " MHz";
    qDebug() << msg;
    g_pResult->Write(msg);

    SetFrequency(minFrequency);
    radio->start();

    msg = "Starting measure...";
    qDebug() << msg;
    g_pResult->Write(msg);

    g_pTime->start();

    ScanUp();

    g_pTest->Start();

    int elapsedTime = g_pTime->elapsed();
    msg = "Time elapsed during scanning: " + QString::number(elapsedTime) + " ms";
    qDebug() << msg;
    g_pResult->Write(msg);

    MWTS_LEAVE;
}

bool FMRadioTest::IsRadioAvailable() const
{
    bool ret;
    if (!radio->isAvailable())
    {
        qCritical() << "No radio found";
        g_pResult->StepPassed("No radio found", false);
        ret = false;
    }
    else
    {
        ret = true;
    }
    return ret;
}

