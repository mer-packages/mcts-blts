/* FMRadioTest.h
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

#ifndef _INCLUDED_FM_RADIO_TEST_H
#define _INCLUDED_FM_RADIO_TEST_H

#include <MwtsCommon>
#include <qradiotuner.h>

#ifdef MWTS_MULTIMEDIA_DEBUG
    #define MWTS_FM_RADIO_DEBUG
#endif

class FMRadioTest : public QObject
{
    Q_OBJECT

public:

    /**
     *  This enum indicates if scanning will be stoped if signal is strong
     *  or will continue searching just after stopping.
     */
    enum ScanMode {StopMode, ContinueMode};

    /**
     * Constructor for FMRadioTest class
     */
    FMRadioTest();

    /**
     * Destructor for FMRadioTest class
     */
    virtual ~FMRadioTest();

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
     * Sets the frequency in Hertz.
     * @param frequency Hz
     */
    void SetFrequency(int frequency);

    /**
     * Starts a foreward scan for a signal, starting from the current frequency
     */
    void ScanUp();

    /**
     * Starts a backward scan for a signal, starting from the current frequency
     */
    void ScanDown();


    /**
     * Set the radio volume percentage.
     * @param volume  as 0 <= integer number <= 100)
     */
    void SetVolume(int volume);

    /**
     * Starts to play the radio on the set frequency for a certain time
     */
    void PlayRadio();

    /**
     * Sets the duration of the radio playing
     * @param radio playing duration in milliseconds
     */
    void SetRadioDuration(int millisecond);

    /**
     * Sets radio band. This also sets minFrequency and maxFrequency values.
     * @param band to set
     */
    void SetBand(QRadioTuner::Band band);

    /**
     * Sets scan mode.
     * @param mode mode to set
     */
    void SetScanMode(ScanMode mode);

    /**
     * Starts scanning the whole renge of set band.
     * Log founded radio stations (freqency and signal stregth)
     */
    void PerformBandScan();

private:
    //The main object used for radio playing
    QRadioTuner* radio;
    //Integer holds the the initial, later set duration value
    int duration;
    //scan mode
    ScanMode scanMode;
    //min and max frequecies of band
    int minFrequency;
    int maxFrequency;


private slots:
    //slots for QRadioTuner signals
    void onBandChanged(QRadioTuner::Band band);
    void onFrequencyChanged(int freqency);
    void onStateChanged(QRadioTuner::State state);
    void onMutedChanged(bool muted);
    void onError(QRadioTuner::Error error);
    void onSignalStrengthChanged(int strength);
    void onSearchingChanged(bool searching);
    void onVolumeChanged(int volume);
    void onStereoStatusChanged(bool stereo);
    //slot for searching changed signal emitting workaround
    void checkSearching();

};


#endif //#ifndef _INCLUDED_FM_RADIO_TEST_H
