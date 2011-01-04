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

class OfonoTest : public MwtsTest
{
    Q_OBJECT
public:    
    /**
     * Constructor for template test class
     */
    OfonoTest();

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
     */
    void OnUninitialize();

    /**
     * Changes the pin given by string type.
     * @param pin type, old pin code, new pin code
     * @return true if the change was successful
     */
    bool changePin(const QString &pinType, const QString &oldPin, const QString &newPin);

    //TODO on request
    //bool enterPin(const QString &pinType, const QString &pin);
    //bool resetPin(const QString &pinType, const QString &puk, const QString &newPin);

    /**
     * Activates the lock for the particular pin type. The
     * device will ask for a PIN automatically next time the
     * device is turned on or the SIM is removed and
     * re-inserter. The current PIN is required for the
     * operation to succeed.
     * @param pin type, current pin code
     * @return true on success, otherwise false
     */
    bool enablePin (const QString &pinType, const QString &pin);

    /**
     * Deactivates the lock for the particular pin type. The
     * current PIN is required for the operation to succeed.
     * @param pin type, current pin code
     * @return true on success, otherwise false
     */
    bool disablePin (const QString &pinType, const QString &pin);

private:
    OfonoSimManager *m;
};

#endif // OFONOTEST_H
