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

#include "ofonotest.h"
#include "stable.h"
#include <QtTest/QtTest>
#define MWTS_SIM_CONF_DEBUG

OfonoTest::OfonoTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

OfonoTest::~OfonoTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void OfonoTest::OnInitialize()
{
    MWTS_ENTER;
  //m = new OfonoSimManager(OfonoModem::ManualSelect, "/phonesim", this);
    m = new OfonoSimManager(OfonoModem::ManualSelect, "/phonesim");
    qDebug () << __FUNCTION__ << "OfonoModem::ManualSelect, /phonesim";

    if (!m->modem()->powered())
    {
        qDebug () << __FUNCTION__ << "not powered on";
        m->modem()->setPowered(true);
     //   sleep(5);
        QTest::qWait(5000);
    }
    if (!m->modem()->online())
    {
        qDebug () << __FUNCTION__ << "not online";
        m->modem()->setOnline(true);
        //sleep(5);
        QTest::qWait(5000);
    }

    if (m->isValid())
    {
        qDebug () << __FUNCTION__ << "modem is valid";
    }
    else
    {
        qCritical ("modem is not valid");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        return;
    }

        qDebug () << __FUNCTION__ << " modem is set :" << m->modem()->powered()
                                  << "\nmodem is set"  << m->modem()->online();
}

void OfonoTest::OnUninitialize()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

bool OfonoTest::changePin(const QString &pinType, const QString &oldPin, const QString &newPin)
{
    MWTS_ENTER;
    QSignalSpy changePin(m, SIGNAL(changePinComplete(bool)));
    m->requestChangePin(pinType, oldPin, newPin);
   // sleep(1);
    QTest::qWait(1000);

    if (changePin.takeFirst().at(0).toBool())
    {
       return TRUE;
    }
    else
    {
      return FALSE;
    }
}

bool OfonoTest::enablePin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;
    QSignalSpy lockPin(m, SIGNAL(lockPinComplete(bool)));
    m->requestLockPin(pinType, pin);
    //sleep(1);
    QTest::qWait(1000);

    if (lockPin.takeFirst().at(0).toBool())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

bool OfonoTest::disablePin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;
    QSignalSpy unlockPin(m, SIGNAL(unlockPinComplete(bool)));
    m->requestUnlockPin(pinType, pin);
    //sleep(1);
    QTest::qWait(1000);

    if (unlockPin.takeFirst().at(0).toBool())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
