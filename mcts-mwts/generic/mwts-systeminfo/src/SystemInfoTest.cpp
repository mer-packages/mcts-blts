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

        m_pProcess = new QProcess();

        MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
SystemInfoTest::~SystemInfoTest()
{
	MWTS_ENTER;

        if ( m_pProcess )
        {
                m_pProcess->kill();
                m_pProcess->close();
                m_pProcess->disconnect();
                delete m_pProcess;
                m_pProcess = NULL;
        }

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




/** Gets called if there is error in process*/
void SystemInfoTest::onError( QProcess::ProcessError error )
{
        MWTS_ENTER;
        m_pReturnValue = false;
        switch ( error )
        {
                case QProcess::FailedToStart:
                        qDebug() << "The process failed to start.";
                        qCritical() << "The process failed to start.";
                        break;
                case QProcess::Crashed:
                        qDebug() << "The process crashed.";
                        qCritical() << "The process crashed.";
                        break;
                case QProcess::Timedout:
                        qDebug() << "The last waitFor...() function timed out.";
                        qCritical() << "The last waitFor...() function timed out.";
                        break;
                case QProcess::WriteError:
                        qDebug() << "An error occurred when attempting to write to the process.";
                        qCritical() << "An error occurred when attempting to write to the process.";
                        break;
                case QProcess::ReadError:
                        qDebug() << "The last waitFor...() function timed out.";
                        qCritical() << "The last waitFor...() function timed out.";
                        break;
                case QProcess::UnknownError:
                        qDebug() << "An unknown error occurred.";
                        qCritical() << "An unknown error occurred.";
                        break;
        }

        m_pReturnValue = false;
        Stop();

        MWTS_LEAVE;
}

/** Called when the process finishes. Logs whether successfull or not*/
void SystemInfoTest::onFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
        MWTS_ENTER;

        qDebug() << "Exit code: " << exitCode;

        switch ( exitStatus )
        {
                case QProcess::NormalExit:
                        qDebug() << "The process exited normally.";
                        m_pReturnValue = true;
                        break;
                case QProcess::CrashExit:
                        qDebug() << "The process crashed.";
                        m_pReturnValue = false;
                        break;
        }

        Stop();
        MWTS_LEAVE;
}

/** Gets called when iperf process started*/
void SystemInfoTest::onStarted()
{
        MWTS_ENTER;
        qDebug() << "The process has started.";
        MWTS_LEAVE;
}

/** Gets called when process state changes*/
void SystemInfoTest::onStateChanged( QProcess::ProcessState newState )
{
        MWTS_ENTER;
        switch ( newState )
        {
                case QProcess::NotRunning:
                        qDebug() << "The process is not running.";
                        break;
                case QProcess::Starting:
                        qDebug() << "The process is starting, but the program has not yet been invoked.";
                        break;
                case QProcess::Running:
                        qDebug() << "The process is running and is ready for reading and writing.";
                        break;
        }
        MWTS_LEAVE;
}






