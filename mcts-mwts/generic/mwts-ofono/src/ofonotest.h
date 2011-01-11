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

#ifndef OFONOTEST_H
#define OFONOTEST_H

#include <MwtsCommon>
#include <ofono-qt/ofonosimmanager.h>
#include <QtTest/QtTest>

//for debug purposes
#define MWTS_OFONO_CONF_DEBUG

//on armv7 n900 images sometime the available libofono-qt version is 0.0.7-2.14 which
//has different public method names (not refactored)
//#define MWTS_SIMMANAGER_OLD

class OfonoTest : public MwtsTest
{
    Q_OBJECT

public:    
    /**
     * Constructor for template test class
     */
    OfonoTest(QObject* pParent = 0 );

    /**
     * Destructor for template test class
     */
    virtual ~OfonoTest();


    /**
     * Overridden functions for MwtsTest class
     * OnInitialize is called before test execution
     */

    void OnInitialize();

    /**
     * Overridden functions for MwtsTest class
     * OnUninitialize is called after test execution
     * Delete SimManager, QEventLoop, QTimer from the heap
     * beside takes care of keeping the original state on the sim
     * original state: PIN, PUK are disabled (unlocked)
     */
    void OnUninitialize();

    /**
     * Enters the currently pending pin.  The type value must
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
     * Verifies the invalidy or validity of the pin code.
     *
     * @param invalid/valid, pin type, current pin code
     * @return true on success, otherwise false
     */
    bool verifyPin(const QString validity, const QString pinType, const QString pin);

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

private Q_SLOTS:
    void testTimeout();

    /**
     * If the pending pin is required (after reboot), it enters that before running any other method
     *
     */
    void onEnterPinRequired(const QString&, const QString&);

private:
    OfonoSimManager *mSimManager;
    QEventLoop      *mEventLoop;
    QTimer          *mTimer;
};

#endif // OFONOTEST_H
