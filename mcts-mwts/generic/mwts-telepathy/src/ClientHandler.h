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
 *  @file       ClientHandler.h
 *  @brief      Methods for managing channel handling client.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#ifndef _INCLUDED_CLIENT_HANDLER_H
#define _INCLUDED_CLIENT_HANDLER_H

#include <TelepathyQt4/AbstractClientHandler>


class PendingOperationWaiter;


/**
 *  @class ChannelhandlerClient
 *  @brief Implements channel handling client for both streamed media channels and text channels
 *  
 *  This class is used by Mission Control to check, wether our test asset
 *  wants to handle a newly created channel
 */
class ChannelHandlerClient : public QObject, public Tp::AbstractClientHandler
{
	Q_OBJECT
	
	public:
		
		/**
		 *  @fn create(const Tp::ChannelClassList& channelFilter, bool bypassApproval = false, bool wantsRequestNotification = false )
		 *  @brief Static member function for creating AbstractClient object.
		 *  @param channelFilter List of filters that signify supported channel features.
		 *  @param bypassApproval Boolean flag, that tells wether channel approval should be bypassed.
		 *  @param wantsRequestNotification Boolean flag, that tells wether request notifications are wanted.
		 *  @return Abstract client pointer.
		 */
		static Tp::AbstractClientPtr create(	Tp::ChannelClassList& aChannelFilter,
							const QStringList& aClientCapabilities,
							bool aWantsRequestNotification );
		
		/**
		 *  @fn ChannelHandlerClient(const Tp::ChannelClassList& channelFilter, bool bypassApproval = false, bool wantsRequestNotification = false )
		 *  @brief Constructor for ChannelHandlerClient.
		 *  @param channelFilter List of filters that signify supported channel features.
		 *  @param bypassApproval Boolean flag, that tells wether channel approval should be bypassed.
		 *  @param wantsRequestNotification Boolean flag, that tells wether request notifications are wanted.
		 */
		ChannelHandlerClient(	Tp::ChannelClassList& aChannelFilter,
					const QStringList& aClientCapabilities,
					bool aWantsRequestNotification );
		
		/**
		 *  @fn virtual ~ChannelHandlerClient()
		 *  @brief Destructor for ChannelHandlerClient.
		 */
		virtual ~ChannelHandlerClient();
		
		/**
		 *  @fn virtual bool bypassApproval() const
		 *  @brief This functions is used to identify wether we want to bypass channel approval.
		 *  @return True, if we want to bypass channel approval. False otherwise.
		 */
		virtual bool bypassApproval() const;

		/**
		 *  @fn virtual void handleChannels(const Tp::MethodInvocationContextPtr<>& context, const Tp::AccountPtr& account, const Tp::ConnectionPtr& connection, const QList<Tp::ChannelPtr>& channels, const QList<Tp::ChannelRequestPtr>& requestsSatisfied, const QDateTime& userActionTime, const QVariantMap& handlerInfo)
		 *  @brief This function is used to handle newly created channels.
		 *  @param context The context this function was called from.
		 *  @param account Telepathy account that these channels are for.
		 *  @param connection Telepathy connection object used in the channels offered throug this call.
		 *  @param channels The newly created channels.
		 *  @param requestsSatisfied Channel creation requests satisfied for this call.
		 *  @param userActionTime Time and date when user took action leading to this function call.
		 *  @param handlerInfo Handler information.
		 */
		virtual void handleChannels(const Tp::MethodInvocationContextPtr<>& context,
			const Tp::AccountPtr& account,
			const Tp::ConnectionPtr& connection,
			const QList<Tp::ChannelPtr>& channels,
			const QList<Tp::ChannelRequestPtr>& requestsSatisfied,
			const QDateTime& userActionTime,
			const QVariantMap& handlerInfo);
		
		/**
		 *  @var mChannels
		 *  @brief QList of the channels received from last call to handleChannels().
		 */
		QList<Tp::ChannelPtr>	mChannels;
		
		/**
		 *  @fn CloseChannels()
		 *  @brief Close and release any channels still left in the channel list.
		 */
		void CloseChannels();
	
	private:
		
		bool			mBypassApproval;
		PendingOperationWaiter*	mWaiter;

	Q_SIGNALS:
		void handleChannelsFinished();
		void channelClosed();
		void incomingChannelReceived( const QString& channelType, const Tp::ChannelPtr& aChannel );
};



/**
 * @}
 */

#endif // _INCLUDED_CLIENT_HANDLER_H
