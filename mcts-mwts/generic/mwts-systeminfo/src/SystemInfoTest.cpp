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
	MWTS_LEAVE;
}

/**
 * Test function
 */
void SystemInfoTest::TestBattery()
{
	MWTS_ENTER;

    QSystemDeviceInfo systeminfo(this);

    QSystemDeviceInfo::BatteryStatus 	batterystatus = systeminfo.batteryStatus();
    qDebug() << "BatteryStatus " << batterystatus;

    QSystemDeviceInfo::PowerState 	powerstate = systeminfo.currentPowerState ();
    qDebug() << "PowerState " << powerstate;


#ifdef oldcode
    QString sExeName;
    QStringList sArgs;

     // Connect to important signals
    connect( m_pProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)) );
    connect( m_pProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onFinished(int,QProcess::ExitStatus)) );
    connect( m_pProcess, SIGNAL(started()), SLOT(onStarted()) );
    connect( m_pProcess, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)) );

    if (remote)
    {
        sExeName = "/usr/bin/ssh";
        sArgs << "root@192.168.2.15";

    }
    else
    {
        sExeName = "/usr/bin/dbus-send";
    }
    sArgs << "--system" << "--print-reply" << "--dest=com.nokia.thermalmanager" << "/com/nokia/thermalmanager" << "com.nokia.thermalmanager.estimate_surface_temperature";

    qDebug() << "execute " << sExeName <<  sArgs;
    m_pProcess->start( sExeName , sArgs);

    qDebug() << "Wating for " << sExeName << " to finish...";
    MWTS_DEBUG("Starting the main loop");
	Start();

    // when proces is finished
	// then we continue here.
	MWTS_DEBUG("Main loop stopped");

    // if normal exit of process
    if (m_pReturnValue)
    {
        QTextStream line;
        QString result;
        QString unit;
        double x;

        // get result, it is 2 lines

        g_pResult->Write("Succesfully called dbus-send!");

        // 1 line: how we was called
        result = m_pProcess->readLine();
        qDebug() << "output line 1 " << result;

        // 2.line: dbus varable type, variable
        result = m_pProcess->readLine();
        qDebug() << "output line 2 " << result;

        //QString strong = in.readLine();
        line.setString(&result);
        line >> unit >> x;

        // write results. the first one is just informal if you want to.
        g_pResult->StepPassed("dsme", true);
        g_pResult->AddMeasure("dsme", x, unit);
    }
    else
    {
        g_pResult->StepPassed("dsme", false);
    }
#endif
}





void SystemInfoTest::TestWallPower()
{
    MWTS_ENTER;
    m_QmExpectedWallPower = true;
    SetCharging();
    MWTS_LEAVE;
}

void SystemInfoTest::TestBatteryPower()
{
    MWTS_ENTER;
    m_QmExpectedWallPower = false;
    SetCharging();
    MWTS_LEAVE;
}

void SystemInfoTest::TestChargingTypeUSB100mA()
{
    MWTS_ENTER;
//    m_QmExpectedChargingType = Maemo::QmBattery::USB_100mA;
    SetCharging();
    MWTS_LEAVE;
}

void SystemInfoTest::SetCharging()
{
    MWTS_ENTER;

    QSystemDeviceInfo systeminfo(this);
    
    connect(&systeminfo,
            SIGNAL(batteryLevelChanged (int)),
            this,
            SLOT(batteryLevelChanged (int)));

    connect(&systeminfo,
            SIGNAL(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),
            this,
            SLOT(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)));

    connect(&systeminfo,
            SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this,
            SLOT(powerStateChanged(QSystemDeviceInfo::PowerState)));

   qDebug("SystemInfoTest::OnInitialize End, all initialised to get signals and listerners started");

    // now start qt main loop
    MWTS_DEBUG("Starting the qt-main loop");
    Start();

    // when timer emits signal 'timeout' the Stop is called
    // then we continue from here.
    MWTS_DEBUG("Main loop stopped");

    disconnect(&systeminfo,
            SIGNAL(batteryLevelChanged (int)),
            this,
            SLOT(batteryLevelChanged (int)));

    disconnect(&systeminfo,
            SIGNAL(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),
            this,
            SLOT(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)));

    disconnect(&systeminfo,
            SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this,
            SLOT(powerStateChanged(QSystemDeviceInfo::PowerState)));

    MWTS_LEAVE;
}


/* Battery callbacks */
void SystemInfoTest::ChargingStateChanged(/*const Maemo::QmBattery::ChargingState state*/)
{
    MWTS_ENTER;
    MWTS_DEBUG("ChargingStateChanged");
/*

    QString resultText = "";
    Maemo::QmBattery::ChargerType chargerType;
    chargerType = m_QmBattery->getChargerType();

    switch (chargerType)
    {
    case Maemo::QmBattery::Unknown:
        MWTS_DEBUG("ChargingType: Unknown charger");
        g_pResult->Write("\nChargingType changed to: Unknown charger");
        break;
    case Maemo::QmBattery::None:
        MWTS_DEBUG("ChargingType: No charger connected");
        g_pResult->Write("\nChargingType changed to: No charger connected");
        break;
    case Maemo::QmBattery::Wall:
        MWTS_DEBUG("ChargingType: Wall charger");
        g_pResult->Write("\nChargingType changed to: Wall charger");
        break;
    case Maemo::QmBattery::USB_500mA:
        MWTS_DEBUG("ChargingType: USB with 500mA output");
        g_pResult->Write("\nChargingType changed to: USB with 500mA output");
        break;
    case Maemo::QmBattery::USB_100mA:
        MWTS_DEBUG("ChargingType: USB with 100mA output");
        g_pResult->Write("\nChargingType changed to: USB with 100mA output");
        break;
    default:
        MWTS_DEBUG("Charging type not recognized");
        g_pResult->Write("\nCharging type not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
        Stop();
        return;
    }

    switch (state)
    {
    case Maemo::QmBattery::StateNotCharging:
        MWTS_DEBUG("Charging state: Not charging");
        g_pResult->Write("ChargingState changed to: Not charging");

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

    case Maemo::QmBattery::StateCharging:
        MWTS_DEBUG("Charging state: Charging");
        g_pResult->Write("ChargingState changed to: Charging");

        if(m_QmChargerPlugged != true)
        {
            m_QmChargerPlugged = true;
            if(chargerType != m_QmExpectedChargingType)
            {
                g_pResult->Write("Wrong ChargingType");
                g_pResult->StepPassed(this->CaseName(), false);
            }
            else
            {
                // first step ok
                MWTS_DEBUG("Charger plugged. Waiting for charger to be unplugged.");
                return;
            }
        }
        else
        {
            g_pResult->Write("Wrong ChargingState: Charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

    case Maemo::QmBattery::StateChargingFailed:
        MWTS_DEBUG("Charging state: Charging failed");
        g_pResult->Write("ChargingState changed to: Charging error, e.g. unsupported charger");
        g_pResult->StepPassed(this->CaseName(), false);
        break;

    default:
        MWTS_DEBUG("Charging state not recognized");
        g_pResult->Write("Charging state not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
        break;
    }

*/
    Stop();
    MWTS_LEAVE;
}

/* This signal is emitted when battery level has changed. level is the new level.*/
void SystemInfoTest::batteryLevelChanged ( int level )
{
    MWTS_ENTER;
    MWTS_DEBUG("batteryLevelChanged ");

    g_pResult->AddMeasure("batteryLevel", level, "int");

    g_pResult->StepPassed(this->CaseName(), true);

    Stop();
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

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

    case QSystemDeviceInfo::BatteryCritical:
        MWTS_DEBUG("Battery Status: Battery level is critical 3\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is critical 3\% or less");

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;


    case QSystemDeviceInfo::BatteryVeryLow:
        MWTS_DEBUG("Battery Status: Battery level is very low, 10\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is very low, 10\% or less");

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

    case QSystemDeviceInfo::BatteryLow:
        MWTS_DEBUG("Battery Status: Battery level is low 40\% or less");
        g_pResult->Write("ChargingState changed to: Battery level is low 40\% or less");

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

     case QSystemDeviceInfo::BatteryNormal:
        MWTS_DEBUG("Battery Status: Battery level is above 40\%");
        g_pResult->Write("ChargingState changed to: Battery level is above 40\%");

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

     default:
	qCritical("ChargingState changed to: unsupported value");
        g_pResult->Write("Charging state not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
    }   
 
 
    Stop();
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
        g_pResult->StepPassed(this->CaseName(), false);
        break;

    case QSystemDeviceInfo::BatteryPower:
        MWTS_DEBUG("Power Status: On battery power");
        g_pResult->Write("PowerState changed to: On battery power");
        m_QmChargerPlugged = false;

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;


    case QSystemDeviceInfo::WallPower:
        MWTS_DEBUG("Power Status: On wall power");
        g_pResult->Write("PowerState changed to: On wall power");
        m_QmChargerPlugged = true;

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

    case QSystemDeviceInfo::WallPowerChargingBattery:
        MWTS_DEBUG("Power Status: On wall power and charging main battery");
        g_pResult->Write("PowerState changed to: On wall power and charging main battery");
        m_QmChargerPlugged = true;

        if(m_QmChargerPlugged == true)
        {
            // passed
            MWTS_DEBUG("Charger unplugged.");
            g_pResult->StepPassed(this->CaseName(), true);
        }
        else
        {
            g_pResult->Write("\nWrong ChargingState: Not charging");
            g_pResult->StepPassed(this->CaseName(), false);
        }
        break;

     default:
	qCritical("PowerState changed to: unsupported value");
        g_pResult->Write("Power state not recognized");
        g_pResult->StepPassed(this->CaseName(), false);
    }   
 
    Stop();
    MWTS_LEAVE;
}







