
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
FeedbackTest::FeedbackTest() : mEffect(0) {

        MWTS_ENTER;
        MWTS_LEAVE;
}

/**
 * Destructor for Feedback test class
 */
FeedbackTest::~FeedbackTest() {

        MWTS_ENTER;
        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void FeedbackTest::OnInitialize() {

        MWTS_ENTER;

        //default values
        int duration = 1000;
        int intensity = 1.0;
        mErrorIndicator = false;

	qDebug() << "in initialize";

        mEffect = new QFeedbackHapticsEffect();
        mEffect->setDuration(duration);
        mEffect->setIntensity(intensity);

        qDebug() << "| attack time " << mEffect->attackTime();
        qDebug() << "| attack intensity " << mEffect->attackIntensity();
        qDebug() << "| duration " << mEffect->duration();
        qDebug() << "| intensity " << mEffect->intensity();
        qDebug() << "| fade time " << mEffect->fadeTime();
        qDebug() << "| fade intensity " << mEffect->fadeIntensity();


        connect(mEffect, SIGNAL(error(QFeedbackEffect::ErrorType)), this, SLOT( onErrorOccurs(QFeedbackEffect::ErrorType)));
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
        if (mEffect) {
            qDebug() << "| attack time " << mEffect->attackTime();
            qDebug() << "| attack intensity " << mEffect->attackIntensity();
            qDebug() << "| duration " << mEffect->duration();
            qDebug() << "| intensity " << mEffect->intensity();
            qDebug() << "| fade time " << mEffect->fadeTime();
            qDebug() << "| fade intensity " << mEffect->fadeIntensity();
        }


        disconnect();

        if (mEffect) {
                mEffect->stop();
                delete mEffect;
                mEffect = 0;
        }

        MWTS_LEAVE;
}

void FeedbackTest::StartEffect() {

        MWTS_ENTER;

        if (mEffect) {
            qDebug() << "starting the effect";
            mEffect->start();
        }

        MWTS_LEAVE;
}

void FeedbackTest::PauseEffect() {

        MWTS_ENTER;

        if (mEffect) {
                mEffect->pause();
        }

        MWTS_LEAVE;
}

void FeedbackTest::StopEffect() {

        MWTS_ENTER;

        if (mEffect) {
                mEffect->stop();
        }

        MWTS_LEAVE;
}



void FeedbackTest::onErrorOccurs(QFeedbackEffect::ErrorType error) {

        MWTS_ENTER;

        mErrorIndicator = true;

        MWTS_LEAVE;
}

void FeedbackTest::onStateChanged() {

        MWTS_ENTER;

        //qDebug() << "state changed to: " << mEffect->state();

        MWTS_LEAVE;
}

QFeedbackEffect::State FeedbackTest::EffectState() const {

        MWTS_ENTER;

        if (mEffect) {
                return mEffect->state();
        }

        MWTS_LEAVE;
}

bool FeedbackTest::ErrorIndicator() const {
        MWTS_ENTER;
        MWTS_LEAVE;
        return mErrorIndicator;
}


void FeedbackTest::SetDuration(int miliseconds) {
    MWTS_ENTER;
    mEffect->setDuration(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetIntensity(qreal intensity) {
    MWTS_ENTER;
    mEffect->setIntensity(intensity);
    MWTS_LEAVE;
}

void FeedbackTest::SetAttackTime(int miliseconds) {
    MWTS_ENTER;
    mEffect->setAttackTime(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetAttackIntensity(qreal intensity) {
    MWTS_ENTER;
    mEffect->setAttackIntensity(intensity);
    MWTS_LEAVE;
}

void FeedbackTest::SetFadeTime(int miliseconds) {
    MWTS_ENTER;
    mEffect->setFadeTime(miliseconds);
    MWTS_LEAVE;
}

void FeedbackTest::SetFadeIntensity(qreal intensity) {
    MWTS_ENTER;
    mEffect->setFadeIntensity(intensity);
    MWTS_LEAVE;
}

QFeedbackHapticsEffect* FeedbackTest::Effect() const {
    MWTS_ENTER;
    return mEffect;
    MWTS_LEAVE;
}



