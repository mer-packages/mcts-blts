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
 *  @file       Call.h
 *  @brief      Methods for managing streamed media channel.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#ifndef _INCLUDED_TELEPATHY_CALL_H
#define _INCLUDED_TELEPATHY_CALL_H


#include <QObject>
#include <TelepathyQt4/StreamedMediaChannel>


class PendingOperationWaiter;


/**
 *  @class Call
 *  @brief Call specific implementation
 *  
 *  Class Call is built around channels of type Tp::StreamedMediaChannel.
 *  It is capable of handling all call related functionality.
 */
class Call : public QObject
{
	Q_OBJECT
	
	public:
		/**
		 *  @fn Call( const Tp::StreamedMediaChannelPtr& aChannel, QObject* pParent = 0 )
		 *  @brief Constructor for Call.
		 *  @param aChannel Tp::StreamedMediaChannel object that should be handled
		 *  @param pParent  Pointer to parent QObject. Default is NULL.
		 */
		Call( const Tp::StreamedMediaChannelPtr& aChannel, QObject* pParent = 0 );
		
		/**
		 *  @fn virtual ~Call()
		 *  @brief Destructor for Call.
		 */
		virtual ~Call();

		const Tp::StreamedMediaChannelPtr& Channel();
		Tp::LocalHoldState LocalHoldState();
		bool isInvalidated();
		
	public: // Call API
		
		/**
		 *  @fn OpenAudioStream( const Tp::ContactPtr& aTarget )
		 *  @brief Opens an audio stream, or create a call, to given contact.
		 *  @param aTarget Tp::Contact that should be called
		 *  @return True, if successfull. False otherwise.
		 */
		bool OpenAudioStream( const Tp::ContactPtr& aTarget );
		
		/**
		 *  @fn AcceptCall()
		 *  @brief Accepts an incoming call.
		 *  @return True, if successfull. False otherwise.
		 */
        bool AcceptCall();

		bool HangUp( Tp::StreamedMediaChannel::StateChangeReason reason
				= Tp::StreamedMediaChannel::StateChangeReasonUserRequested,
			     const QString& detailedReason = "",
			     const QString& message = "" );
		
		/**
		 *  @fn Close()
		 *  @brief Closes the Tp::StreamedMediaChannel.
		 *  @warning This function should only be called jsut before Call object is deleted.
		 *  @return True, if successfull. False otherwise.
		 */
		bool Close();
		
		
		/**
		 *  @fn Hold()
		 *  @brief Holds the current call.
		 *  @return True, if successfull. False otherwise.
		 */
		bool Hold();
		
		/**
		 *  @fn Unhold()
		 *  @brief Unholds the current call.
		 *  @return True, if successfull. False otherwise.
		 */
		bool Unhold();
		
		/**
		 *  @fn SendDTMFTone( Tp::DTMFEvent aEvent )
		 *  @brief Sends DTMF tone through the current audio stream.
		 *  @param aEvent DTMF tone event to be sent.
		 *  @return True, if successfull. False otherwise.
		 */
		bool SendDTMFTone( Tp::DTMFEvent aEvent );
		
		/**
		 *  @fn StayOnLine( int aTime )
		 *  @brief Starts a waiting loop for given period of time, while listening for stream errors.
		 *  @param aTime Time, in milliseconds, to stay on line.
		 *  @return True, if successfull. False otherwise.
		 */
		bool StayOnLine( int aTime );
		
		/**
		  *  @fn bool MergeCall( const Tp::StreamedMediaChannel& aChannel )
		  *  @brief Merges a call with the current one.
		  *  @param aChannel Streamed media channel to merge.
		  *  @return True, if merge was successfull. False otherwise.
		  */
		bool MergeCall( const Tp::StreamedMediaChannelPtr& aChannel );

	Q_SIGNALS:

		//void CallEnded( Call* aCall );
		void callEnded(	const QString& errorName, const QString& errorMessage );
	
	private Q_SLOTS:
		
		// From streamed media channel
                void onStreamAdded( const Tp::StreamedMediaStreamPtr& );
                void onStreamRemoved( const Tp::StreamedMediaStreamPtr& );
                void onStreamDirectionChanged(	const Tp::StreamedMediaStreamPtr&,
						Tp::MediaStreamDirection,
						Tp::MediaStreamPendingSend );
                void onStreamStateChanged( const Tp::StreamedMediaStreamPtr&,
					  Tp::MediaStreamState );
                void onStreamError(	const Tp::StreamedMediaStreamPtr,
					Tp::MediaStreamError errorCode,
					const QString& errorMessage );
		
		void onLocalHoldStateChanged(	Tp::LocalHoldState state,
						Tp::LocalHoldStateReason reason );
		
		void onGroupMembersChanged( const Tp::Contacts& aGroupMembersAdded,
					    const Tp::Contacts& aGroupLocalPendingMembersAdded,
					    const Tp::Contacts& aGroupRemotePendingMembersAdded,
					    const Tp::Contacts& aGroupMembersRemoved,
					    const Tp::Channel::GroupMemberChangeDetails& aDetails );

		// From DBusProxy
		void onInvalidated(	Tp::DBusProxy* proxy,
					const QString& errorName,
					const QString& errorMessage );
		
		// From timeout
		void callTimeout();
		
	private:
		
		// Internal error tracking
		bool				haveErrors();
		bool				mHaveErrors;
		
		// Objects from TelepathyQt, that are specific to calls
		Tp::StreamedMediaChannelPtr	mChannel;
		Tp::MediaStreamDirection	mDirection;
		Tp::MediaStreamState		mState;
		Tp::LocalHoldState			mHoldState;
		
		// Object, that syncronizes Tp::PendingOperation obejct signaling
		PendingOperationWaiter*		mWaiter;
		
		// For internal waiting
		QEventLoop*			mEventLoop;
		QTimer*				mTimer;
		bool				mRetValue;
		bool				mIsInvalidated;
		bool				mIsGroupEmpty;
};

/**
 * @}
 */


#endif // _INCLUDED_TELEPATHY_CALL_H
