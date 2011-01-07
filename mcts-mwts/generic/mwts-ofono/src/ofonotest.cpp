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

//original state: PIN, PUK are disabled (unlocked)
OfonoTest::OfonoTest(QObject* pParent)
    : MwtsTest(pParent)
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

    //qDebug() << "Config file name: " << g_pConfig->fileName();
    QString modem = g_pConfig->value("DEFAULT/modem").toString();
    qDebug () << __PRETTY_FUNCTION__ << "modem: " << modem;
    mSimManager = new OfonoSimManager(OfonoModem::ManualSelect, modem, this);

    //needs to have to have proper waiting time until getting the dbus answer
    mEventLoop = new QEventLoop;
    mTimer = new QTimer;
    connect( mTimer, SIGNAL(timeout()),
             this, SLOT(testTimeout()));
    //for resolving pending pin code issue
    connect( this, SIGNAL(enterPinRequired(const QString&, const QString&)),
             this, SLOT(onEnterPinRequired(const QString&, const QString&)));

    if (!mSimManager->modem()->powered())
    {
        qDebug () << __PRETTY_FUNCTION__ << "modem is not powered on";
        mSimManager->modem()->setPowered(true);

        mTimer->start(5000);
        mEventLoop->exec();
    }
    //not needed for now
    if (!mSimManager->modem()->online())
    {
        qDebug () << __PRETTY_FUNCTION__ << "modem is not online";
        mSimManager->modem()->setOnline(true);

        mTimer->start(5000);
        mEventLoop->exec();
    }

    if (mSimManager->isValid())
    {
        qDebug () << __PRETTY_FUNCTION__ << "modem is valid";
    }
    else
    {
        qCritical ("modem is not valid");
        g_pResult->StepPassed(__PRETTY_FUNCTION__, FALSE);
        MWTS_LEAVE;
        return;
    }

        qDebug () << __PRETTY_FUNCTION__ << "modem is powered: " << mSimManager->modem()->powered();
        qDebug () << __PRETTY_FUNCTION__ << "modem is online: "  << mSimManager->modem()->online();

        MWTS_LEAVE;

}

void OfonoTest::OnUninitialize()
{
    MWTS_ENTER;    

    // to set back the original state: PIN, PUK are disabled (unlocked)
    if (mSimManager->lockedPins().contains("pin"))
    {
        this->disablePin("pin", mPin);
    }

    if (mSimManager->lockedPins().contains("puk"))
    {
        this->disablePin("puk", mPuk);
    }

    if (mTimer->isActive())
    {
        qDebug() << "Timer active, stopping it...";
        mTimer->stop();
    }

    if (mEventLoop->isRunning())
    {
        qDebug() << "Event loop running, stopping it...";
        mEventLoop->exit();
    }

    qDebug() << "Killing timer...";

    if (mTimer)
    {
        if (mTimer->isActive())
        {
            qDebug() << "Timer active, stopping it...";
            mTimer->stop();
        }
        qDebug() << "Deleting timer...";
        delete mTimer;
        mTimer = NULL;
    }

    qDebug() << "Killing event loop...";
    if (mEventLoop)
    {
        if (mEventLoop->isRunning())
        {
            qDebug() << "Event loop running, stoping it...";
            mEventLoop->exit();
        }
        qDebug() << "Deleting event loop...";
        delete mEventLoop;
        mEventLoop = NULL;
    }

    if (mSimManager)
    {
        delete mSimManager;
        mSimManager = NULL;
    }

    MWTS_LEAVE;
}

bool OfonoTest::enterPin(const QString &pinType, const QString &pin)
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

bool OfonoTest::changePin(const QString &pinType, const QString &oldPin, const QString &newPin)
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

bool OfonoTest::enablePin(const QString &pinType, const QString &pin)
{
    MWTS_ENTER;

    //check whether EnterPin is required or not
    emit enterPinRequired(pinType, pin);

    //save the values into class members in order disable the pin, puk later at OnUninitialize()
    if (pinType=="pin")
    {
        mPin = pin;
    }
    if (pinType=="puk")
    {
        mPuk = pin;
    }

    //check whether the pin is already enabled
    if (mSimManager->lockedPins().contains(pinType))
    {
        qCritical () << __PRETTY_FUNCTION__ << pinType << " is already enabled.";
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

bool OfonoTest::disablePin(const QString &pinType, const QString &pin)
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

bool OfonoTest::resetPin (const QString &pinType, const QString &puk, const QString &newPin)
{
    MWTS_ENTER;

    QSignalSpy resetPin(mSimManager, SIGNAL(resetPinComplete(bool)));

    #ifdef MWTS_SIMMANAGER_OLD
        mSimManager->requestResetPin(pinType, puk, newPin);
    #else
        mSimManager->resetPin(pinType, puk, newPin);
    #endif

        qDebug () << __PRETTY_FUNCTION__ << "pin type:" << pinType
                                         << " puk code:" << puk
                                         << " new pin code:" << newPin;

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

void OfonoTest::simInfo (void)
{
    MWTS_ENTER;
    qDebug () << __PRETTY_FUNCTION__ << "Pin required: " << mSimManager->pinRequired();
    qDebug () << __PRETTY_FUNCTION__ << "Locked pins: " << mSimManager->lockedPins().count();
    MWTS_LEAVE;
}

void OfonoTest::testTimeout()
{
    MWTS_ENTER;
    if ( mEventLoop->isRunning() )
    {
        qDebug() << "Event loop running! Stopping...";
        mTimer->stop();
        mEventLoop->exit();
    }
    MWTS_LEAVE;
}

void OfonoTest::onEnterPinRequired(const QString &pinType, const QString &pin)
{
    if (pinType == mSimManager->pinRequired())
    {
        qDebug () << __PRETTY_FUNCTION__ << "Entering of pending " << pinType
                 << " code is required. Running enterPin ...";
        qDebug () << __PRETTY_FUNCTION__ << "If it fails you might wanto to run EnterPin explicitly from the script";

        this->enterPin(pinType, pin);
    }

    MWTS_LEAVE;
}
