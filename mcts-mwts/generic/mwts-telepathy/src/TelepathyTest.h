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

/**
 *  @addtogroup    MwtsTelepathy mwts-telepathy
 *  mwts-telepathy implements Telepathy test asset. It includes account
 *  creation process, requesting contact and channels. There is also
 *  specialized implementations for testing voice and VoIP calls and
 *  textual messaging.
 * 
 *  @{
 *
 *  @file       TelepathyTest.h
 *  @brief      Test assets main class. Creates account, requests contacts and opens channels.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#ifndef _INCLUDED_TELEPATHY_TEST_NEW_H
#define _INCLUDED_TELEPATHY_TEST_NEW_H

#include <MwtsCommon>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/ClientRegistrar>


class PendingOperationWaiter;
class ChannelHandlerClient;
class Call;
class TextChat;
class ChannelHandlerClient;
class TextChannelClient;
class StreamedMediaChannelClient;
class Contacts;
class ContactManagement;



/**
 *  @class TelepathyTest
 *  @brief Implements main functionality of the test asset
 *  
 *  This class is used as the main test asset code. Implements account
 *  creation, requesting contacts, creating channels and manages the
 *  specialized functionality of Call and TextChat.
 */
class TelepathyTest : public MwtsTest
{
	Q_OBJECT
	
	public:
		
		/**
		 *  @fn TelepathyTest( bool isCli = false, QObject* pParent = 0 )
		 *  @brief Constructor for TelepathyTest.
		 *  @param isCli Boolean value. True if class was constructed from command line application. False otherwise.
		 *  @param pParent Pointer to parent.
		 */
		TelepathyTest( bool isCli = false, QObject* pParent = 0 );
		
		/**
		 *  @fn virtual ~TelepathyTest()
		 *  @brief Virtual destructor for TelepathyTest.
		 */
		virtual ~TelepathyTest();
		
		/**
		 *  @fn virtual void OnInitialize()
		 *  @brief Virtual initialization function. Derived from MwtsTest.
		 */
		virtual void OnInitialize();
		
		/**
		 *  @fn virtual void OnUninitialize()
		 *  @brief Virtual uninitialization function. Derived from MwtsTest.
		 */
		virtual void OnUninitialize();
		
		/**
		 *  @fn virtual QString CaseName()
		 *  @brief Virtual function for deciding the test case name. Derived from MwtsTest.
		 *  @return Test case name.
		 */
		virtual QString CaseName();
		
	public: // Test asset API
		
		void StartLatencyMeasuring();
		int NextLatencyMeasure();
		int ReportOutgoingCallLatency();
		int ReportIncomingCallLatency();
		
		/**
		 *  @fn bool CreateAccountFromConf( const QString& conf )
		 *  @brief Reads given account configuration, and creates account from it.
		 *  @param conf Name of the account configuration
		 *  @return True, if successfull. False otherwise.
		 */
		bool CreateAccountFromConf( const QString& conf );
		
		/**
		 *  @fn bool CreateAccountFromPath( const QString& path )
		 *  @brief Creates an account from given D-Bus object path.
		 *  @param path The accounts D-Bus object path
		 *  @return True, if successfull. False otherwise.
		 */
		bool CreateAccountFromPath( const QString& path );
		
		/**
		 *  @fn bool RemoveAccount()
		 *  @brief Removes current account from account manager.
		 *  @return True, if successfull. False otherwise.
		 */
		bool RemoveAccount();
		
		/**
		 *  @fn bool SetEnabled( bool value )
		 *  @brief Enables or disables current account.
		 *  @param value Boolean value. True to enable and false to disable account.
		 *  @return True, if successfull. False otherwise.
		 */
		bool SetEnabled( bool value );
		
		/**
		 *  @fn bool SetRequestedPresence( const Tp::SimplePresence& value )
		 *  @brief Sets current accounts requested presence to given value.
		 *  @param value The wanted requested presence
		 *  @return True, if successfull. False otherwise.
		 */
		bool SetRequestedPresence( const Tp::SimplePresence& value );
		
		/**
		 *  @fn bool WaitForAccountConnected()
		 *  @brief Waits until account gets connected or timeout occurs.
		 *  @return True, if successfull. False otherwise.
		 */
		bool WaitForAccountConnected();
		
		/**
		 *  @fn bool ContactsForIdentifiers( const QStringList& identifiers )
		 *  @brief Requests contacts for given identifiers.
		 *  @param names Account names or phone numbers to convert to Tp::Contacts
		 *  @return True, if successfull. False otherwise.
		 */
		bool ContactsForIdentifiers( const QStringList& identifiers );
		
		/**
		 *  @fn bool RequestContacts( const QStringList& names )
		 *  @brief Requests contacts for given names.
		 *  @param names Account names or phone numbers to convert to Tp::Contacts
		 *  @return True, if successfull. False otherwise.
		 */
		bool RequestContacts( const QStringList& names );
		
		/**
		 *  @fn bool GetContacts( const QStringList& names )
		 *  @brief Reqeust contacts from accounts contact list.
		 *  @param names The names from contact list to convert to Tp::Contacts
		 *  @return True, if successfull. False otherwise.
		 */
		bool GetContacts( const QStringList& names );
		
		/**
		 *  @fn bool EnsureMediaCall( const QString& contactIdentifier, const QDateTime& userActionTime = QDateTime::currentDateTime(), const QString& preferredHandler = QString() )
		 *  @brief Request streamed media channel to be created.
		 *  @param contactIdentifier Account name or phone number to call to,
		 *  @param userActionTime Time and date for use as user action time. Default is current date and time.
		 *  @param preferredHandler D-Bus object path to preferred channel handler. Default is empty path.
		 *  @return True, if successfull. False otherwise.
		 */
		bool EnsureMediaCall(	const QString& contactIdentifier,
					const QDateTime& userActionTime = QDateTime::currentDateTime(),
					const QString& preferredHandler = QString() );
		
		/**
		 *  @fn bool ActivateCall()
		 *  @brief Activate a phone call from previously created streamed media channel.
		 *  @param bool speaker, True if uses speaker
		 *  @return True, if successfull. False otherwise.
		 */
		bool ActivateCall();
		
		/**
		 *  @fn bool EndCall()
		 *  @brief End the current phone call.
		 *  @return True, if successfull. False otherwise.
		 */
		bool EndCall();

		/**
		 *  @fn bool WaitForCallEnded()
		 *  @brief Waits until current call is ended or timeout occurs.
		 *  @return True, if successfull. False otherwise.
		 */
		bool WaitForCallEnded();
		
		/**
		 *  @fn bool WaitForIncomingCall()
		 *  @brief Waits until incoming call is received or timeout occurs.
		 *  @return True, if successfull. False otherwise.
		 */
		bool WaitForIncomingCall();

		 /**
		 *  @fn bool CreateConferenceCall()
		 *  @brief Creates conference call from all calls currently on hold or active.
		 *  @return True, if successfull. False otherwise.
		 */
		bool CreateConferenceCall();

		/**
		 *  @fn bool AcceptCall()
		 *  @brief Accept a call from previously received incoming streamed media channel.
		 *  @return True, if successfull. False otherwise.
		 */
        	bool AcceptCall();
		
		/**
		 *  @fn bool HoldCall()
		 *  @brief Hold the current call.
		 *  @return True, if successfull. False otherwise.
		 */
		bool HoldCall();
		
		/**
		 *  @fn bool UnholdCall()
		 *  @brief Unhold current call.
		 *  @return True, if successfull. False otherwise.
		 */
		bool UnholdCall();
		
		/**
		 *  @fn bool SendDTMFTone( Tp::DTMFEvent event )
		 *  @brief Send DTMF tone through current call.
		 *  @param event DTMF tone event to be sent
		 *  @return True, if successfull. False otherwise.
		 */
		bool SendDTMFTone( Tp::DTMFEvent event );
		
		/**
		 *  @fn bool Wait( int msec )
		 *  @brief Wait given amount of time.
		 *  @param msec Time, in milliseconds, to be waited
		 *  @return True, if successfull. False otherwise.
		 */
		bool Wait( int msec );
		
		/**
		 *  @fn bool StayOnLine( int msec )
		 *  @brief Wait given amount of time on line of the current call .
		 *  @param msec Time, in milliseconds, to be waited
		 *  @return True, if successfull. False otherwise.
		 */
		bool StayOnLine( int msec );
		
		/**
		 *  @fn bool RequestHandles( uint handleType, const QStringList& names )
		 *  @brief Request given names as certain handle types.
		 *  @param handleType Wanted hadle type
		 *  @param names List of requested names
		 *  @return True, if successfull. False otherwise.
		 */
		bool RequestHandles( uint handleType, const QStringList& names );
		
		/**
		 *  @fn bool EnsureTextChat( const QString& contactIdentifier, const QDateTime& userActionTime = QDateTime::currentDateTime(), const QString& preferredHandler = QString() )
		 *  @brief Request text channel to be created.
		 *  @param contactIdentifier Account name or phone number to send message to,
		 *  @param userActionTime Time and date for use as user action time. Default is current date and time.
		 *  @param preferredHandler D-Bus object path to preferred channel handler. Default is empty path.
		 *  @return True, if successfull. False otherwise.
		 */
		bool EnsureTextChat(	const QString& contactIdentifier,
					const QDateTime& userActionTime = QDateTime::currentDateTime(),
					const QString& preferredHandler = QString() );
		
		/**
		 *  @fn bool SendMessage( const QString& message, const QString& length, const QString& coding, const QString& classtype )
		 *  @brief Sends a message through current text channel.
		 *  @param message The message to be sent
		 *  @param lenght Length of the message
		 *  @param coding The coding used on the message
		 *  @param classType The messages class
		 *  @return True, if successfull. False otherwise.
		 */
		bool SendMessage( const QString& message,
				  const QString& length,
				  const QString& coding,
				  const QString& classtype );
		
		/**
		 *  @fn bool WaitForIncomingIM()
		 *  @brief Waits until incoming IM is received or timeout occurs.
		 *  @return True, if successfull. False otherwise.
		 */
		bool WaitForIncomingIM();

        //bool WaitForMmsAndParse(const QString& comparison_file);
        //bool SendMms(const QString& number, const QString& file);

	protected:  // From MwtsTest
		
		/**
		 *  @fn virtual void OnIdle()
		 *  @brief Virtual function derived from MwtsTest. This function is called when test asset is idling in some wait loop.
		 */
		virtual void OnIdle();
		
	private Q_SLOTS:
		
		// From account manager
		// THESE BECAME DEPRECATED
		void onAccountCreated( const QString& path );
		void onAccountRemoved( const QString& path );
		void onAccountValidityChanged( const QString& path, bool valid );
		
		void onNewAccount( const Tp::AccountPtr& account );

		void onPendingAccountFinished( Tp::PendingOperation* operation );

		// From account
		void onStateChanged( bool state );
		void onConnectionStatusChanged( Tp::ConnectionStatus status,
						Tp::ConnectionStatusReason statusReason );
		void onHaveConnectionChanged( bool haveConnection );
		
		// From accounts connection
		void onNewChannels( const Tp::ChannelDetailsList& channels );
		
		// From channel handlers
		void onIncomingChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel );
		void onIncomingStreamedMediaChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel );
		void onIncomingTextChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel );

		void onCallEnded( const QString& errorName, const QString& errorMessage );
		
		// From timeout
		void testTimeout();
		
	private:
		
		// To be used with CreateAccountFromPath
		bool isValidAccountPath( const QString& path );
		
		bool				mIsCli;
		
		ContactManagement*		mContactManagement;
		
		// Internal TelepathyQt object and their statuses
		Tp::AccountManagerPtr		mAccountMgr;
		Tp::AccountPtr			mAccount;
		Tp::Contacts			mContacts;
		bool				mAccountState;
		Tp::ConnectionStatus		mConnectionStatus;
		Tp::ConnectionStatusReason	mConnectionStatusReason;
		bool				mHaveConnection;
		
		// Object, that syncronizes Tp::PendingOperation obejct signaling
		PendingOperationWaiter*		mWaiter;
		
		// Channel handling
		Tp::ClientRegistrarPtr		mClientRegistrar;
		ChannelHandlerClient*		mChannelHandler;
		TextChannelClient*		mTextHandler;
		StreamedMediaChannelClient*	mStreamedMediaHandler;
		
		// Call object
		Call*				mCall;
		Tp::StreamedMediaChannelPtr	mConferenceCall;
		QList<Call*>			mCalls;
		TextChat*			mTextChat;
		QEventLoop*			mEventLoop;
		QTimer*				mTimer;
		bool				mRetValue;
		
		// For latency measurement
		QTime*				mLatencyMeasure;
		QTime*				mOutgoingCallMeasure;
		int				mOutgoingCallLatency;
		QTime*				mIncomingCallMeasure;
		int				mIncomingCallLatency;
		
		// Store info of sent text message for comparison
		QString mSentMessageContent;
		QString mSentMessageLength;
		QString mSentMessageCoding;
		QString mSentMessageClassType;
};

/**
 * @}
 */

#endif // _INCLUDED_TELEPATHY_TEST_NEW_H
