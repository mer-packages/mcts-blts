/* FeedbackTest.h -- Header file for Feedback test class
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

#ifndef _INCLUDED_FEEDBACK_TEST_H
#define _INCLUDED_FEEDBACK_TEST_H

#include <MwtsCommon>
#include <QFeedbackHapticsEffect>

QTM_USE_NAMESPACE;


/**
 * Feedback test class
 * Demonstrates how to create test classes
 * Inherited from MwtsTest class
 */
class FeedbackTest : public MwtsTest {

        Q_OBJECT

        public:
                /**
                 * Contructor
                 */
                FeedbackTest();

                /**
                 * Destructor
                 */
                virtual ~FeedbackTest();

                /**
                 * Overridden functions for MwtsTest class
                 * OnInitialize is called before test execution
                 */
                void OnInitialize();

                /**
                 * Overridden functions for MwtsTest class
                 * OnUninitialize is called after test execution
                 */
                void OnUninitialize();

                /**
                 * Starts the effect
                 * Switching state of effect to QFeedbackEffect::Running (2)
                 */
                void StartEffect();

                /**
                 * Pauses the effect
                 * Switching state of effect to QFeedbackEffect::Paused (1)
                 */
                void PauseEffect();

                /**
                 * Stops the effect
                 * Switching state of effect to QFeedbackEffect::Stopped (0)
                 */
                void StopEffect();

                /**
                 *  Sets duration of object pointed by mEffect
                 *  @param miliseconds duration time
                 */
                void SetDuration(int miliseconds);

                /**
                 *  Sets intensity of object pointed by mEffect
                 *  @param intensity value between 0 and 1
                 */
                void SetIntensity(qreal intensity);



                /**
                 *  Sets attack duration of object pointed by mEffect
                 *  @param miliseconds
                 */
                void SetAttackTime(int miliseconds);

                /**
                 *  Sets attack intensity of object pointed by mEffect
                 *  @param
                 */
                void SetAttackIntensity(qreal intensity);

                /**
                 *  Sets fade duration of object pointed by mEffect
                 *  @param miliseconds
                 */
                void SetFadeTime(int miliseconds);

                /**
                 *  Sets fade intensity of object pointed by mEffect
                 *  @param
                 */
                void SetFadeIntensity(qreal intensity);

				void debugMessage() const;

				void OnFailTimeout();

        private slots:
				void onError(QFeedbackEffect::ErrorType error);
                void onStateChanged();


        private:
				QFeedbackHapticsEffect* effect;


};


#endif // _INCLUDED_FEEDBACK_TEST_H

