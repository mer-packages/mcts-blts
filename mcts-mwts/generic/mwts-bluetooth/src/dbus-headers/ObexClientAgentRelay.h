/* ObexClientAgentRelay.h
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

#ifndef OBEXCLIENTAGENTRELAY_H_
#define OBEXCLIENTAGENTRELAY_H_

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

/*
 *
 */
class ObexClientAgentRelay: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openobex.Agent")
public:
	ObexClientAgentRelay(QObject* parent=0);
	virtual ~ObexClientAgentRelay();

public slots:
	void Release();
	QString Request(const QDBusObjectPath &transfer);
	void Progress(const QDBusObjectPath &transfer,qulonglong transferred);
	void Complete(const QDBusObjectPath &transfer);
	void Error(const QDBusObjectPath& transfer,const QString& message);
};

#endif /* OBEXCLIENTAGENTRELAY_H_ */
