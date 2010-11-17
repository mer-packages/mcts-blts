/* agentrelay.cpp
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

#include <QtDBus>

#include "agentrelay.h"


AgentRelay::AgentRelay(QObject *parent) :
    QDBusAbstractAdaptor(parent)
{
    qDebug() << "AgentRelay::AgentRelay(QObject *parent)";
    setAutoRelaySignals(true);
}

AgentRelay::~AgentRelay()
{
    qDebug() << "AgentRelay::~AgentRelay()";
}

void AgentRelay::Authorize(const QDBusObjectPath &device, const QString &uuid)
{
    qDebug() << "Authorize";
    QMetaObject::invokeMethod(parent(), "Authorize", Q_ARG(QDBusObjectPath, device), Q_ARG(QString, uuid));
}

void AgentRelay::Cancel()
{
    qDebug() << "Cancel";
    QMetaObject::invokeMethod(parent(), "Cancel");
}

void AgentRelay::ConfirmModeChange(const QString &mode)
{
    qDebug() << "ConfirmModeChange";
    QMetaObject::invokeMethod(parent(), "ConfirmModeChange", Q_ARG(QString, mode));
}

void AgentRelay::DisplayPasskey(const QDBusObjectPath &device, uint passkey, uint entered)
{
    qDebug() << "DisplayPasskey";
    QMetaObject::invokeMethod(parent(), "DisplayPasskey", Q_ARG(QDBusObjectPath, device), Q_ARG(uint, passkey), Q_ARG(uint, entered));
}

void AgentRelay::Release()
{
    qDebug() << "Release";
    QMetaObject::invokeMethod(parent(), "Release");
}

void AgentRelay::RequestConfirmation(const QDBusObjectPath &device, uint passkey)
{
    qDebug() << "RequestConfirmation";
    QMetaObject::invokeMethod(parent(), "RequestConfirmation", Q_ARG(QDBusObjectPath, device), Q_ARG(uint, passkey));
    return;
}

uint AgentRelay::RequestPasskey(const QDBusObjectPath &device)
{
    qDebug() << "RequestPasskey";
    uint out0;
    QMetaObject::invokeMethod(parent(), "RequestPasskey", Q_RETURN_ARG(uint, out0), Q_ARG(QDBusObjectPath, device));
    return out0;
}

QString AgentRelay::RequestPinCode(const QDBusObjectPath &device)
{
    qDebug() << "RequestPinCode";
    QString out0;
    QMetaObject::invokeMethod(parent(), "RequestPinCode", Q_RETURN_ARG(QString, out0), Q_ARG(QDBusObjectPath, device));
    return out0;
}
