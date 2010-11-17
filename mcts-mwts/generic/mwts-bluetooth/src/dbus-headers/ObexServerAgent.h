/* ObexServerAgent.h
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

#ifndef OBEXSERVERAGENT_H_
#define OBEXSERVERAGENT_H_

#include <stable.h>
#include <QtDBus>

class ObexServerAgentRelay;

/*
 *
 */
class ObexServerAgent: public QObject
{
    Q_OBJECT
public:
	ObexServerAgent(const QString& file, QObject* parent=0);
	virtual ~ObexServerAgent();

	const QString& Path() const;

public slots:
	QString Authorize(	const QDBusObjectPath& transfer,
						const QString& bt_address,
						const QString& name,
						const QString& type,
						int length,
						int time);
	void Cancel();

signals:
	void authorize(	const QDBusObjectPath& transfer,
					const QString& bt_address,
					const QString& name,
					const QString& type,
					int length,
					int time);
	void cancelled();

private:
	Q_DISABLE_COPY(ObexServerAgent);
	friend class ObexServerAgentRelay;

	ObexServerAgentRelay* 	m_pRelay;		///< pointer to the relay class
    QString					m_sFile;		///< list of file paths to be transferred
    QString					m_sPath;		///< dbus path
};

#endif /* OBEXSERVERAGENT_H_ */
