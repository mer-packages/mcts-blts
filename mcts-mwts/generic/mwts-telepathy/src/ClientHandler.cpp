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
#include "ClientHandler.h"
#include "PendingOperationWaiter.h"



Tp::AbstractClientPtr ChannelHandlerClient::create(	Tp::ChannelClassSpecList& aChannelFilter,
							const QStringList& aClientCapabilities,
							bool aWantsRequestNotification )
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return Tp::AbstractClientPtr::dynamicCast(Tp::SharedPtr<ChannelHandlerClient>(
                        new ChannelHandlerClient(	aChannelFilter,
							aClientCapabilities,
							aWantsRequestNotification ) ) );
}

ChannelHandlerClient::ChannelHandlerClient(	Tp::ChannelClassSpecList& aChannelFilter,
						const QStringList& aClientCapabilities,
                                                bool aWantsRequestNotification )
        : Tp::AbstractClientHandler(	aChannelFilter,
					aClientCapabilities,
					aWantsRequestNotification ),
	mWaiter( PendingOperationWaiter::instance() )

{
	MWTS_ENTER;
	MWTS_LEAVE;
}

ChannelHandlerClient::~ChannelHandlerClient()
{
	MWTS_ENTER;
	CloseChannels();
	disconnect();
	MWTS_LEAVE;
}


bool ChannelHandlerClient::bypassApproval() const
{
	MWTS_ENTER;
	MWTS_LEAVE;
	return mBypassApproval;
}


void ChannelHandlerClient::CloseChannels()
{
	MWTS_ENTER;
	foreach (const Tp::ChannelPtr& channel, mChannels)
	{
		bool ret = (*mWaiter)( channel->requestClose() );
		if ( ret == true )
		{
			qDebug() << "Channel successfully closed.";
		}
		else
		{
			qWarning() << "Channel failed to close!";
		}
	}
	mChannels.clear();
	MWTS_LEAVE;
}



void ChannelHandlerClient::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                            const Tp::AccountPtr &account,
                            const Tp::ConnectionPtr &connection,
                            const QList<Tp::ChannelPtr> &channels,
                            const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                            const QDateTime &userActionTime,
                            const HandlerInfo &handlerInfo)
{
	MWTS_ENTER;
	Q_UNUSED( account );
	Q_UNUSED( connection );
	Q_UNUSED( requestsSatisfied );
	Q_UNUSED( userActionTime );
	Q_UNUSED( handlerInfo );
	
	mChannels = channels;
        qDebug() << "Number of channels received: " << mChannels.size();
	foreach (const Tp::ChannelPtr& channel, mChannels)
	{
		Tp::Features feats;
		feats.insert( Tp::Channel::FeatureCore );

		bool ret = (*mWaiter)( channel->becomeReady( feats ) );
		if ( ret == true )
		{
			qDebug() << "Channel successfully became ready.";
			connect(channel.data(),
				SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
				SIGNAL(channelClosed()));
			bool isRequested = channel->isRequested();
			if ( channel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT ) == 0 )
			{
				qDebug() << "Text channel received.";

			}
			else if ( channel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ) == 0 )
			{
				qDebug() << "Streamed media channel received.";
			}
			if ( channel->isRequested() == true )
			{
				qDebug() << "Channel is requested!";
			}
			else
			{
				qDebug() << "Channel is not requested!";
				emit incomingChannelReceived( channel->channelType(), channel );
			}
		}
		else
		{
			qWarning() << "Channel failed to become ready!";
		}
	}
	context->setFinished();
	QTimer::singleShot(0, this, SIGNAL(handleChannelsFinished()));
	MWTS_LEAVE;
}


