/*
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

#ifndef OFONOTESTINTERFACE_H
#define OFONOTESTINTERFACE_H

#include <MwtsCommon>
#include <QtTest/QtTest>

#define MODEM_TIMEOUT 5000
class OfonoTestInterface : public QObject
{
    Q_OBJECT

public:
    explicit OfonoTestInterface(QObject *parent = 0);

private slots:
    void testTimeout();

protected:
    /**
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    QEventLoop      *mEventLoop;
    QTimer          *mTimer;
};

#endif // OFONOTESTINTERFACE_H
