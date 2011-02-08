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
 *  @file       TextChat.cpp
 *  @version    0.0.3
 *  @brief      Methods for managing text chat channel.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */


#include <QMimeData>
#include <QLabel>
#include "stable.h"
#include "TextChat.h"
#include "PendingOperationWaiter.h"



const int CALL_TIMEOUT = 120000;
#define MESSAGE_0 ""
#define MESSAGE_140 "message140message140message140message140message140message140message140"\
		"message140message140message140message140message140message140message140"
#define MESSAGE_160 "message160message160message160message160message160message160message160message160"\
		"message160message160message160message160message160message160message160message160"
#define MESSAGE_180 "message180message180message180message180message180message180message180message180"\
		"message180message180message180message180message180message180message180message180"\
		"message180message180"
#define MESSAGE_380 "message380message380message380message380message380message380message380"\
		"message380message380message380message380message380message380message380message380"\
		"message380message380message380message380message380message380message380message380"\
		"message380message380message380message380message380message380message380message380"\
		"message380message380message380message380message380message380message380"
#define MESSAGE_1000 "messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"\
		"messag1000messag1000messag1000messag1000messag1000"
/**
 * Constructor
 *
 * @param aChannel Channel name
 * @param pParent qobject parent
 */

TextChat::TextChat( const Tp::TextChannelPtr& aChannel, QObject* pParent )
 : QObject( pParent ), mChannel( aChannel ), mReceivedMessageObject( NULL )
{
	MWTS_ENTER;
	qDebug() << "Constructing TextChat object";
	bool ret = false;
	mWaiter = PendingOperationWaiter::instance();
	mEventLoop = new QEventLoop;
	mTimer = new QTimer;
	connect( mTimer, SIGNAL(timeout()), SLOT(callTimeout()));
		
	
	Tp::Features feats;
	feats.insert( Tp::TextChannel::FeatureMessageQueue );
	feats.insert( Tp::TextChannel::FeatureMessageCapabilities );
	feats.insert( Tp::TextChannel::FeatureMessageSentSignal );	

	if (!mChannel)
	{
		qDebug() << "Text channel does not exist!";
	}
	
	ret = (*mWaiter)( mChannel->becomeReady(  feats  ) );
	if ( ret == true )
	{
		// AccountManager is now ready
		qDebug() << "TextChannel became successfully ready.";
		
		connect(mChannel.data(),
			SIGNAL(messageSent(const Tp::Message&,
					   Tp::MessageSendingFlags,
					   const QString&)),
			SLOT(onMessageSent(const Tp::Message&,
					   Tp::MessageSendingFlags,
					   const QString&)));
		connect(mChannel.data(),
			SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
			SLOT(onMessageReceived(const Tp::ReceivedMessage&)));

		connect(mChannel.data(),
			SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
			SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));
			
			

		Tp::Client::ChannelTypeTextInterface* textInterface = mChannel->textInterface();
		if ( textInterface )
		{
	        connect(textInterface, SIGNAL(LostMessage()),this,SLOT(onMessageLost()));
	        connect(textInterface, SIGNAL(Received(uint,uint,uint,uint,uint,const QString&)),
							 this, SLOT(onMessageReceived(uint,uint,uint,uint,uint,const QString&)));
		}

		Tp::Client::ChannelInterfaceMessagesInterface* msgInterface = mChannel->messagesInterface();
		if ( msgInterface )
		{
	        connect(msgInterface, SIGNAL(MessageReceived(const Tp::MessagePartList&)),
							 this, SLOT(onMessageReceived(const Tp::MessagePartList&)));
		}
	}
	else
	{
		qWarning() << "Channel failed to become ready!";
		MWTS_ERROR("Channel failed to become ready!");
	}

	MWTS_LEAVE;
}

/**
 * Destructor
 *
 */
TextChat::~TextChat()
{
	MWTS_ENTER;
	if ( mReceivedMessageObject )
	{
		delete mReceivedMessageObject;
		mReceivedMessageObject = NULL;
	}
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

/**
 * SendMessage
 *
 * Gives text channel a message to be sent.
 * Text channel will return info of sending 
 * by using callback method onMessageSent
 *
 * @param message Message to be sent
 *
 */
bool TextChat::SendMessage( const QString& message,
			    const QString& length,
			    const QString& coding,
			    const QString& classtype)
{
	MWTS_ENTER;
	bool ret;
	qDebug() << "Sending message content: " << message;
	qDebug() << "Sending message length : " << length;
	qDebug() << "Sending message coding: " << coding;
	qDebug() << "Sending message class: " << classtype;
	
	if (!message.isEmpty())
	{
	  	mSentMessage = message;
	}
	else if (length == "140")
	{
		mSentMessage = MESSAGE_140;		
	}
	else if (length == "160")
	{
		mSentMessage = MESSAGE_160;		
	}
	else if (length == "180")
	{
		mSentMessage = MESSAGE_180;		
	}
	else if (length == "380")
	{
		mSentMessage = MESSAGE_380;
	}
	else if (length == "1000")
	{
		mSentMessage = MESSAGE_1000;		
	}
	else if (length == "0")
	{
		mSentMessage = MESSAGE_0;
	}	else
	{
		mSentMessage = "Hello world";
	}
	
	if ( !(*mWaiter)( mChannel->send( mSentMessage, static_cast<Tp::ChannelTextMessageType>(classtype.toInt()))))
	{
		MWTS_ERROR( "Failed to send text message!" );
		QString s = mWaiter->lastErrorName();
		s += ": ";
		s += mWaiter->lastErrorMessage();
		MWTS_ERROR( s ); 
		return false;
	}
	
	ret = (*mWaiter)( mChannel->requestClose() );
	if ( ret == true )
	{
		qDebug() << "Channel successfully closed.";
	}
	else
	{
		qWarning() << "Channel failed to close!";
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
		ret = false;
	}

	MWTS_LEAVE;
	return ret;
}


bool TextChat::VerifyReceivedMessage( const QString& sentMessage,
                                      const QString& sentLength,
                                      const QString& sentCoding,
                                      const QString& sentClasstype )
{
	MWTS_ENTER;

	mRetValue = false;
	mSentMessage = sentMessage;
	bool hasPendingMessages = false;
	QList<Tp::ReceivedMessage> acknowledgeMessageList;


	//check if the message has already arrived
	if( mChannel->messageQueue().isEmpty() == false )
	{
		qDebug() << "Messages in the queue: " << mChannel->messageQueue().count();
		foreach(const Tp::ReceivedMessage& message,mChannel->messageQueue())
		{
			qDebug() << "MessageParts in message: " << message.parts().count();
			qDebug() << "Expected message parts: " << message.size();
			// check if message is complete
			if(message.parts().count() == message.size())
			{
				if ( mRetValue == false )
				{
					mRetValue = parseSms( message,sentMessage );
				}
				acknowledgeMessageList.append(message);
			}
			else
				hasPendingMessages = true;
		}
		if ( mRetValue == false )
		{
			if ( mTimer->isActive() )
			{
				mTimer->stop();
			}
			if ( mEventLoop->isRunning() )
			{
				qDebug() << "Event loop running! Stopping...";
				mEventLoop->exit();
			}
			mTimer->start( CALL_TIMEOUT );
			mEventLoop->exec();
		}
	}
	else
	{
		if ( mTimer->isActive() )
		{
			mTimer->stop();
		}
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mEventLoop->exit();
		}
		mTimer->start( CALL_TIMEOUT );
		mEventLoop->exec();
	}
	qDebug() << "Messages in the queue: " << mChannel->messageQueue().count();
	foreach(const Tp::ReceivedMessage& message,mChannel->messageQueue())
	{
		qDebug() << "MessageParts in message: " << message.parts().count();
		qDebug() << "Expected message parts: " << message.size();
		// check if message is complete
		if(message.parts().count() == message.size())
		{
			if ( mRetValue == false )
			{
				mRetValue = parseSms( message,sentMessage );
			}
			acknowledgeMessageList.append(message);
		}
		else
			hasPendingMessages = true;
	}

	// aknowledge message as received
	mChannel->acknowledge( acknowledgeMessageList );

	MWTS_LEAVE;
	return mRetValue;
}



bool TextChat::VerifyReceivedIM()
{
	MWTS_ENTER;
	mRetValue = false;
    mSentMessage = "Hello world";
    bool hasPendingMessages = false;
    QList<Tp::ReceivedMessage> acknowledgeMessageList;


    //check if the message has already arrived
    if(!mChannel->messageQueue().isEmpty())
    {
        qDebug() << "Messages in the queue: " << mChannel->messageQueue().count();
        foreach(const Tp::ReceivedMessage& message,mChannel->messageQueue())
        {
            qDebug() << "MessageParts in message: " << message.parts().count();
            qDebug() << "Expected message parts: " << message.size();
            // check if message is complete
            if(message.parts().count() == message.size())
            {
            	if ( mRetValue == false )
            	{
                    mRetValue = ( message.text().compare( mSentMessage ) == 0 );
            	}
                acknowledgeMessageList.append(message);
            }
            else
                hasPendingMessages = true;
        }
        if ( mRetValue == false )
        {
        	if ( mTimer->isActive() )
        	{
        		mTimer->stop();
        	}
        	if ( mEventLoop->isRunning() )
        	{
        		qDebug() << "Event loop running! Stopping...";
        		mEventLoop->exit();
        	}
        	mTimer->start( CALL_TIMEOUT );
        	mEventLoop->exec();
        }
    }
    else
    {
    	if ( mTimer->isActive() )
    	{
    		mTimer->stop();
    	}
    	if ( mEventLoop->isRunning() )
    	{
    		qDebug() << "Event loop running! Stopping...";
    		mEventLoop->exit();
    	}
    	mTimer->start( CALL_TIMEOUT );
    	mEventLoop->exec();
    }

    qDebug() << "Messages in the queue: " << mChannel->messageQueue().count();

    mChannel->acknowledge( acknowledgeMessageList );


    MWTS_LEAVE;
    return mRetValue;
}



bool TextChat::parseSms(const Tp::ReceivedMessage& message,const QString& sentMessage)
{
    MWTS_ENTER;

    switch ( message.messageType() )
    {
        case Tp::ChannelTextMessageTypeNormal:
            qDebug() << "Message type: Normal";
            break;
        case Tp::ChannelTextMessageTypeAction:
            qDebug() << "Message type: Action";
            break;
        case Tp::ChannelTextMessageTypeNotice:
            qDebug() << "Message type: Notice";
            break;
        case Tp::ChannelTextMessageTypeAutoReply:
            qDebug() << "Message type: Auto-Reply";
            break;
        case Tp::ChannelTextMessageTypeDeliveryReport:
            qDebug() << "Message type: Delivery Report";
            break;
    }

    if ( message.messageType() == Tp::ChannelTextMessageTypeDeliveryReport )
    {
        Tp::MessagePart header = message.header();
        if ( header.contains( "delivery-status" ) == true )
        {
            QDBusVariant deliveryStatusVariant = header.value( "delivery-status" );
            if ( deliveryStatusVariant.variant().canConvert( QVariant::UInt ) )
            {
                bool ok = false;
                uint deliveryStatus = deliveryStatusVariant.variant().toUInt( &ok );
                if ( ok )
                {
                    switch ( deliveryStatus )
                    {
                        case 0:
                            qDebug() << "Delivery status: Unknown (0)";
                            break;
                        case 1:
                            qDebug() << "Delivery status: Delivered (1)";
                            break;
                        case 2:
                            qCritical() << "Delivery status: Temporarily_Failed (2)";
                            break;
                        case 3:
                            qCritical() << "Delivery status: Permanently_Failed (3)";
                            break;
                        case 4:
                            qDebug() << "Delivery status: Accepted (4)";
                            break;
                    }
                }
            }
        }
    }
    else if ( message.messageType() == Tp::ChannelTextMessageTypeNormal )
    {
            qDebug() << "Sent message: " << sentMessage;
            qDebug() << "Received message: " << message.text();
            mRetValue = ( QString::compare( sentMessage, message.text() ) == 0 ? true : false );
    }

    MWTS_LEAVE;
    return mRetValue;
}



/**
 * callTimeout
 * 
 * Slot function for timeout callbacks.
 * Used by mwts-common to stop testcase which has gone overtime
 * 
 * No params
 */
void TextChat::callTimeout()
{
	MWTS_ENTER;
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mEventLoop->exit();
	}
	MWTS_LEAVE;
}


/**
 * onMessageSent
 *
 * Callback function to inform that message is sent.
 * Used by TextChannel.
 *
 * @param message Sent message
 * @param flags Sending flags of message
 * @param sentMessageToken Token 
 *
 */
void TextChat::onMessageSent(const Tp::Message &message,
		Tp::MessageSendingFlags flags,
		const QString &sentMessageToken)
{
	MWTS_ENTER;
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mEventLoop->exit();
	}
	qDebug() << "Message: " << message.text();
        mSentMessage = message.text();
	//qDebug() << "Sent message token: " << sentMessageToken;

	MWTS_LEAVE;
}

/**
 * onMessageReceived
 *
 * Callback function to inform that message is received
 * Used by TextChannel.
 * 
 * @param message Received message
 *
 */
void TextChat::onMessageReceived(const Tp::ReceivedMessage &message)
{
	MWTS_ENTER;

    qDebug() << "Message received!";
    qDebug() << "Timestamp: " << message.received();
    qDebug() << "Sender: " << message.sender();
    switch ( message.messageType() )
    {
		case Tp::ChannelTextMessageTypeNormal:
			qDebug() << "Type: Normal";
			break;
		case Tp::ChannelTextMessageTypeAction:
			qDebug() << "Type: Action";
			break;
		case Tp::ChannelTextMessageTypeNotice:
			qDebug() << "Type: Notice";
			break;
		case Tp::ChannelTextMessageTypeAutoReply:
			qDebug() << "Type: Auto Reply";
			break;
		case Tp::ChannelTextMessageTypeDeliveryReport:
			qDebug() << "Type: Delivery Report";
			break;
    }
    qDebug() << "Text: " << message.text();

    if ( message.messageType() == Tp::ChannelTextMessageTypeNormal )
    {
    	if ( mTimer->isActive() )
        {
            mTimer->stop();
        }
        if ( mEventLoop->isRunning() )
        {
            qDebug() << "Event loop running! Stopping...";
            mEventLoop->exit();
        }

        mRetValue = ( mSentMessage.compare( message.text() ) == 0 ? true : false );
        if ( mReceivedMessageObject == NULL )
        {
        	qDebug() << "Creating new message object...";
        	mReceivedMessageObject = new Tp::ReceivedMessage( message );
        }
        else
        {
        	qDebug() << "Recreating message object...";
        	delete mReceivedMessageObject;
        	mReceivedMessageObject = new Tp::ReceivedMessage( message );
        }
    }

	MWTS_LEAVE;
}

/**
 * onPendingMessageReceived
 *
 * Callback function to inform that message is received
 * Used by TextChannel.
 * 
 * @param message Received message
 *
 */
void TextChat::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
	MWTS_ENTER;
	/*if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mEventLoop->exit();
	}*/
	qDebug() << "Message: " << message.text();

	MWTS_LEAVE;
}

void TextChat::onMessageLost()
{
    MWTS_ENTER;
    /*if ( mTimer->isActive() )
    {
        mTimer->stop();
    }
    if ( mEventLoop->isRunning() )
    {
        qDebug() << "Event loop running! Stopping...";
        mEventLoop->exit();
    }*/
    qWarning() << "Message Lost!";

    MWTS_LEAVE;
}


void TextChat::onMessageReceived(	uint ID, uint timestamp, uint sender,
									uint type, uint flags, const QString &text )
{
	MWTS_ENTER;

    qDebug() << "Message received!";
    qDebug() << "ID: " << ID;
    qDebug() << "Timestamp: " << timestamp;
    qDebug() << "Sender: " << sender;
    switch ( type )
    {
		case Tp::ChannelTextMessageTypeNormal:
			qDebug() << "Type: Normal";
			break;
		case Tp::ChannelTextMessageTypeAction:
			qDebug() << "Type: Action";
			break;
		case Tp::ChannelTextMessageTypeNotice:
			qDebug() << "Type: Notice";
			break;
		case Tp::ChannelTextMessageTypeAutoReply:
			qDebug() << "Type: Auto Reply";
			break;
		case Tp::ChannelTextMessageTypeDeliveryReport:
			qDebug() << "Type: Delivery Report";
			break;
    }
    qDebug() << "Flags: " << flags;
    qDebug() << "Text: " << text;

    if ( type == Tp::ChannelTextMessageTypeNormal )
    {
    	if ( mTimer->isActive() )
        {
            mTimer->stop();
        }
        if ( mEventLoop->isRunning() )
        {
            qDebug() << "Event loop running! Stopping...";
            mEventLoop->exit();
        }

        mRetValue = ( mSentMessage.compare( text ) == 0 ? true : false );
    }

	MWTS_LEAVE;
}


void TextChat::onMessageReceived( const Tp::MessagePartList& message )
{
	MWTS_ENTER;

    qDebug() << "Message received!";

    foreach ( Tp::MessagePart part, message )
    {
        Tp::MessagePart::const_iterator i = part.constBegin();
    	while ( i != part.constEnd() )
    	{
    		if ( QString( "message-type" ).compare( i.key() ) == 0 )
    		{
    			QVariant variant = i.value().variant();
    			if ( variant.canConvert( QVariant::UInt ) )
    			{
    				bool ok = false;
    				uint type = variant.toUInt( &ok );
    				if ( ok )
    				{
    				    switch ( type )
    				    {
    						case Tp::ChannelTextMessageTypeNormal:
    							qDebug() << "Message type: Normal";
    							break;
    						case Tp::ChannelTextMessageTypeAction:
    							qDebug() << "Message type: Action";
    							break;
    						case Tp::ChannelTextMessageTypeNotice:
    							qDebug() << "Message type: Notice";
    							break;
    						case Tp::ChannelTextMessageTypeAutoReply:
    							qDebug() << "Message type: Auto Reply";
    							break;
    						case Tp::ChannelTextMessageTypeDeliveryReport:
    							qDebug() << "Message type: Delivery Report";
    							break;
    				    }
    				    if ( type == Tp::ChannelTextMessageTypeNormal )
    				    {
    				    	if ( mTimer->isActive() )
    				        {
    				            mTimer->stop();
    				        }
    				        if ( mEventLoop->isRunning() )
    				        {
    				            qDebug() << "Event loop running! Stopping...";
    				            mEventLoop->exit();
    				        }

    				        mRetValue = true;
    				    }
    				}
    			}
    		}
    		else
    		{
        		qDebug() << i.key() << ": " << i.value().variant();
    		}
    		++i;
    	}
    }

	MWTS_LEAVE;
}

void TextChat::onChatStateChanged(const Tp::ContactPtr &contact,Tp::ChannelChatState state)
{
	MWTS_ENTER;
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mEventLoop->exit();
	}
	qDebug() << "State changed: " << state;

	MWTS_LEAVE;
}
