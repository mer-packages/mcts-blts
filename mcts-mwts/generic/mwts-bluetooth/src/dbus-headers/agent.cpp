/* agent.cpp
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

#include <QCoreApplication>
#include <QDebug>
#include <QtDBus>
#include <stable.h>
#include "dbus-headers/agentrelay.h"
#include "agent.h"
#include "BluetoothCommonTypes.h"


Agent::Agent(QObject* parent,uint passkey):
    QObject(parent), m_passkey(passkey), m_sPath(DBUS_AGENT_PATH)
{
    MWTS_ENTER;
    qDebug() << "Agent got passkey:"<< m_passkey;
    qDebug() << "Agent got path:"<<m_sPath;
    m_pRelay = new AgentRelay(this);
    bool bret = QDBusConnection::systemBus().registerObject(DBUS_AGENT_PATH, this);
    qDebug() << "Agent object registered == " << bret;
    MWTS_LEAVE;
}

Agent::~Agent()
{
    MWTS_ENTER;
    delete m_pRelay;
    m_pRelay = 0;
    qDebug() << "Agent Relay pointer deleted";
    MWTS_LEAVE;
}

void Agent::DisplayPasskey(const QDBusObjectPath& device, uint passkey,uint entered)
{
    MWTS_ENTER;
    qDebug() << "Passkey:" << passkey;
    MWTS_LEAVE;
}

void Agent::RequestConfirmation(const QDBusObjectPath& device, uint passkey)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

uint Agent::RequestPasskey(const QDBusObjectPath& device)
{
    MWTS_ENTER;
    MWTS_LEAVE;
    return m_passkey;
}

QString Agent::RequestPinCode(const QDBusObjectPath& device)
{
    MWTS_ENTER;
    QString pin;
    pin.sprintf("%.4d", m_passkey);
    qDebug() << "Giving out pin code: "<<pin;
    MWTS_LEAVE;
    return pin;
}

void Agent::Release()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void Agent::Cancel()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void Agent::ConfirmModeChanged(const QString& mode)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void Agent::Authorize(const QDBusObjectPath& device,const QString& uuid)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

const QString& Agent::path() const
{
    MWTS_ENTER;
    MWTS_LEAVE;
    return m_sPath;
}
