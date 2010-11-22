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
FeedbackTest::FeedbackTest() : effect(0)
{

        MWTS_ENTER;
        qDebug() << "in constructor ";
	std::cout << "i'm in constructor" << std::endl;

        MWTS_LEAVE;

        //export MWTS_DEBUG=1
        //export MWTS_LOG_PRINT=1
        //export DISPLAY=:0
        //source /tmp/session_bus_address.user
}

/**
 * Destructor for Feedback test class
 */
FeedbackTest::~FeedbackTest() {

        MWTS_ENTER;

        //MwtsApp::EnableUI(true);
        //g_pMainWindow

	qDebug() << "in destructor";

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void FeedbackTest::OnInitialize() {

        MWTS_ENTER;

        //duration = 1000;
        //intensity = 1.0;
        //errorIndicator = false;




	qDebug() << "in initialize";

        effect = new QFeedbackHapticsEffect();
        //effect->setDuration(duration);
        //effect->setIntensity(intensity);
	//effect->setDuration(1000);
        //effect->setIntensity(1.0);


        //connect(effect, SIGNAL(error(QFeedbackEffect::ErrorType)), this, SLOT(errorOccurs(QFeedbackEffect::ErrorType)));
        //connect(effect, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void FeedbackTest::OnUninitialize() {

        MWTS_ENTER;

	qDebug() << "in uninitialize";


        //disconnect();

        //if (effect) {
        //        effect->stop();
        //        delete effect;
        //        effect = NULL;
        //}

        MWTS_LEAVE;
}

void FeedbackTest::StartEffect() {

        MWTS_ENTER;

	qDebug() << "in starteffect";


        //if (effect) {
        //        effect->start();
        //}

        MWTS_LEAVE;
}

void FeedbackTest::PauseEffect() {

        MWTS_ENTER;

        //if (effect) {
        //        effect->pause();
        //}

        MWTS_LEAVE;
}

void FeedbackTest::StopEffect() {

        MWTS_ENTER;

        //if (effect) {
        //        effect->stop();
        //}

        MWTS_LEAVE;
}



void FeedbackTest::onErrorOccurs(QFeedbackEffect::ErrorType error) {

        MWTS_ENTER;

        //errorIndicator = true;

        MWTS_LEAVE;


}

void FeedbackTest::onStateChanged() {

        MWTS_ENTER;

        //qDebug() << "state changed to: " << effect->state();

        MWTS_LEAVE;

}


QFeedbackEffect::State FeedbackTest::EffectState() const {

        MWTS_ENTER;

        //if (effect) {
        //        return effect->state();
        //}

        MWTS_LEAVE;
}



bool FeedbackTest::ErrorIndicator() const {

        MWTS_ENTER;

        //return errorIndicator;

        MWTS_LEAVE;

	return false;

}

