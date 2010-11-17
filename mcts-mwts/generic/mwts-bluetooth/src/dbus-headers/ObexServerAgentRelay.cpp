/* ObexServerAgentRelay.cpp
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
#include "ObexServerAgentRelay.h"

ObexServerAgentRelay::ObexServerAgentRelay(QObject *parent) :
	QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

ObexServerAgentRelay::~ObexServerAgentRelay()
{
}


QString ObexServerAgentRelay::Authorize(const QDBusObjectPath &transfer,
										const QString& bt_address,
										const QString& name,
										const QString& type,
										int length,
										int time)
{
	qDebug() << "Authorize";
	QString out0;
    QMetaObject::invokeMethod(parent(), "Authorize", 	Q_RETURN_ARG(QString, out0),
    													Q_ARG(QDBusObjectPath, transfer),
    													Q_ARG(QString, bt_address),
    													Q_ARG(QString, name),
    													Q_ARG(QString, type),
    													Q_ARG(int, length),
    													Q_ARG(int, time));
	return out0;
}

void ObexServerAgentRelay::Cancel()
{
	qDebug() << "Cancel";
    QMetaObject::invokeMethod(parent(), "Cancel");
}
