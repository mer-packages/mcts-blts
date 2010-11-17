/* ObexClientAgent.h
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

#ifndef OBEXCLIENTAGENT_H_
#define OBEXCLIENTAGENT_H_

#include <stable.h>
#include <QtDBus>

class ObexClientAgentRelay;

/*
 *
 */
class ObexClientAgent: public QObject
{
    Q_OBJECT
public:
	ObexClientAgent(const QString& file, QObject* parent=0);
	virtual ~ObexClientAgent();

	const QString& Path() const;

public slots:
	void Release();
	QString Request(const QDBusObjectPath &transfer);
	void Progress(const QDBusObjectPath &transfer,qulonglong transferred);
	void Complete(const QDBusObjectPath &transfer);
	void Error(const QDBusObjectPath& transfer, const QString& message);

signals:
	void request(const QDBusObjectPath& transfer);
	void progress(const QDBusObjectPath& transfer, qulonglong transferred);
	void complete(const QDBusObjectPath& transfer, bool success);
	void release();
	void error(const QString& message);

private:
    Q_DISABLE_COPY(ObexClientAgent);
    friend class ObexClientAgentRelay;

    ObexClientAgentRelay* 	m_pRelay;		///< pointer to the relay class
    QString					m_sFile;		///< list of file paths to be transferred
    QString					m_sPath;		///< dbus path
};

#endif /* OBEXCLIENTAGENT_H_ */
