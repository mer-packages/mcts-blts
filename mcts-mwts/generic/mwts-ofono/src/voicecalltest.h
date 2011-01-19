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
#include <ofono-qt/ofonocallsettings.h>

#define FORWARDING_TIMEOUT 5000

class VoiceCallTest : public OfonoTestInterface
{
    Q_OBJECT

public:
    explicit VoiceCallTest(QObject *parent = 0);

    /**
     * Destructor for VoiceCallTest test class
     */
    virtual ~VoiceCallTest();

    /**
     * OnInitialize is called before test execution
     */
    void OnInitialize();

    /**
     * OnUninitialize is called after test execution
     */
    void OnUninitialize();

    /************************************************************
     *
     * Voice call forwarding methods
     *
     */

    /**
     * Contains the value of the voice "Busy" call forwarding rule
     * @param property is the telephone number value
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallBusy(const QString &property);

    /**
     * Contains the value of the voice "No Reply" call forwarding rule.
     * @param property is the telephone number value
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallNoReply(const QString &property);

    /**
     * Contains the value of the voice "Not Reachable" callforwarding rule.
     * @param property is the telephone number value
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallNotReachable(const QString &property);

    /**
     * Contains the value of the voice unconditional call
     * forwarding property. If the value is an empty string,
     * then this call forwarding rule is not active.  Otherwise
     * the rule is active with the string value as the phone number.
     * @param property is the telephone number value
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallUnconditional(const QString &property);

    /**
     * Disables all call forwarding rules for type.
     *	Type can be one of:
     *			"all" or "" - Disables all rules
     *			"conditional" - Disables all conditional rules,
     *				e.g. busy, no reply and not reachable.
     * @param the type is "all" or "conditional"
     * @return TRUE on success, otherwise FALSE
     */
    bool disableAll(const QString &type);

    /************************************************************
     *
     * Voice call settings methods
     *
     */

    /**
     * Contains the call waiting status for Voice calls.
     * If enabled, the call waiting status will be
     * presented to the subscriber for voice calls.
     * Possible values are:
     *               "disabled",
     *               "enabled",
     * @param enabled/disabled
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallWaiting (const QString &setting);

    /************************************************************
     *
     * Voice call barring methods
     *
     */

    /**
     * Contains the value of the barrings for the incoming
     * voice calls. The possible values are:
     * - "always" bar all incoming voice calls
     * - "whenroaming" bar incoming voice calls when roaming,
     * - "disabled" if no barring is active
     * @param barrings= always|whenroaming|disabled, pin code
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallIncoming(const QString &barrings, const QString &pin);

    /**
     * Contains the value of the barrings for the outgoing
     * voice calls. The possible values are:
     * - "all" bar all outgoing calls
     * - "international" bar all outgoing international calls
     * - "internationalnothome" bar all outgoing
     * international calls except to home country
     * - "disabled" if no barring is active
     * @param barrings= international|internationalnothome|disabled, pin code
     * @return TRUE on success, otherwise FALSE
     */
    bool setVoiceCallOutgoing(const QString &barrings, const QString &pin);

private:
    OfonoCallForwarding *mCallForwarding;
    OfonoCallSettings *mCallSettings;
    OfonoCallBarring *mCallBarring;
    QVariantList list;
};

#endif // VOICECALLTEST_H
