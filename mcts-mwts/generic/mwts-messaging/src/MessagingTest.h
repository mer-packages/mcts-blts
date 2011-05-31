/* MessagingTest.h -- Header file for Messaging test class

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _INCLUDED_MESSAGING_TEST_H
#define _INCLUDED_MESSAGING_TEST_H

#include <MwtsCommon>
#include <qmessagemanager.h>
#include <qmessageservice.h>
#include <QTime>

QTM_USE_NAMESPACE;


/**
 * Messaging test class
 * Demonstrates how to create test classes
 * Inherited from MwtsTest class
 */
class MessagingTest : public MwtsTest
{
	Q_OBJECT

	public: // Functions

		/**
		 * Constructor for Messaging test class
		 */
		MessagingTest();

		/**
		 * Destructor for Messaging test class
		 */
		virtual ~MessagingTest();

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

		void TestSendingIM();

		/**
		 * Sets currently used message type
		 */
		void SetMessageType( QMessage::Type type );

		/**
		 * Write latencies to report
		 */
		void ReportLatencies();

		/**
		 * Queries all accounts available for Qt Mobility Messaging
		 */
		void QueryAllAccounts();

		/**
		 * Prints preferred charsets for messages
		 */
		void PreferredCharsets();

		/**
		 * Send SMS to self number
		 */
		void SendSMS( const QString& message );

		/**
		 * Send SMS to self number with given encoding
		 */
		void SendSMS( const QString& message, const QString& encoding );

		/**
		 * Send SMS to self number with given MIB of a known encoding
		 */
		void SendSMS( const QString& message, int mib );

		/**
		 * Send E-Mail message
		 */
		void SendEmail( const QString& subject, const QString& message );

		/**
		 * Waits for incoming message
		 */
		void WaitForIncomingMessage();

	private slots: // Slots

		// From QMessageManager
		void onMessageAdded( const QMessageId& id,
				const QMessageManager::NotificationFilterIdSet& matchingFilterIds );
		void onMessageRemoved( const QMessageId& id,
				const QMessageManager::NotificationFilterIdSet& matchingFilterIds );
		void onMessageUpdated( const QMessageId& id,
				const QMessageManager::NotificationFilterIdSet& matchingFilterIds );

		// From QMessageService
		void onMessagesCounted( int count );
		void onMessagesFound( const QMessageIdList& ids );
		void onProgressChanged( uint value, uint total );
		void onStateChanged( QMessageService::State newState );

		void verifyMessage();

	private: // Functions

		bool sendMessage( QMessage& message );


	private: // Data

		QMessageService*						mMsgService;
		QMessageManager*						mMsgManager;

		QString									mSelfNumber;
		QString									mEmailMasterName;
		QString									mEmailSlaveAddress;
		QString									mMsgBody;

		bool									mRetValue;

		QMessageManager::NotificationFilterId	mFilterId;

		QTime									mSendTime;
		QTime									mReceiveTime;
		int										mSendLatency;
		int										mReceiveLatency;

		QMessageId								mReceivedMessage;

		QEventLoop								mEventLoop;
};

#endif // _INCLUDED_MESSAGING_TEST_H

