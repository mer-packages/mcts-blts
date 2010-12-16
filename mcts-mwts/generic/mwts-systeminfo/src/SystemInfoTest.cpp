/* SystemInfoTest.cpp -- source code for SystemInfoTest class
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

#include <QSystemDeviceInfo>
//#include <qsystemdeviceinfo.h>
//#include <qsystemdeviceinfo.h>



#include "stable.h"
#include "SystemInfoTest.h"

QTM_USE_NAMESPACE

/**
 * Constructor for template test class
 */
SystemInfoTest::SystemInfoTest()
{
	MWTS_ENTER;

        MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
SystemInfoTest::~SystemInfoTest()
{
	MWTS_ENTER;

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void SystemInfoTest::OnInitialize()
{
    MWTS_ENTER;

	// create objects for the test
	// connect wanted signals
	// do any kind of initialization here
    m_pSysteminfo = new QSystemDeviceInfo(this);

    qDebug() << "SystemInfoTest::OnInitialize: connect";

    connect(m_pSysteminfo,
            SIGNAL(batteryLevelChanged (int)),
            this,
            SLOT(batteryLevelChanged (int)));

    connect(m_pSysteminfo,
            SIGNAL(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),
            this,
            SLOT(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)));

    connect(m_pSysteminfo,
            SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this,
            SLOT(powerStateChanged(QSystemDeviceInfo::PowerState)));

    qDebug("SystemInfoTest::OnInitialize all initialised to get signals and listerners started");
 

    m_QmExpectedWallPower = false;
    m_QmExpectedBatteryPower = false;
    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void SystemInfoTest::OnUninitialize()
{
    MWTS_ENTER;
	// disconnect signals
	// clean objects etc

    disconnect(m_pSysteminfo,
            SIGNAL(batteryLevelChanged (int)),
            this,
            SLOT(batteryLevelChanged (int)));

    disconnect(m_pSysteminfo,
            SIGNAL(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),
            this,
            SLOT(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)));

    disconnect(m_pSysteminfo,
            SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this,
            SLOT(powerStateChanged(QSystemDeviceInfo::PowerState)));
    delete m_pSysteminfo;
    m_pSysteminfo = NULL;

    MWTS_LEAVE;
}




void SystemInfoTest::TestWallPower()
{
    MWTS_ENTER;
    qDebug() << "m_QmExpectedWallPower = true";
    m_QmExpectedWallPower = true;
    qDebug() << "SetCharging()";
    SetCharging();
    MWTS_LEAVE;
}

void SystemInfoTest::TestBatteryPower()
{
    MWTS_ENTER;
    qDebug() << "m_QmExpectedWallPower = false";
    m_QmExpectedBatteryPower  = false;
    SetCharging();
    MWTS_LEAVE;
}

void SystemInfoTest::SetCharging()
{
    MWTS_ENTER;

    QSystemDeviceInfo::BatteryStatus batterystatus = m_pSysteminfo->batteryStatus();
    qDebug() << "SetCharging: BatteryStatus " << batterystatus;

    QSystemDeviceInfo::PowerState powerstate = m_pSysteminfo->currentPowerState ();
    qDebug() << "SetCharging: PowerState " << powerstate;
    
    m_FirstStep = false;

    // now start qt main loop
    MWTS_DEBUG("Starting the qt-main loop");
    Start();

    // when timer emits signal 'timeout' the Stop is called
    // then we continue from here.
    MWTS_DEBUG("Main loop stopped");

    MWTS_LEAVE;
}


/* Battery callbacks */

/* This signal is emitted when battery level has changed. level is the new level.*/
void SystemInfoTest::batteryLevelChanged ( int level )
{
    MWTS_ENTER;
    MWTS_DEBUG("batteryLevelChanged ");

    g_pResult->AddMeasure("batteryLevel", level, "int");

    MWTS_LEAVE;
}


/* This signal is emitted when battery status has changed. status is the new status. */
void SystemInfoTest::batteryStatusChanged ( QSystemDeviceInfo::BatteryStatus status )
{
    MWTS_ENTER;
    MWTS_DEBUG("batteryStatusChanged");

    switch (status)
    {
    case QSystemDeviceInfo::NoBatteryLevel:
        MWTS_DEBUG("Battery Status: Battery level undetermined");
        g_pResult->Write("ChargingState changed to: Battery level undetermined");
        break;

    case QSystemDeviceInfo::BatteryCritical:
        MWTS_DEBUG("Battery Status: Battery level is critical 3\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is critical 3\% or less");
        break;


    case QSystemDeviceInfo::BatteryVeryLow:
        MWTS_DEBUG("Battery Status: Battery level is very low, 10\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is very low, 10\% or less");
        break;

    case QSystemDeviceInfo::BatteryLow:
        MWTS_DEBUG("Battery Status: Battery level is low 40\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is low 40\% or less");
        break;

     case QSystemDeviceInfo::BatteryNormal:
        MWTS_DEBUG("Battery Status: Battery level is above 40\%");
        g_pResult->Write("ChargingState changed to: Battery level is above 40\%");
        break;

     default:
	qCritical("ChargingState changed to: unsupported value");
        g_pResult->Write("Charging state not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
    }   
 
    MWTS_LEAVE;
}

/* This signal is emitted when the power state has changed, such as when a phone gets plugged in to the wall. state is the new power state. */
void SystemInfoTest::powerStateChanged ( QSystemDeviceInfo::PowerState state )
{
    MWTS_ENTER;
    MWTS_DEBUG("powerStateChanged");

    switch (state)
    {
    case QSystemDeviceInfo::UnknownPower:
        MWTS_DEBUG("Power Status: Power error");
        g_pResult->Write("PowerState changed to: Power error");
        qCritical("PowerState changed to: unsupported value");
        g_pResult->Write("Power state not recognized");
        break;

    case QSystemDeviceInfo::BatteryPower:
        MWTS_DEBUG("Power Status: On battery power");
        g_pResult->Write("PowerState changed to: On battery power");
        m_QmChargerPlugged = false;
        break;
 

    case QSystemDeviceInfo::WallPower:
        MWTS_DEBUG("Power Status: On wall power");
        g_pResult->Write("PowerState changed to: On wall power");
        m_QmChargerPlugged = true;
        break;

    case QSystemDeviceInfo::WallPowerChargingBattery:
        MWTS_DEBUG("Power Status: On wall power and charging main battery");
        g_pResult->Write("PowerState changed to: On wall power and charging main battery");
        m_QmChargerPlugged = true;
        break;

     default:
	qCritical("PowerState changed to: unsupported value");
        g_pResult->Write("Power state not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
    }

    if (m_QmExpectedWallPower && m_QmChargerPlugged && m_FirstStep)
    {
         g_pResult->StepPassed(this->CaseName(), true);
         Stop();
    }
    else
    if (m_QmExpectedWallPower && !m_QmChargerPlugged&& !m_FirstStep)
    {
        m_FirstStep = true;
    }
    else
    if (m_QmExpectedBatteryPower && !m_QmChargerPlugged && m_FirstStep)
    {
         g_pResult->StepPassed(this->CaseName(), true);
         Stop();
    }
    else
    if (m_QmExpectedBatteryPower  && m_QmChargerPlugged && !m_FirstStep)
    {
         m_FirstStep =  true;
    }

    MWTS_LEAVE;
}







