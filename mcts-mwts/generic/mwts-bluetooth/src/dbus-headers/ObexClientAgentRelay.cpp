/* ObexClientAgentRelay.cpp
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
 */
#include "ObexClientAgentRelay.h"

ObexClientAgentRelay::ObexClientAgentRelay(QObject *parent) :
	QDBusAbstractAdaptor(parent)
{
	qDebug() << "ObexClientAgentRelay::ObexClientAgentRelay(QObject *parent)";
    setAutoRelaySignals(true);
}

ObexClientAgentRelay::~ObexClientAgentRelay()
{
}

void ObexClientAgentRelay::Release()
{
	qDebug() << "ObexClientAgentRelay::Release()";
    QMetaObject::invokeMethod(parent(), "Release");
}

QString ObexClientAgentRelay::Request(const QDBusObjectPath &transfer)
{
	qDebug() << "ObexClientAgentRelay::Request(const QDBusObjectPath &transfer)";
	QString out0;
    QMetaObject::invokeMethod(parent(), "Request", Q_RETURN_ARG(QString, out0), Q_ARG(QDBusObjectPath, transfer));
	return out0;
}

void ObexClientAgentRelay::Progress(const QDBusObjectPath &transfer,qulonglong transferred)
{
	qDebug() << "ObexClientAgentRelay::Progress(const QDBusObjectPath &transfer,qulonglong transferred)";
    QMetaObject::invokeMethod(parent(), "Progress", Q_ARG(QDBusObjectPath, transfer), Q_ARG(qulonglong, transferred));
}

void ObexClientAgentRelay::Complete(const QDBusObjectPath &transfer)
{
	qDebug() << "ObexClientAgentRelay::Complete(const QDBusObjectPath &transfer)";
    QMetaObject::invokeMethod(parent(), "Complete", Q_ARG(QDBusObjectPath, transfer));
}

void ObexClientAgentRelay::Error(const QDBusObjectPath& transfer,const QString& message)
{
	qDebug() << "ObexClientAgentRelay::Error(const QDBusObjectPath& transfer,const QString& message)";
    QMetaObject::invokeMethod(parent(), "Error", Q_ARG(QDBusObjectPath, transfer), Q_ARG(QString, message));
}
