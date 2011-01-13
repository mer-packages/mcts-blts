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

#ifndef VOICECALLTEST_H
#define VOICECALLTEST_H

#include "ofonotestinterface.h"
#include <QtTest/QtTest>

#include <ofono-qt/ofonocallforwarding.h>
#include <ofono-qt/ofonocallbarring.h>
#include <ofono-qt/ofonovoicecallmanager.h>

class VoiceCallManagerTest : public OfonoTestInterface
{
    Q_OBJECT

public:
    explicit VoiceCallManagerTest(QObject *parent = 0);
};


class VoiceCallForwardingTest : public OfonoTestInterface
{
    Q_OBJECT

public:
    explicit VoiceCallForwardingTest(QObject *parent = 0);
};

class VoiceCallBarringTest : public OfonoTestInterface
{
    Q_OBJECT

public:
    explicit VoiceCallBarringTest(QObject *parent = 0);
};

#endif // VOICECALLTEST_H
