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
 *  @file       TextChat.h
 *  @brief      Methods for managing text chat channel.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */


#ifndef _INCLUDED_TELEPATHY_TEXTMESSAGE_H
#define _INCLUDED_TELEPATHY_TEXTMESSAGE_H


#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Message>

class PendingOperationWaiter;
class MMSMessage;


class TextChat : public QObject
{
	Q_OBJECT
	
	public:
		TextChat( const Tp::TextChannelPtr& aChannel, QObject* pParent = 0 );
		virtual ~TextChat();
		
	public:
		
		bool SendMessage( const QString& message,
			    const QString& length,
			    const QString& coding,
			    const QString& classtype);

		QString SentMessageText() const { return mSentMessage; }

		bool VerifyReceivedMessage( const QString& sentMessage,
					const QString& sentLength,
					const QString& sentCoding,
					const QString& sentClasstype);

		bool VerifyReceivedIM();


	private Q_SLOTS:

		bool parseSms(const Tp::ReceivedMessage& message,const QString& sentMessage);

		    // From Text channel 
		void onMessageSent(const Tp::Message&,
				Tp::MessageSendingFlags,
				const QString&);
                void onMessageReceived(const Tp::ReceivedMessage&);
		void onPendingMessageRemoved(const Tp::ReceivedMessage&);
		void onChatStateChanged(const Tp::ContactPtr&,Tp::ChannelChatState);
        void onMessageLost();
        void onMessageReceived( uint ID,
								uint timestamp,
								uint sender,
								uint type,
								uint flags,
								const QString &text );
        void onMessageReceived( const Tp::MessagePartList &message );
	    
		// From timeout
		void callTimeout();
		
	private:
		
		Tp::TextChannelPtr			mChannel;
		PendingOperationWaiter*		mWaiter;
		QEventLoop*					mEventLoop;
		QTimer*						mTimer;
		bool						mRetValue;
		QString						mSentMessage;
		int							mReceivedMessageType;
		QString						mReceivedMessage;
		Tp::ReceivedMessage*		mReceivedMessageObject;
};


#endif // _INCLUDED_TELEPATHY_CALL_H
