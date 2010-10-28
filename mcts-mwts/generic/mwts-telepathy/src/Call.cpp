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

#include "stable.h"
#include "Call.h"
#include "PendingOperationWaiter.h"


const int CALL_TIMEOUT = 300000;

// Definitions for Stream Engine DBus API
#define STREAM_ENGINE_SRV    "org.maemo.Telepathy.StreamEngine"
#define STREAM_ENGINE_PATH   "/org/maemo/Telepathy/StreamEngine"
#define STREAM_ENGINE_IF     "org.maemo.Telepathy.StreamEngine"
#define STREAM_ENGINE_ATTACH "AttachToChannel"



Call::Call( const Tp::StreamedMediaChannelPtr& aChannel, QObject* pParent )
 : QObject( pParent ),
   mHaveErrors( false ),
   mChannel( aChannel ),
   mDirection( Tp::MediaStreamDirectionNone ),
   mState( Tp::MediaStreamStateDisconnected ),
   mHoldState( Tp::LocalHoldStateUnheld ),
   mWaiter( NULL ),
   mEventLoop( NULL ),
   mTimer( NULL ),
   mRetValue( false ),
   mIsInvalidated( false ),
   mIsGroupEmpty( false )
{
	MWTS_ENTER;
	
	mWaiter = PendingOperationWaiter::instance();
	mEventLoop = new QEventLoop;
	mTimer = new QTimer;
	connect( mTimer, SIGNAL(timeout()), SLOT(callTimeout()));
	
	Tp::Features feats;
	feats.insert( Tp::StreamedMediaChannel::FeatureContents );
	feats.insert( Tp::StreamedMediaChannel::FeatureStreams );
	feats.insert( Tp::StreamedMediaChannel::FeatureLocalHoldState );
	
	// Synchronize call to becomeReady
	// Execution will not continue, until becomeReady has finished
	bool ret = (*mWaiter)( mChannel->becomeReady( feats ) );
	if ( ret == true )
	{
		qDebug() << "Channel became successfully ready.";
		
		connect(mChannel.data(),
			SIGNAL(streamAdded(const Tp::MediaStreamPtr&)),
			SLOT(onStreamAdded(const Tp::MediaStreamPtr&)));
		connect(mChannel.data(),
			SIGNAL(streamRemoved(const Tp::MediaStreamPtr&)),
			SLOT(onStreamRemoved(const Tp::MediaStreamPtr&)));
		connect(mChannel.data(),
			SIGNAL(streamDirectionChanged(const Tp::MediaStreamPtr&,
							Tp::MediaStreamDirection,
							Tp::MediaStreamPendingSend)),
			SLOT(onStreamDirectionChanged(const Tp::MediaStreamPtr&,
							Tp::MediaStreamDirection,
							Tp::MediaStreamPendingSend)));
		connect(mChannel.data(),
			SIGNAL(streamStateChanged(const Tp::MediaStreamPtr&,
						Tp::MediaStreamState)),
			SLOT(onStreamStateChanged(const Tp::MediaStreamPtr&,
						Tp::MediaStreamState)));
		connect(mChannel.data(),
			SIGNAL(streamError(const Tp::MediaStreamPtr&,Tp::MediaStreamError,const QString&)),
			SLOT(onStreamError(const Tp::MediaStreamPtr&,Tp::MediaStreamError,const QString&)));

		connect(mChannel.data(),
			SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
			SLOT(onLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));
		
		connect(mChannel.data(),
			SIGNAL(groupMembersChanged(const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Channel::GroupMemberChangeDetails&)),
			SLOT(onGroupMembersChanged(const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Contacts&,
						   const Tp::Channel::GroupMemberChangeDetails&)));

		connect(mChannel.data(),
			SIGNAL(invalidated(Tp::DBusProxy*, const QString&, const QString&)),
			SLOT(onInvalidated(Tp::DBusProxy*, const QString&, const QString&)));
	}
	else
	{
		qWarning() << "Channel failed to become ready!";
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}


	// make sure the connection is ready with the contact roster and self
	// contact features
	feats.clear();
	feats.insert( Tp::Connection::FeatureRoster );
	feats.insert( Tp::Connection::FeatureSelfContact );
	ret = (*mWaiter)( mChannel->connection()->becomeReady( feats ) );
	if ( ret == true )
	{
		qDebug() << "Channel connection became successfully ready!";
	}
	else
	{
		qWarning() << "Channel connection failed to become ready!";
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	
	// Make sure we enable stream handling for those channels that require it
	if ( mChannel->handlerStreamingRequired() == true )
	{
		// Stream engine only need this one D-Bus call
		qDebug() << "Handler streaming is required!";
		QDBusMessage msg =
			QDBusMessage::createMethodCall( STREAM_ENGINE_SRV,
							STREAM_ENGINE_PATH,
							STREAM_ENGINE_IF,
							STREAM_ENGINE_ATTACH );
		msg << mChannel->busName();
		msg << QVariant::fromValue(
			QDBusObjectPath(mChannel->connection()->objectPath()) );
		msg << mChannel->channelType();
		msg << QVariant::fromValue(
			QDBusObjectPath(mChannel->objectPath()) );
		msg << mChannel->targetHandleType();
		msg << mChannel->targetHandle();
		qDebug() << "DBUS ARGS FOR STREAM-ENGINE:" << msg.arguments();
		QDBusMessage response = QDBusConnection::sessionBus().call( msg );
		if (response.type() == QDBusMessage::ErrorMessage)
		{
			qCritical()	<< "ATTACH TO S-E FAILED!"
					<< response.errorName() << ": " << response.errorMessage();
			MWTS_ERROR(QString("%1: %2").arg(response.errorName()).arg(response.errorMessage()));
			return;
		}
	}
	
	MWTS_LEAVE;
}


Call::~Call()
{
	MWTS_ENTER;
	if ( mEventLoop )
	{
		if ( mEventLoop->isRunning() )
		{
			mEventLoop->exit();
		}
		delete mEventLoop;
		mEventLoop = NULL;
	}
	
	if ( mTimer )
	{
		if ( mTimer->isActive() )
		{
			mTimer->stop();
		}
		delete mTimer;
		mTimer = NULL;
	}
	
	mChannel.reset();
	disconnect();
	MWTS_LEAVE;
}


const Tp::StreamedMediaChannelPtr& Call::Channel()
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return mChannel;
}


Tp::LocalHoldState Call::LocalHoldState()
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return mChannel->localHoldState();
}


bool Call::isInvalidated()
{
	MWTS_ENTER;
	qDebug() << "Is channel invalidated? " << ( mIsInvalidated ? "Yes" : "No" );
	MWTS_LEAVE;
	return mIsInvalidated;
}



bool Call::OpenAudioStream( const Tp::ContactPtr& aTarget )
{
	MWTS_ENTER;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qWarning() << "Channel unusable!";
		MWTS_LEAVE;
		return false;
	}
	qDebug() << "  Contact ID: " << aTarget->id();
	
	// Synchronize call to requestStream
	// Execution will not continue untill reqestStream has finished
	mRetValue = (*mWaiter)( mChannel->requestStream( aTarget, Tp::MediaStreamTypeAudio ) );
	if ( mRetValue == true )
	{
		qDebug() << "Succesfully requested stream.";
		// Wait, until out stream is either connected and bi-directional or it fails
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
	}
	else
	{
		qWarning()	<< "Failed to request stream!"
				<< mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}

	MWTS_LEAVE;
	return mRetValue;
}


bool Call::AcceptCall()
{
	MWTS_ENTER;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qCritical() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}
	
	// Synchronize call to acceptCall
	// Execution will not continue untill reqestStream has finished
	qDebug() << "Telepathy accept call...";
	bool ret = (*mWaiter)( mChannel->acceptCall() );
	if ( ret == true )
	{
		qDebug() << "Succesfully accepted call.";
		// Wait, until our stream is either connected and bi-directional or it fails
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
	}
	else
	{
		qWarning()	<< "Failed to accept call!"
				<< mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	
	MWTS_LEAVE;
	return mRetValue;
}


bool Call::HangUp( Tp::StreamedMediaChannel::StateChangeReason reason, const QString& detailedReason, const QString& message )
{
	MWTS_ENTER;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qWarning() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}

	switch ( reason )
	{
		case Tp::StreamedMediaChannel::StateChangeReasonUnknown:
			qDebug() << "Reason: Unknown";
			break;
		case Tp::StreamedMediaChannel::StateChangeReasonUserRequested:
			qDebug() << "Reason: user Requested";
			break;
	}
	qDebug() << "Detailed reason: " << detailedReason;
	qDebug() << "Message: " << message;

	// Synchronize call to requestStream
	// Execution will not continue until reqestStream has finished
	mRetValue = (*mWaiter)( mChannel->hangupCall( reason, detailedReason, message ) );
	if ( mRetValue == true )
	{
		qDebug() << "Succesfully hanged up.";
	}
	else
	{
		qWarning()	<< "Failed to hang up!"
				<< mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return mRetValue;
}


bool Call::Close()
{
	MWTS_ENTER;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qCritical() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}
	
	bool ret = HangUp();

	qDebug() << "Is invalidated: " << mIsInvalidated;
	qDebug() << "Is group empty: " << mIsGroupEmpty;

	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mEventLoop->exit();
	}
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}

	MWTS_LEAVE;
	return ret;
}


bool Call::Hold()
{
	MWTS_ENTER;
	mRetValue = false;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qWarning() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}

        if ( mHoldState == Tp::LocalHoldStateHeld || mHoldState == Tp::LocalHoldStatePendingHold )
        {
            qDebug() << "Call is already held or pending hold...";
            MWTS_LEAVE;
            return true;
        }

	// Synchronize call to requestHold
	// Execution will not continue, until requestHold has finished
	mRetValue = (*mWaiter)( mChannel->requestHold( true ) );
	if ( mRetValue == true )
	{
		qDebug() << "Succesfully held call.";
		// Wait, until the call is really held
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
	}
	else
	{
		qWarning() << "Failed to hold call!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return mRetValue;
}


bool Call::Unhold()
{
	MWTS_ENTER;
	mRetValue = false;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qWarning() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}

        if ( mHoldState == Tp::LocalHoldStateUnheld || mHoldState == Tp::LocalHoldStatePendingUnhold )
        {
            qDebug() << "Call is already unheld or pending unhold...";
            MWTS_LEAVE;
            return true;
        }

        // Synchronize call to requestHold
	// Execution will not continue, until requestHold has finished
	mRetValue = (*mWaiter)( mChannel->requestHold( false ) );
	if ( mRetValue == true )
	{
		qDebug() << "Succesfully unheld call.";
		// Wait, until the call is really unheld
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
	}
	else
	{
		qWarning() << "Failed to unhold call!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return mRetValue;
}


bool Call::SendDTMFTone( Tp::DTMFEvent aEvent )
{
	MWTS_ENTER;
	if ( mChannel.isNull() || mChannel->isReady() == false )
	{
		qWarning() << "Channel unusable!";
		MWTS_ERROR("Channel unusable!");
		MWTS_LEAVE;
		return false;
	}
	
	// Make sure we have audio streams to send the tones to
	Tp::MediaStreams streams = mChannel->streamsForType( Tp::MediaStreamTypeAudio );
	
	if ( streams.size() == 0 )
	{
		qDebug() << "No audio streams!";
		MWTS_ERROR("No audio streams!");
		MWTS_LEAVE;
		return false;
	}
	
	// Just take the first stream, we shouldn't have more
	Tp::MediaStreamPtr stream = streams.at( 0 );
	
	// Begin sending the tone, and synchronize the call
	// Execution will not continue, until startDTMFTone has finished
	if ( (*mWaiter)( stream->startDTMFTone( aEvent ) ) == false )
	{
		qCritical() << "Failed to start DTMF tone : "
			<< mWaiter->lastErrorName() << " : " 
			<< mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
		MWTS_LEAVE;
		return false;
	}
	
	// Wait for half a second before stopping DTMF tone
	QEventLoop waitLoop;
	QTimer::singleShot( 500, &waitLoop, SLOT(quit()) );
	waitLoop.exec();
	
	// Stop sending the tone, and synchronize the call
	// Execution will not continue, until stopDTMFTone has finished
	if ( (*mWaiter)( stream->stopDTMFTone() ) == false )
	{
		qCritical() << "Failed to stop DTMF tone : "
			<< mWaiter->lastErrorName() << " : "
			<< mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
		MWTS_LEAVE;
		return false;
	}
	
	MWTS_LEAVE;
	return true;
}


bool Call::StayOnLine( int aTime )
{
	MWTS_ENTER;

	// Use the instance variable for this, because all the error handling
	// set this variable to false.
	mRetValue = true;

	qDebug() << mEventLoop;
	qDebug() << mTimer;

	if ( mTimer )
	{
		qDebug() << "Is timer active? " << mTimer->isActive();
	}

	if ( mEventLoop )
	{
		qDebug() << "Is event loop running? " << mEventLoop->isRunning();
	}

	if ( mTimer && mTimer->isActive() )
	{
		qDebug() << "Stopping timer...";
		mTimer->stop();
	}
	if ( mEventLoop && mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mEventLoop->exit();
	}
	
	qDebug() << "Starting timer...";
	mTimer->start( aTime );

	qDebug() << "Starting event loop...";
	mEventLoop->exec();
	
	qDebug() << "Done!";

	MWTS_LEAVE;
	return mRetValue;
}


bool Call::MergeCall( const Tp::StreamedMediaChannelPtr& aChannel )
{
    MWTS_ENTER;
    bool mChannelCanMerge = mChannel->hasMergeableConferenceInterface();
    bool aChannelCanMerge = aChannel->hasMergeableConferenceInterface();
    qDebug() << "mChannel can merge? " << mChannelCanMerge;
    qDebug() << "aChannel can merge? " << aChannelCanMerge;

    bool ret = mChannel->hasMergeableConferenceInterface();
    if ( ret )
    {
        if ( aChannel->localHoldState() == Tp::LocalHoldStateHeld )
        {
            mRetValue = (*mWaiter)( aChannel->requestHold( false ) );
            if ( mRetValue == true )
            {
		qDebug() << "Succesfully unheld call.";
		// Wait, until the call is really unheld
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
            }
        }

        Tp::ChannelPtr chan = Tp::ChannelPtr( qobject_cast<Tp::Channel*>( aChannel.data() ) );
        Tp::PendingOperation* mergeOp = mChannel->conferenceMergeChannel( chan );

        // Merge the channels.
        // Execution will not continue, until conferenceMergeChannel operation has finished
        if ( (*mWaiter)( mergeOp ) == false )
        {
                qCritical() << "Failed to merge channels : "
                        << mWaiter->lastErrorName() << " : "
                        << mWaiter->lastErrorMessage();
				MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
                MWTS_LEAVE;
                return false;
        }
        ret = true;
    }
    else
    {
            qWarning() << "The channel doesn't have mergeable conference interface!";
			MWTS_ERROR("The channel doesn't have mergeable conference interface!");
    }
    MWTS_LEAVE;
    return ret;
}


void Call::callTimeout()
{
	MWTS_ENTER;
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		bool err = haveErrors();
		if ( err == true || mState == Tp::MediaStreamStateConnecting )
		{
			mRetValue = false;
		}
		else
		{
			mRetValue = true;
		}

		qDebug() << "err == " << err;
		qDebug() << "mState == " << mState;
		qDebug() << "mRetValue == " << mRetValue;
		mEventLoop->exit();
	}
	MWTS_LEAVE;
}


bool Call::haveErrors()
{
	MWTS_ENTER;
	qDebug() << "Have errors? " << ( mHaveErrors ? "Yes" : "No" );
	MWTS_LEAVE;
	return mHaveErrors;
}


void Call::onStreamAdded( const Tp::MediaStreamPtr& stream )
{
	MWTS_ENTER;
	qDebug() << "Call::onStreamAdded:" <<
		(stream->type() == Tp::MediaStreamTypeAudio ? "Audio" : "Video") <<
		"stream created";

	mDirection = stream->direction(); 
	if ( mDirection == Tp::MediaStreamDirectionSend )
	{
		qDebug() << "Direction: Sending";
	}
	else if ( mDirection == Tp::MediaStreamDirectionReceive )
	{
		qDebug() << "Direction: Receiving";
	}
	else if ( mDirection == Tp::MediaStreamDirectionBidirectional )
	{
		qDebug() << "Direction: Sending/Receiving";
	}
	else
	{
		qDebug() << "Direction: None";
	}
	mState = stream->state();
	if ( mState == Tp::MediaStreamStateDisconnected )
	{
		qDebug() << "State: Disconnected";
	}
	else if ( mState == Tp::MediaStreamStateConnecting )
	{
		qDebug() << "State: Connecting";
	}
	else if ( mState == Tp::MediaStreamStateConnected )
	{
		qDebug() << "State: Connected";
	}
	
	mHaveErrors = false;
	MWTS_LEAVE;
}


void Call::onStreamRemoved( const Tp::MediaStreamPtr& stream )
{
	MWTS_ENTER;
	qDebug() << "Call::onStreamRemoved:" <<
		(stream->type() == Tp::MediaStreamTypeAudio ? "Audio" : "Video") <<
		"stream removed";
	qDebug() << "Direction: None";
	qDebug() << "State: Disconnected";
	mHaveErrors = false;
	MWTS_LEAVE;
}


void Call::onStreamDirectionChanged(	const Tp::MediaStreamPtr& stream,
					Tp::MediaStreamDirection direction,
					Tp::MediaStreamPendingSend pendingSend )
{
	MWTS_ENTER;
	Q_UNUSED( pendingSend );

	qDebug() << "Call::onStreamDirectionChanged:" <<
		(stream->type() == Tp::MediaStreamTypeAudio ? "Audio" : "Video") <<
		"stream direction changed to" << direction;
	mDirection = direction;
	if ( mDirection == Tp::MediaStreamDirectionSend )
	{
		qDebug() << "Direction: Sending";
	}
	else if ( mDirection == Tp::MediaStreamDirectionReceive )
	{
		qDebug() << "Direction: Receiving";
	}
	else if ( mDirection == Tp::MediaStreamDirectionBidirectional )
	{
		qDebug() << "Direction: Sending/Receiving";
    }
	else
	{
		qDebug() << "Direction: None";
	}
	mHaveErrors = false;
	MWTS_LEAVE;
}


void Call::onStreamStateChanged(	const Tp::MediaStreamPtr &stream,
					Tp::MediaStreamState state )
{
	MWTS_ENTER;
	qDebug() << "Call::onStreamStateChanged:" <<
		(stream->type() == Tp::MediaStreamTypeAudio ? "Audio" : "Video") <<
		"stream state changed to" << state;
	mState = state;
	
	if ( mState == Tp::MediaStreamStateDisconnected )
	{
		qDebug() << "State: Disconnected";
		mHaveErrors = true;
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = false;
			mTimer->stop();
			mEventLoop->exit();
		}
	}
	else if ( mState == Tp::MediaStreamStateConnecting )
	{
		qDebug() << "State: Connecting";
	}
	else if ( mState == Tp::MediaStreamStateConnected )
	{
		qDebug() << "State: Connected";
		mHaveErrors = false;
                if ( mEventLoop->isRunning() && mDirection == Tp::MediaStreamDirectionBidirectional )
		{
                        qDebug() << "Event loop running and stream is bidirectional! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
			
		}
	}
	MWTS_LEAVE;
}


void Call::onStreamError( const Tp::MediaStreamPtr& stream,
			  Tp::MediaStreamError errorCode,
			  const QString& errorMessage )
{
	MWTS_ENTER;
	Q_UNUSED(stream);
	
	qDebug() << "Error code: ";
	switch ( errorCode )
	{
		case Tp::MediaStreamErrorUnknown:
			qDebug() << " Unknown";
			break;
		case Tp::MediaStreamErrorEOS:
			qDebug() << " End-Of-Stream";
			break;
		case Tp::MediaStreamErrorCodecNegotiationFailed:
			qDebug() << " Codec negotiation failed";
			break;
		case Tp::MediaStreamErrorConnectionFailed:
			qDebug() << " Connection failed";
			break;
		case Tp::MediaStreamErrorNetworkError:
			qDebug() << " Network error";
			break;
		case Tp::MediaStreamErrorNoCodecs:
			qDebug() << " No codecs";
			break;
		case Tp::MediaStreamErrorInvalidCMBehavior:
			qDebug() << " Invalid connection manager behavior";
			break;
		case Tp::MediaStreamErrorMediaError:
			qDebug() << " Media error";
			break;
	}
	qDebug() << "Error message: " << errorMessage;
	// Make sure out event loop isn't running
	mHaveErrors = true;
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mTimer->stop();
		mEventLoop->exit();
		
	}
	MWTS_LEAVE;
}


void Call::onLocalHoldStateChanged(	Tp::LocalHoldState state,
					Tp::LocalHoldStateReason reason )
{
	MWTS_ENTER;
        mHoldState = state;
        qDebug() << "Local hold state: ";
	mHaveErrors = false;
	switch ( state )
	{
		case Tp::LocalHoldStateUnheld:
			qDebug() << " Unheld";
			if ( mEventLoop->isRunning() )
			{
				qDebug() << "Event loop running! Stopping...";
				mRetValue = true;
				mTimer->stop();
				mEventLoop->exit();
			}
			break;
		case Tp::LocalHoldStateHeld:
			qDebug() << " Held";
			if ( mEventLoop->isRunning() )
			{
				qDebug() << "Event loop running! Stopping...";
				mRetValue = true;
				mTimer->stop();
				mEventLoop->exit();
			}
			break;
		case Tp::LocalHoldStatePendingHold:
			qDebug() << " Pending Hold";
			break;
		case Tp::LocalHoldStatePendingUnhold:
			qDebug() << " Pending Unhold";
			break;
	}
	qDebug() << "Local hold state reason: ";
	switch ( reason )
	{
		case Tp::LocalHoldStateReasonNone:
			qDebug() << " None";
			break;
		case Tp::LocalHoldStateReasonRequested:
			qDebug() << " Requested";
			break;
		case Tp::LocalHoldStateReasonResourceNotAvailable:
			qDebug() << " Resource not available";
			break;
	}
	MWTS_LEAVE;
}


void Call::onGroupMembersChanged( const Tp::Contacts& aGroupMembersAdded,
				  const Tp::Contacts& aGroupLocalPendingMembersAdded,
				  const Tp::Contacts& aGroupRemotePendingMembersAdded,
				  const Tp::Contacts& aGroupMembersRemoved,
				  const Tp::Channel::GroupMemberChangeDetails& aDetails )
{
	MWTS_ENTER;

	Q_UNUSED( aGroupMembersAdded );
	Q_UNUSED( aGroupLocalPendingMembersAdded );
	Q_UNUSED( aGroupRemotePendingMembersAdded );
	Q_UNUSED( aGroupMembersRemoved );

	if ( aDetails.hasReason() )
	{
		qDebug() << "Reason: " << aDetails.reason();
	}
	if ( aDetails.hasMessage() )
	{
		qDebug() << "Message: " << aDetails.message();
	}
	if ( aDetails.hasError() )
	{
		qDebug() << "Error: " << aDetails.error();
	}
	if ( aDetails.hasDebugMessage() )
	{
		qDebug() << "Debug message: " << aDetails.debugMessage();
	}

	qDebug() << "Number of group members added: " << aGroupMembersAdded.size();
	foreach ( Tp::ContactPtr contact, aGroupMembersAdded )
	{
		qDebug() << "  " << contact->id();
	}

	qDebug() << "Number of group local pending members added: " << aGroupLocalPendingMembersAdded.size();
	foreach ( Tp::ContactPtr contact, aGroupLocalPendingMembersAdded )
	{
		qDebug() << "  " << contact->id();
	}

	qDebug() << "Number of group remote pending members added: " << aGroupRemotePendingMembersAdded.size();
	foreach ( Tp::ContactPtr contact, aGroupRemotePendingMembersAdded )
	{
		qDebug() << "  " << contact->id();
	}

	qDebug() << "Number of group members removed: " << aGroupMembersRemoved.size();
	foreach ( Tp::ContactPtr contact, aGroupMembersRemoved )
	{
		qDebug() << "  " << contact->id();
	}

	qDebug() << "Group contact count: " << mChannel->groupContacts().count();

	// check if the call was answered or ended
	if ( !aGroupMembersAdded.isEmpty() &&
	     mChannel->groupContacts().count() >= 2 )
	{
		// contacts were added and there is now more than 2
		// participants --> the call is now active
		qDebug() << "Call was answered";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop is running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
		}
	}
	if ( !aGroupMembersRemoved.isEmpty() &&
	     mChannel->groupContacts().count() < 2 )
	{
		// contacts were removed from active or held call and the
		// number of participants is now less than 2
		// --> the call has ended
		qDebug() << "Call was rejected";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop is running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
		}
		mIsGroupEmpty = true;
	}
	MWTS_LEAVE;
}


void Call::onInvalidated( Tp::DBusProxy* proxy,
			  const QString& errorName,
			  const QString& errorMessage )
{
	MWTS_ENTER;
	Q_UNUSED(proxy);
	
	if ( proxy && proxy->isValid() )
	{
		QDBusConnection dbus = proxy->dbusConnection();
		QDBusError lastError = dbus.lastError();
		QDBusError::ErrorType lastErrorType = lastError.type();
		if ( lastErrorType != QDBusError::NoError )
		{
			mHaveErrors = false;
		}
		else
		{
			mHaveErrors = true;
			qDebug() << "Last error from D-Bus:";
			qDebug() << "Error type:";
			switch ( lastErrorType )
			{
				case QDBusError::Other:
					qDebug() << "  Other error, an error that is one of the well-known ones.";
					break;
				case QDBusError::Failed:
					qDebug() << "  The call failed.";
					break;
				case QDBusError::NoMemory:
					qDebug() << "  Out of memory.";
					break;
				case QDBusError::ServiceUnknown:
					qDebug() << "  The called service is not known.";
					break;
				case QDBusError::NoReply:
					qDebug() << "  The called method did not reply within the specified timeout.";
					break;
				case QDBusError::BadAddress:
					qDebug() << "  The address given is not valid.";
					break;
				case QDBusError::NotSupported:
					qDebug() << "  The call/operation is not supported.";
					break;
				case QDBusError::LimitsExceeded:
					qDebug() << "  The limits allocated to this process/call/connection exceeded the pre-defined values.";
					break;
				case QDBusError::AccessDenied:
					qDebug() << "  The call/operation tried to access a resource it isn't allowed to.";
					break;
				case QDBusError::NoServer:
					qDebug() << "  Documentation doesn't say what this is for.";
					break;
				case QDBusError::Timeout:
					qDebug() << "  Documentation doesn't say what this is for or how it's used.";
					break;
				case QDBusError::NoNetwork:
					qDebug() << "  Documentation doesn't say what this is for.";
					break;
				case QDBusError::AddressInUse:
					qDebug() << "  D-Bus server tried to bind to an address that is already in use.";
					break;
				case QDBusError::Disconnected:
					qDebug() << "  The call/process/message was sent after D-Bus connection disconnected.";
					break;
				case QDBusError::InvalidArgs:
					qDebug() << "  The arguments passed to this call/operation are not valid.";
					break;
				case QDBusError::UnknownMethod:
					qDebug() << "  The method called was not found in this object/interface with the given parameters.";
					break;
				case QDBusError::TimedOut:
					qDebug() << "  Documentation doesn't say...";
					break;
				case QDBusError::InvalidSignature:
					qDebug() << "  The type signature is not valid or compatible.";
					break;
				case QDBusError::UnknownInterface:
					qDebug() << "  The interface is not known.";
					break;
				case QDBusError::InternalError:
					qDebug() << "  An internal error occurred.";
					break;
				case QDBusError::UnknownObject:
					qDebug() << "  The remote object could not be found.";
					break;
				case QDBusError::NoError:
					qDebug() << "  No error.";
					break;
				case QDBusError::InvalidService:
					qDebug() << "  Invalid service.";
					break;
				case QDBusError::InvalidObjectPath:
					qDebug() << "  Invalid object path.";
					break;
				case QDBusError::InvalidInterface:
					qDebug() << "  Invalid interface.";
					break;
				case QDBusError::InvalidMember:
					qDebug() << "  Invalid member.";
					break;
			}
			qDebug() << lastError.name() << ": " << lastError.message();
		}
	}
	else
	{
		qWarning() << "NULL or invalid proxy!";
		mHaveErrors = true;
	}
	
	qDebug() << errorName << ": " << errorMessage;
	emit callEnded( errorName, errorMessage );

	// Make sure out event loop isn't running
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mTimer->stop();
		mEventLoop->exit();
	}
	mIsInvalidated = true;
	MWTS_LEAVE;
}


