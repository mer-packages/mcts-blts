/* agent.h Processes all Bluez dbus commands relayed to it.
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

#ifndef _AGENT_H_
#define _AGENT_H_

#include <stable.h>
#include <QtDBus>

class AgentRelay;

class Agent: public QObject
{
    Q_OBJECT

public:
    Agent(QObject* parent=0, uint passkey=0);
    virtual ~Agent();

    const QString& path()const;

public slots:
    void DisplayPasskey(const QDBusObjectPath& device, uint passkey, uint entered);
    void RequestConfirmation(const QDBusObjectPath& device,uint passkey);
    uint RequestPasskey(const QDBusObjectPath& device);
    QString RequestPinCode(const QDBusObjectPath& device);
    void Release();
    void Cancel();
    void ConfirmModeChanged(const QString& mode);
    void Authorize(const QDBusObjectPath& device,const QString& uuid);

private:
    Q_DISABLE_COPY(Agent);
    friend class AgentRelay;

    AgentRelay* m_pRelay;
    uint m_passkey;
    QString m_sPath;
};

#endif // _AGENT_H_
