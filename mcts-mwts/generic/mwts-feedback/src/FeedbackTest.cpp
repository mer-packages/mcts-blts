
/* FeedbackTest.cpp -- source code for FeedbackTest class
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
#include "stable.h"
#include "FeedbackTest.h"
#include <iostream>


QTM_USE_NAMESPACE

/**
 * Constructor for Feedback test class
 */
FeedbackTest::FeedbackTest()
{

        MWTS_ENTER;
        MWTS_LEAVE;
}

/**
 * Destructor for Feedback test class
 */
FeedbackTest::~FeedbackTest()
{

        MWTS_ENTER;
        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void FeedbackTest::OnInitialize()
{

        MWTS_ENTER;

        //default values
		int duration = 3000;
        int intensity = 1.0;

		effect = new QFeedbackHapticsEffect();
		effect->setDuration(duration);
		effect->setIntensity(intensity);

		debugMessage();

		PrintAvailableActuator();

		connect(effect, SIGNAL(error(QFeedbackEffect::ErrorType)), this, SLOT( onError(QFeedbackEffect::ErrorType)));
		connect(effect, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void FeedbackTest::OnUninitialize()
{

        MWTS_ENTER;

        disconnect();

		if (effect)
		{
				delete effect;
				effect = NULL;
        }

        MWTS_LEAVE;
}

void FeedbackTest::StartEffect()
{

        MWTS_ENTER;

		qDebug() << "starting the effect";
		effect->start();
		g_pTest->Start();

        MWTS_LEAVE;
}

void FeedbackTest::PauseEffect()
{

        MWTS_ENTER;

		effect->pause();

        MWTS_LEAVE;
}

void FeedbackTest::StopEffect()
{

        MWTS_ENTER;

		effect->stop();

        MWTS_LEAVE;
}

void FeedbackTest::debugMessage() const
{
	qDebug() << "| attack time " << effect->attackTime();
	qDebug() << "| attack intensity " << effect->attackIntensity();
	qDebug() << "| duration " << effect->duration();
	qDebug() << "| intensity " << effect->intensity();
	qDebug() << "| fade time " << effect->fadeTime();
	qDebug() << "| fade intensity " << effect->fadeIntensity();
	qDebug() << "| effect state " << effect->state();
}

void FeedbackTest::OnFailTimeout()
{
	g_pTest->Stop();
	qCritical() << "Effect was not played properly.";
	debugMessage();
}

void FeedbackTest::onError(QFeedbackEffect::ErrorType error)
{
		MWTS_ENTER;
		qCritical() << "Error occured, error type: " << error;
		MWTS_LEAVE;
}

void FeedbackTest::onStateChanged()
{
		MWTS_ENTER;

		switch (effect->state())
		{
		case QFeedbackEffect::Stopped:
			qDebug() << "Effect state changed to: Stopped";
			g_pTest->Stop();
			break;
		case QFeedbackEffect::Paused:
			qDebug() << "Effect state changed to: Paused";
			break;
		case QFeedbackEffect::Running:
			qDebug() << "Effect state changed to: Running";
			break;
		case QFeedbackEffect::Loading:
			qDebug() << "Effect state changed to: Loading";
			break;
		default:
			break;
		}

		MWTS_LEAVE;
}

void FeedbackTest::SetDuration(int miliseconds)
{
    MWTS_ENTER;
	effect->setDuration(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetIntensity(qreal intensity)
{
    MWTS_ENTER;
	effect->setIntensity(intensity);
    MWTS_LEAVE;
}

void FeedbackTest::SetAttackTime(int miliseconds)
{
    MWTS_ENTER;
	effect->setAttackTime(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetAttackIntensity(qreal intensity)
{
    MWTS_ENTER;
	effect->setAttackIntensity(intensity);
    MWTS_LEAVE;
}

void FeedbackTest::SetFadeTime(int miliseconds)
{
    MWTS_ENTER;
	effect->setFadeTime(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetFadeIntensity(qreal intensity)
{
    MWTS_ENTER;
	effect->setFadeIntensity(intensity);
    MWTS_LEAVE;
}

/* private */

void FeedbackTest::PrintAvailableActuator() const
{
	MWTS_ENTER;

	QList<QFeedbackActuator*> actuators = QFeedbackActuator::actuators();
	if (actuators.count() == 0)
	{
		qCritical() << "No actuators available in system - failing.";
		return;
	}
	foreach (QFeedbackActuator* act, actuators)
		qDebug() << act->name();


	MWTS_LEAVE;
}
