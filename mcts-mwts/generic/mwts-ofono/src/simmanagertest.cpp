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

#include "simmanagertest.h"

SimManagerTest::SimManagerTest(QObject *parent)
    :OfonoTestInterface(parent)

{
    MWTS_ENTER;
    MWTS_LEAVE;
}

SimManagerTest::~SimManagerTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void SimManagerTest::OnInitialize()
{
    MWTS_ENTER;

    //qDebug() << "Config file name: " << g_pConfig->fileName();
    QString modem = g_pConfig->value("DEFAULT/modem").toString();
    qDebug () << __PRETTY_FUNCTION__ << "modem: " << modem;
    //TODO, might put on option for AUTOSELECT
    mSimManager = new OfonoSimManager(OfonoModem::ManualSelect, modem, this);

    connect( this, SIGNAL(enterPinRequired(const QString&, const QString&)),
             this, SLOT(onEnterPinRequired(const QString&, const QString&)));

    if (!mSimManager->modem()->powered())
    {
        qDebug () << __PRETTY_FUNCTION__ << "modem is not powered on";
        mSimManager->modem()->setPowered(true);

        mTimer->start(5000);
        mEventLoop->exec();
    }

    if (mSimManager->isValid())
    {
        qDebug () << __PRETTY_FUNCTION__ << "modem is valid: TRUE";
        g_pResult->StepPassed("modem is valid: ", TRUE);
    }
    else
    {
        qCritical ("modem is not valid");
        g_pResult->StepPassed(__PRETTY_FUNCTION__, FALSE);
        MWTS_LEAVE;
        return;
    }

        qDebug () << __PRETTY_FUNCTION__ << "modem is powered: " << mSimManager->modem()->powered();
        g_pResult->StepPassed("modem is powered: ", mSimManager->modem()->powered());

        MWTS_LEAVE;

}

void SimManagerTest::OnUninitialize()
{
    MWTS_ENTER;

    OfonoTestInterface::OnUninitialize();

    if (mSimManager)
    {
        delete mSimManager;
        mSimManager = NULL;
    }

    MWTS_LEAVE;
}

bool SimManagerTest::enterPin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;

    if (mSimManager->pinRequired() != pinType )
    {
        qCritical () << __PRETTY_FUNCTION__ << pinType << " is not required to enter.";
        MWTS_LEAVE;
        return FALSE;
    }

    QSignalSpy enterPin(mSimManager, SIGNAL(enterPinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestEnterPin(pinType, pin);
    #else
        mSimManager->enterPin(pinType, pin);
    #endif

    qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType << " pin code:" << pin;

    mTimer->start(1000);
    mEventLoop->exec();

    qDebug () << __PRETTY_FUNCTION__ << enterPin.count();
    if (enterPin.takeFirst().at(0).toBool())
    {
        qDebug () << __PRETTY_FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __PRETTY_FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }
}

bool SimManagerTest::changePin(const QString &pinType, const QString &oldPin, const QString &newPin)
{
    MWTS_ENTER;

    //check whether EnterPin is required or not
    emit enterPinRequired(pinType, oldPin);

    //if lock pin is not enabled
    if (!mSimManager->lockedPins().contains(pinType))
    {
        qDebug () << __PRETTY_FUNCTION__ <<
            pinType << "has to be enabled to change the code.Running enablePin ...";
        //enable the pin lock and call changePin again
        if (this->enablePin(pinType, oldPin))
        {
            qDebug () << __PRETTY_FUNCTION__ << "Running changePin again ...";
            return this->changePin(pinType, oldPin, newPin);
        }
    }

    QSignalSpy changePin(mSimManager, SIGNAL(changePinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestChangePin(pinType, oldPin, newPin);
    #else
        mSimManager->changePin(pinType, oldPin, newPin);
    #endif

    qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType
              << " old pin:" << oldPin << " new pin:" << newPin;

    mTimer->start(1000);
    mEventLoop->exec();

    qDebug () << __PRETTY_FUNCTION__ << changePin.count();
    if (changePin.takeFirst().at(0).toBool())
    {
        qDebug () << __PRETTY_FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __PRETTY_FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }
}

bool SimManagerTest::enablePin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;

    //check whether EnterPin is required or not
    emit enterPinRequired(pinType, pin);

    //check whether the pin is already enabled
    if (mSimManager->lockedPins().contains(pinType))
    {
        qDebug () << __PRETTY_FUNCTION__ << pinType << " is already enabled.";
        MWTS_LEAVE;
        return FALSE;
    }

    QSignalSpy lockPin(mSimManager, SIGNAL(lockPinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestLockPin(pinType, pin);
    #else
        mSimManager->lockPin(pinType, pin);
    #endif

    qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType << " pin code:" << pin;

    mTimer->start(1000);
    mEventLoop->exec();

    qDebug () << __PRETTY_FUNCTION__ << lockPin.count();
    if (lockPin.takeFirst().at(0).toBool())
    {
        qDebug () << __PRETTY_FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __PRETTY_FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }
}

bool SimManagerTest::disablePin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;

    //check whether EnterPin is required or not
    emit enterPinRequired(pinType, pin);

    //check whether the pin is already disabled
    if (!mSimManager->lockedPins().contains(pinType))
    {
        qCritical () << __PRETTY_FUNCTION__ << pinType << " is already disabled.";
        MWTS_LEAVE;
        return FALSE;
    }

    QSignalSpy unlockPin(mSimManager, SIGNAL(unlockPinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestUnlockPin(pinType, pin);
    #else
        mSimManager->unlockPin(pinType, pin);
    #endif

    qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType << " pin code:" << pin;

    mTimer->start(1000);
    mEventLoop->exec();

    qDebug () << __PRETTY_FUNCTION__ << unlockPin.count();
    if (unlockPin.takeFirst().at(0).toBool())
    {
        qDebug () << __PRETTY_FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __PRETTY_FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }
}

bool SimManagerTest::verifyPin(const QString validity, const QString pinType, const QString pin)
{
    MWTS_ENTER;
    bool retval = false;
    QString error;

    //CHECK PIN IS LOCKED (ENABLED)
    if (mSimManager->lockedPins().contains(pinType))
    {
        //unlock (disable) it to verify the pin code
        retval = this->disablePin(pinType, pin);
        //setting back to latter state
        if (retval)
            this->enablePin(pinType, pin);
        //othwerwise, get the dbus errormessage
        else
            error = mSimManager->errorName();
    }
    //CHECK PIN IS UNLOCKED (DISABLED)
    else
    {
        //lock (enable) it to verify the pin code
        retval = this->enablePin(pinType, pin);
        //if success, set back the latter state
        if (retval)
            this->disablePin(pinType, pin);
        //othwerwise, get the dbus errormessage
        else
            error = mSimManager->errorName();
    }

    //VERIFY INVALIDITY
    if (validity == "invalid")
    {
        qDebug ("invalid");
        MWTS_LEAVE;
        return invalidityCheck(error);
    }
    else
    {
        MWTS_LEAVE;
        return retval;
    }

    MWTS_LEAVE;
}

//TODO resetPin, it does not work. It might be a bug report to ofono
bool SimManagerTest::verifyPuk(const QString validity, QString puk)
{
    MWTS_ENTER;
    bool retval = false;

    //seems the only way to verify the puk code somehow
    retval = this->resetPin("pin", puk, "1234");

   QString error = mSimManager->errorName();

   //VERIFY INVALIDITY
   if (validity == "invalid")
   {
       MWTS_LEAVE;
       return invalidityCheck(error);
   }
   else
   {
       MWTS_LEAVE;
       return retval;
   }

    MWTS_LEAVE;
}

//TODO, it does not work. It might be a bug report to ofono
bool SimManagerTest::resetPin(const QString pinType, const QString puk, const QString newPin)
{
    MWTS_ENTER;

    QSignalSpy resetPin(mSimManager, SIGNAL(resetPinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestResetPin(pinType, puk, newPin);
    #else
        mSimManager->resetPin(pinType, puk, newPin);
    #endif

    qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType << " puk code:" << puk << "new pin " << newPin;

    mTimer->start(1000);
    mEventLoop->exec();

    qDebug () << __PRETTY_FUNCTION__ << resetPin.count();
    if (resetPin.takeFirst().at(0).toBool())
    {
        qDebug () << __PRETTY_FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __PRETTY_FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }
}

void SimManagerTest::simInfo (void)
{
    MWTS_ENTER;
    qDebug () <<  "XXXXXXXXXXXXXXXXXXXXXX SIM INFO STARTS XXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug () <<  "Pending pin required: " << mSimManager->pinRequired();
    qDebug () <<  "Locked (enabled) pins: " << mSimManager->lockedPins().count();
    foreach (QString str, mSimManager->lockedPins())
    {
        qDebug () << str;
    }
    qDebug () <<  "XXXXXXXXXXXXXXXXXXXXXX SIM INFO ENDS XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

    MWTS_LEAVE;
}

void SimManagerTest::onEnterPinRequired(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;
    if (pinType == mSimManager->pinRequired())
    {
        qDebug () << __PRETTY_FUNCTION__ << "Entering of pending " << pinType
                 << " code is required. Running enterPin ...";
        qDebug () << __PRETTY_FUNCTION__ << "If it fails you might wanto to run EnterPin explicitly from the script";

        this->enterPin(pinType, pin);
    }

    else if (mSimManager->pinRequired() == "puk")
    {
        qCritical () << __PRETTY_FUNCTION__ << "Ooops, puk in required to enter. You might want to call " <<
                                                "EnterPin puk <puk code>  explicitly.";
        MWTS_LEAVE;
        return;
    }

    MWTS_LEAVE;
}

bool SimManagerTest::invalidityCheck (const QString error)
{
    //VERIFY INVALIDITY
    MWTS_ENTER;

    //the dbus throw invalid format error
    if (error == "org.ofono.Error.InvalidFormat")
    {
        qCritical () << __PRETTY_FUNCTION__ << "The dbus raised the following error message " <<
                     "(might be ofono bug, or simply invalid arguments): " << error;
        MWTS_LEAVE;
        return FALSE;
    }
    //dbus throws error failed hopefully means the invalidity check is okay
    else if (error == "org.ofono.Error.Failed")
    {
        qDebug () << __PRETTY_FUNCTION__ << "The dbus raised the following message" <<
                " (which is good at invalidity check)" << error;
        MWTS_LEAVE;
        return TRUE;
    }
    //TODO, might want check other sort of dbus messages
    else
    {
        MWTS_LEAVE;
        return TRUE;
    }
}
