/*
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

#ifndef SIMMANAGERTEST_H
#define SIMMANAGERTEST_H

#include <ofono-qt/ofonosimmanager.h>
#include "ofonotestinterface.h"

//for debug purposes
#define MWTS_OFONO_CONF_DEBUG

//on armv7 n900 images sometime the available libofono-qt version is 0.0.7-2.14 which
//has different public method names (not refactored)
//#define MWTS_SIMMANAGER_OLD

class SimManagerTest :  public OfonoTestInterface
{
    Q_OBJECT

public:
    /**
     * Constructor for SimManagerTest test class
     */
    SimManagerTest(QObject* parent = 0) ;

    /**
     * Destructor for SimManagerTest test class
     */
    virtual ~SimManagerTest();


    /**
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /************************************************************
     *
     * Sim manager methods
     *
     */

    /**
     * Enters the currently pending pin (after reboot). The type value must
     * match the pin type being asked in the PinRequired
     * property.
     * @param pin type, pin code
     * @return true on success
     */
    bool enterPin(const QString &pinType, const QString &pin);

    /**
     * Changes the pin given by string type.
     * @param pin type, old pin code, new pin code
     * @return true if the change was successful
     */
    bool changePin(const QString &pinType, const QString &oldPin, const QString &newPin);

    /**
     * Activates the lock for the particular pin type. The
     * device will ask for a PIN automatically next time the
     * device is turned on or the SIM is removed and
     * re-inserter. The current PIN is required for the
     * operation to succeed.
     * @param pin type, current pin code
     * @return true on success, otherwise false
     */
    bool enablePin(const QString &pinType, const QString &pin);

    /**
     * Deactivates the lock for the particular pin type. The
     * current PIN is required for the operation to succeed.
     * @param pin type, current pin code
     * @return true on success, otherwise false
     */
    bool disablePin(const QString &pinType, const QString &pin);

    /**
     * Verifies the invalidity or validity of the pin code.
     *
     * @param invalid/valid, pin type, current pin code
     * @return true on success, otherwise false
     */
    bool verifyPin(const QString validity, const QString pinType, const QString pin);

    /**
     * Verifies the invalidity or validity of the puk code.
     *
     * @param invalid/valid, puk code
     * @return true on success, otherwise false
     */
    //seems the only way to verify the puk code somehow
    bool verifyPuk(const QString validity, const QString puk);

    /**
     * Provides the unblock key to the modem and if correct
     * resets the pin to the new value of newpin.
     * @param  pin type, puk code, new pin code
     * @return true on success, otherwise false
     */
    bool resetPin(const QString pinType, const QString puk, const QString newPin);

    /**
     * Prints info about the sim card (locked pins, pin required)
     *
     */
    void simInfo(void);

signals:
    /**
     * Signal if pending pin code is required
     *
     */
    void enterPinRequired(const QString&, const QString&);

private slots:
    /**
     * If the pending pin is required (after reboot), it enters that before running any other method
     *
     */
    void onEnterPinRequired(const QString&, const QString&);

private:
    bool invalidityCheck (const QString error);

    OfonoSimManager *mSimManager;
};

#endif // SIMMANAGERTESTH
