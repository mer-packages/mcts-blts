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

    vol = g_pConfig->value("FMRADIO/volume").toInt();
    freq = g_pConfig->value("FMRADIO/frequency").toInt();
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

void FMRadioTest::SetFrequency(int freq)
{
    MWTS_ENTER;

    radio = new QRadioTuner;

    connect(radio,SIGNAL(frequencyChanged(int)),
        this,SLOT(freqChanged(int)));
    connect(radio,SIGNAL(signalStrengthChanged(int)),
        this,SLOT(signalChanged(int)));
    connect(radio, SIGNAL(error (QRadioTuner::Error)),
        this, SLOT(error(QRadioTuner::Error)));
    connect(radio, SIGNAL(error(QRadioTuner::Error)),
        this, SLOT(error(QRadioTuner::Error)));

    if(radio->isBandSupported(QRadioTuner::FM))
        radio->setBand(QRadioTuner::FM);

    if (radio->isAvailable())
    {
        qDebug() << "No Signal";
    }
    else
    {
        qCritical() << "No radio found";
        g_pResult->StepPassed("No radio found!", false);
    }


    radio->setFrequency(freq);

    qDebug () << "Initial frequency: " << radio->frequency()/1000 << " kHz";
    qDebug () << "Initial radio playing duration: " << duration;

    MWTS_LEAVE;
    return;
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
    duration=millisecond;
    qDebug () << "Radio playing duration: " << duration;
}

void FMRadioTest::PlayRadio()
{
    MWTS_ENTER;

    radio->start();
    QTimer::singleShot(duration, g_pTest, SLOT(Stop()));
    g_pTest->Start();
    radio->stop();

    MWTS_LEAVE;
}

void FMRadioTest::freqChanged(int)
{
    MWTS_ENTER;

    qDebug () << radio->frequency()/1000 << " kHz";

    MWTS_LEAVE;
}

void FMRadioTest::signalChanged(int)
{
    MWTS_ENTER;

    if(radio->signalStrength() > 25)
        qDebug () << "Got Signal";
    else
        qDebug () << "No Signal";

    MWTS_LEAVE;
}

void FMRadioTest::error(QRadioTuner::Error error)
{
    MWTS_ENTER;

    qWarning().nospace() << "Warning: received error QRadioTuner:" << radio->errorString();

    MWTS_LEAVE;
}

void FMRadioTest::stateChanged(QRadioTuner::State state)
{
    MWTS_ENTER;

    switch (state)
    {
        case QRadioTuner::ActiveState:
            qDebug() << "QRadioTuner is in active state.";
            break;
        case QRadioTuner::StoppedState:
            qDebug() << "QRadioTuner is in stopped state.";
            break;
    }

    MWTS_LEAVE;
}
