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
#include "TelepathyTest.h"
#include "Call.h"
#include "TextChat.h"
#include "ClientHandler.h"
#include "PendingOperationWaiter.h"
#include "Contacts.h"

/**************** Class TelepathyTest ********************/


const int TEST_TIMEOUT = 180000;


TelepathyTest::TelepathyTest( bool isCli, QObject* pParent )
: MwtsTest( pParent ), mIsCli( isCli )
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

TelepathyTest::~TelepathyTest()
{
}

void TelepathyTest::OnInitialize()
{
	MWTS_ENTER;

	// Enable debbugging and traces from mwts-common
	g_pLog->EnableDebug( true );
	g_pLog->EnableTrace( true );

	qDebug() << "Initializing internal variables...";
	
	mWaiter = PendingOperationWaiter::instance();
	mEventLoop = new QEventLoop;
	mTimer = new QTimer;
	connect( mTimer, SIGNAL(timeout()), SLOT(testTimeout()));
	mLatencyMeasure = new QTime;
	mOutgoingCallMeasure = new QTime;
	mIncomingCallMeasure = new QTime;
	
	// Initialize TelepathyQt, and enable debugging
	qDebug() << "Initializing Telepathy-Qt4 with debug & warnings...";
	Tp::registerTypes();
	Tp::enableDebug( true );
	Tp::enableWarnings( true );
	
	// Register our channel handler
	qDebug() << "Creating channel handlers...";

	mClientRegistrar = Tp::ClientRegistrar::create();
	Tp::ChannelClassList clientFilters;
	Tp::ChannelClassList text;
	Tp::ChannelClassList streamedMedia;

	Tp::ChannelClassList filters;
	// match filters with the ones defined in CallUi.client file:
	//  [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 2]
	//  org.freedesktop.Telepathy.Channel.ChannelType s=org.freedesktop.Telepathy.Channel.Type.StreamedMedia
	//  org.freedesktop.Telepathy.Channel.TargetHandleType u=1
	//
	//  [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 3]
	//  org.freedesktop.Telepathy.Channel.ChannelType s=org.freedesktop.Telepathy.Channel.Type.StreamedMedia
	//  org.freedesktop.Telepathy.Channel.TargetHandleType u=1
	//  org.freedesktop.Telepathy.Channel.Type.StreamedMedia.InitialAudio b=true
	//
	//  [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 4]
	//  org.freedesktop.Telepathy.Channel.ChannelType s=org.freedesktop.Telepathy.Channel.Type.StreamedMedia
	//  org.freedesktop.Telepathy.Channel.TargetHandleType u=1
	//  org.freedesktop.Telepathy.Channel.Type.StreamedMedia.InitialVideo b=true

	//QMap<QString, QDBusVariant> streamedMediaFilters;
	{ // [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 2]
		Tp::ChannelClass filter;
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
			QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
			QDBusVariant((uint) Tp::HandleTypeContact) );
		streamedMedia.append( filter );
	}

	{ // [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 3]
		Tp::ChannelClass filter;
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
			QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
			QDBusVariant((uint) Tp::HandleTypeContact) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ".InitialAudio"),
			QDBusVariant(true) );
		streamedMedia.append( filter );
	}

	{ // [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 4]
		Tp::ChannelClass filter;
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
			QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
			QDBusVariant((uint) Tp::HandleTypeContact) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ".InitialVideo"),
			QDBusVariant(true) );
		streamedMedia.append( filter );
	}

	// Have all the caps defined in http://telepathy.freedesktop.org/spec/org.freedesktop.Telepathy.Channel.Interface.MediaSignalling.html#org.freedesktop.Telepathy.Channel.Interface.MediaSignalling/gtalk-p2p
	// (no need for specific codec capabilities)
	QStringList clientCapabilities;
	clientCapabilities
	    << QLatin1String(TELEPATHY_INTERFACE_CHANNEL_INTERFACE_MEDIA_SIGNALLING "/gtalk-p2p")
	    << QLatin1String(TELEPATHY_INTERFACE_CHANNEL_INTERFACE_MEDIA_SIGNALLING "/ice-udp")
	    << QLatin1String(TELEPATHY_INTERFACE_CHANNEL_INTERFACE_MEDIA_SIGNALLING "/wlm-8.5");

	// [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 0]
	// org.freedesktop.Telepathy.Channel.ChannelType s=org.freedesktop.Telepathy.Channel.Type.Text
	// org.freedesktop.Telepathy.Channel.TargetHandleType u=1
	//
	// [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 1]
	// org.freedesktop.Telepathy.Channel.ChannelType s=org.freedesktop.Telepathy.Channel.Type.Text
	// org.freedesktop.Telepathy.Channel.TargetHandleType u=2

	{ // [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 0]
		Tp::ChannelClass filter;
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
			QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
			QDBusVariant((uint) Tp::HandleTypeContact) );
		text.append( filter );
	}

	{ // [org.freedesktop.Telepathy.Client.Handler.HandlerChannelFilter 1]
		Tp::ChannelClass filter;
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
			QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT) );
		filter.insert(
			QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
			QDBusVariant((uint) Tp::HandleTypeRoom) );
		text.append( filter );
	}

	clientFilters.append( text );
	clientFilters.append( streamedMedia );
	
	mChannelHandler = new ChannelHandlerClient( clientFilters, clientCapabilities, true );
	
	qDebug() << "Registering CommonClientHandler...";
	Tp::AbstractClientPtr client = Tp::AbstractClientPtr::dynamicCast(Tp::SharedPtr<ChannelHandlerClient>( mChannelHandler ));
	if ( !mClientRegistrar->registerClient( client, "CommonClientHandler" ) )
	{
		MWTS_ERROR( "Failed to register client." );
	}

	connect( mChannelHandler,
			 SIGNAL(incomingChannelReceived(const QString&,const Tp::ChannelPtr&)),
			 SLOT(onIncomingChannelReceived(const QString&,const Tp::ChannelPtr&)) );

	// Wait a moment, so that channel handler can be set up
	mTimer->start( 500 );
	mEventLoop->exec();
	
	// Create account manager
	qDebug() << "Opening account manager...";
	mAccountMgr = Tp::AccountManager::create( QDBusConnection::sessionBus() );
	
	// Synchronize the returned ReadyObject, and wait untile Account Manger is ready
	// Execution will not continue, until becomeReady() has finished
	bool ret = (*mWaiter)( mAccountMgr->becomeReady() );
	if ( ret == true )
	{
		// AccountManager is now ready
		qDebug() << "Bus name: " << mAccountMgr->busName();
		qDebug() << "Object path: " << mAccountMgr->objectPath();
		qDebug() << "Valid accounts:";
		foreach ( const QString& path, mAccountMgr->validAccountPaths() )
		{
			qDebug() << " path: " << path;
		}
		QList<Tp::AccountPtr> validAccounts = mAccountMgr->validAccounts();
		foreach( Tp::AccountPtr account, validAccounts )
		{
			qDebug() << "Account protocol: " << account->protocol();
			if ( account->protocol().compare( "jabber" ) == 0 ||
				 account->protocol().compare( "sip" ) == 0 )
			{
				bool ret = (*mWaiter)( account->remove() );
				if ( ret == true )
				{
					// AccountManager is now ready
					qDebug() << "Successfully removed account.";
				}
				else
				{
					qWarning() << "Failed to remove account!";
					qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
				}
			}
		}
	}
	else
	{
		qCritical() << "Failed to create account manager!";
		qCritical() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	
	mConferenceCall = Tp::StreamedMediaChannelPtr();



	qDebug() << "Configuration test:  ";
	qDebug() << "Reading from file %1" << g_pConfig->fileName();

	qDebug() << "All keys available in configuration file: ";
	foreach( QString keyName, g_pConfig->allKeys() )
	{
		qDebug() << "  " << keyName << " == "
				 << g_pConfig->value( keyName, QString( "<error>" ) ).toString();
	}

	MWTS_LEAVE;
}


void TelepathyTest::OnUninitialize()
{
	MWTS_ENTER;

	if ( mTimer->isActive() )
	{
		qDebug() << "Timer active, stopping it...";
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running, stopping it...";
		mEventLoop->exit();
	}

	qDebug() << "Clearing all contacts...";
	mContacts.clear();

	if ( mTimer->isActive() )
	{
		qDebug() << "Timer active, stopping it...";
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running, stopping it...";
		mEventLoop->exit();
	}
	
	qDebug() << "Unregistering channel handler...";
	Tp::AbstractClientPtr client = Tp::AbstractClientPtr::dynamicCast(Tp::SharedPtr<ChannelHandlerClient>( mChannelHandler ));
	if ( !mClientRegistrar->unregisterClient( client ) )
	{
		MWTS_ERROR( "Failed to register client." );
	}
	
	qDebug() << "Killing timer...";
	if ( mTimer )
	{
		if ( mTimer->isActive() )
		{
			qDebug() << "Timer active, stopping it...";
			mTimer->stop();
		}
		qDebug() << "Deleting timer...";
		delete mTimer;
		mTimer = NULL;
	}
	
	qDebug() << "Killing event loop...";
	if ( mEventLoop )
	{
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running, stoping it...";
			mEventLoop->exit();
		}
		qDebug() << "Deleting event loop...";
		delete mEventLoop;
		mEventLoop = NULL;
	}
	
	if ( mLatencyMeasure )
	{
		qDebug() << "Deleting latency measurement...";
		delete mLatencyMeasure;
		mLatencyMeasure = NULL;
	}
	
	if ( mOutgoingCallMeasure )
	{
		qDebug() << "Deleting outgoing call latency measurement...";
		delete mOutgoingCallMeasure;
		mOutgoingCallMeasure = NULL;
	}
	
	if ( mIncomingCallMeasure )
	{
		qDebug() << "Deleting incoming call latency measurement...";
		delete mIncomingCallMeasure;
		mIncomingCallMeasure = NULL;
	}
	
	mConferenceCall.reset();
	if ( mCalls.size() > 0 )
	{
		qDebug() << "Deleting " << mCalls.size() << " calls...";
		QList<Call*>::iterator i;
		for ( i = mCalls.begin(); i != mCalls.end(); ++i )
		{
			Call* c = *i;
			if ( c->isInvalidated() == true )
			{
				qDebug() << "Not closing call that is already invalidated!";
			}
			else
			{
				c->Close();
			}
			delete c;
		}
		mCalls.clear();
	}

	qDebug() << "Deleting account...";
	mAccount.reset();

	qDebug() << "Deleting account manager...";
	mAccountMgr.reset();
	disconnect();
	if ( mWaiter )
	{
		qDebug() << "Deleting PenfingOperationWaiter...";
		delete mWaiter;
		mWaiter = NULL;
	}
	
	MWTS_LEAVE;
}


QString TelepathyTest::CaseName()
{
	MWTS_ENTER;
	if ( mIsCli == true )
	{
		qDebug() << "Returning mwts-telepathy-cli as case name.";
		MWTS_LEAVE;
		return QString( "mwts-telepathy-cli" );
	}
	QString s = MwtsTest::CaseName();
	qDebug() << "Returning " << s << " as case name.";
	MWTS_LEAVE;
	return s;
}


void TelepathyTest::StartLatencyMeasuring()
{
	MWTS_ENTER;
	mLatencyMeasure->start();
	MWTS_LEAVE;
}


int TelepathyTest::NextLatencyMeasure()
{
	MWTS_ENTER;
	int elapsed = mLatencyMeasure->restart();
	qDebug() << "Time elapsed since last measure: " << elapsed << " msec";
	MWTS_LEAVE;
	return elapsed;
}


int TelepathyTest::ReportOutgoingCallLatency()
{
	MWTS_ENTER;
	qDebug() << "Outgoing call latency: " << mOutgoingCallLatency << " msec";
	if ( g_pResult )
	{
		g_pResult->AddMeasure( QString( "Outgoing call latency" ), mOutgoingCallLatency, QString( "msec" ) );
	}
	MWTS_LEAVE;
	return mOutgoingCallLatency;
}


int TelepathyTest::ReportIncomingCallLatency()
{
	MWTS_ENTER;
	qDebug() << "Incoming call latency: " << mIncomingCallLatency << " msec";
	if ( g_pResult )
	{
		g_pResult->AddMeasure( QString( "Incoming call latency" ), mIncomingCallLatency, QString( "msec" ) );
	}
	MWTS_LEAVE;
	return mIncomingCallLatency;
}


// Create an account from given configuration
bool TelepathyTest::CreateAccountFromConf( const QString& conf )
{
	MWTS_ENTER;
	bool ret = false;
	
	MWTS_DEBUG( QString( "Reading from file %1" ).arg( g_pConfig->fileName() ) );
	MWTS_DEBUG( QString( "Reading from group %1" ).arg( conf ) );
	
	// Read the configuration
	g_pConfig->beginGroup( conf );
	QString account = g_pConfig->value( QString( "account" ), QString( "" ) ).toString();
	QString password = g_pConfig->value( QString( "password" ), QString( "" ) ).toString();
	QString protocol = g_pConfig->value( QString( "protocol" ), QString( "" ) ).toString();
	QString connMgr = g_pConfig->value( QString( "connmgr" ), QString( "" ) ).toString();
	QString transport = g_pConfig->value( QString( "transport" ), QString( "" ) ).toString();
	g_pConfig->endGroup();
	
	qDebug() << "Account: " << account 
			<< "Password: " << password
			<< "Protocol: " << protocol
			<< "Connection manager: " << connMgr
			<< "Transport: " << transport;
	
	QVariantMap parameters;
	parameters.insert( QString( "account" ), account );
	parameters.insert( QString( "password" ), password );
	if ( transport.length() > 0 )
	{
		parameters.insert( QString( "transport" ), transport );
	}

	// Synchronize the returned PendingOperation, and wait untile Account Manger is ready
	// Execution will not continue, until createAccount() has finished
	//ret = (*mWaiter)( mAccountMgr->createAccount( connMgr, protocol, account, parameters ) );
	Tp::PendingAccount* pa = mAccountMgr->createAccount( connMgr, protocol, account, parameters );

	ret = mWaiter->createAccount( pa, mAccount );

	if ( ret == true )
	{
		qDebug() << "Account created successfully.";
		qDebug() << "Is valid account? " << mAccount->isValidAccount();
		connect( mAccount.data(), SIGNAL(stateChanged(bool)),
			 SLOT(onStateChanged(bool)) );
		connect( mAccount.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)),
			 SLOT(onConnectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)) );
		connect( mAccount.data(), SIGNAL(haveConnectionChanged(bool)),
			 SLOT(onHaveConnectionChanged(bool)) );

		// Synchronize the call to accounts becomeReady
		// Execution will not continue, until becomeReady is finished
		Tp::Features feats;
		feats.insert( Tp::Account::FeatureCore );
		feats.insert( Tp::Account::FeatureAvatar );
		feats.insert( Tp::Account::FeatureProtocolInfo );
		bool ret = (*mWaiter)( mAccount->becomeReady( feats ) );
		if ( ret == true )
		{
			qDebug() << "Account successfully became ready.";
		}
		else
		{
			qWarning() << "Account failed to become ready!";
			qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
			MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
		}
	}
	else
	{
		qWarning() << "Failed to create account!";
		MWTS_ERROR("Failed to create account!");
	}
	MWTS_LEAVE;
	return ret;
}


void TelepathyTest::onPendingAccountFinished( Tp::PendingOperation* operation )
{

}


bool TelepathyTest::CreateAccountFromPath( const QString& path )
{
	MWTS_ENTER;
	bool ret = false;
	
	// Validate given account path
	ret = isValidAccountPath( path );
	if ( ret == true )
	{
		mAccount = mAccountMgr->accountForPath( path );
		// Synchronize the returned ReadyObject, and wait untile Account Manger is ready
		// Execution will not continue, until becomeReady() has finished
		Tp::Features feats;
		feats.insert( Tp::Account::FeatureCore );
		feats.insert( Tp::Account::FeatureAvatar );
		feats.insert( Tp::Account::FeatureProtocolInfo );
		ret = (*mWaiter)( mAccount->becomeReady( feats ) );
		if ( ret == true )
		{
			// AccountManager is now ready
			qDebug() << "Account successfully created from path: " << path;
			
			mAccountState = mAccount->isEnabled();
			mHaveConnection = mAccount->haveConnection();
			mConnectionStatus = mAccount->connectionStatus();
			mConnectionStatusReason = mAccount->connectionStatusReason();
			
			connect( mAccount.data(), SIGNAL(stateChanged(bool)),
				this, SLOT(onStateChanged(bool)) );
			connect( mAccount.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)),
				this, SLOT(onConnectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)) );
			connect( mAccount.data(), SIGNAL(haveConnectionChanged(bool)),
				this, SLOT(onHaveConnectionChanged(bool)) );
		}
		else
		{
			qWarning() << "Failed to create account!";
			qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
			MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
		}
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::SetEnabled( bool value )
{
	MWTS_ENTER;
	if ( mAccount.isNull() )
	{
		qWarning() << "No account!";
		MWTS_LEAVE;
		return false;
	}
	// Synchronize call to setEnabled
	// Execution will not continue, until setEnables has finished
	bool ret = (*mWaiter)( mAccount->setEnabled( value ) );
	if ( ret == true )
	{
		qDebug() << "Successfully changed account state state to " << ( value ? "enabled" : "disabled" );
	}
	else
	{
		qWarning() << "Failed to change account state!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::SetRequestedPresence( const Tp::SimplePresence& value )
{
	MWTS_ENTER;
	if ( mAccount.isNull() )
	{
		qWarning() << "No account!";
		MWTS_LEAVE;
		return false;
	}
	// Synchronize call to setRequestedPresence
	// Execution will not continue, until setRequestedPresence has finished
	bool ret = (*mWaiter)( mAccount->setRequestedPresence( value ) );
	if ( ret == true )
	{
		qDebug() << "Successfully changed account presence.";
	}
	else
	{
		qWarning() << "Failed to change account presence!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return ret;
}


// WaitForAccountConnected
// This function stops to wait, until account has signaled either Connected or Disconnected
bool TelepathyTest::WaitForAccountConnected()
{
	MWTS_ENTER;
	mRetValue = false;
	if ( mConnectionStatus == Tp::ConnectionStatusConnected )
	{
		qDebug() << "Account is already connected.";
		mRetValue = true;
		// Let's see, if we can set up contact management now
		if ( mHaveConnection )
		{
			mContactManagement = new ContactManagement( mAccount->connection() );
		}
	}
	else
	{
		qDebug() << "Waiting for account to get connected...";
		if ( mEventLoop->isRunning() )
		{
			qWarning() << "Event loop already running!";
			MWTS_LEAVE;
			return false;
		}
		// Set up timeout, to stop internal event loop
		mTimer->start( TEST_TIMEOUT );
		// Start waiting
		mEventLoop->exec();
		
		// Let's see, if we can set up contact management now
		if ( mHaveConnection )
		{
			mContactManagement = new ContactManagement( mAccount->connection() );
		}
	}
	MWTS_LEAVE;
	return mRetValue;
}


bool TelepathyTest::ContactsForIdentifiers( const QStringList& identifiers )
{
	MWTS_ENTER;
	mContacts.clear();
	bool ret = false;
	if ( mContactManagement )
	{
		QStringList contactNames;

		foreach ( QString confName, identifiers )
		{
			MWTS_DEBUG( QString( "Reading from file %1" ).arg( g_pConfig->fileName() ) );
			MWTS_DEBUG( QString( "Reading from group %1" ).arg( confName ) );

			// Read the actual contact name from configuration
			QString contactName =
					g_pConfig->value( QString( confName ), QString( "" ) ).toString();
			MWTS_DEBUG( QString( "Contact name: %1" ).arg( contactName ) );

			if ( contactName.length() > 0 )
			{
				contactNames.append( contactName );
			}
		}

		ret = mContactManagement->contactsForIdentifiers( contactNames, mContacts );
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::RequestContacts( const QStringList& names )
{
	MWTS_ENTER;
	mContacts.clear();
	bool ret = false;
	if ( mHaveConnection )
	{
		Tp::ConnectionPtr conn = mAccount->connection();
		
		if ( conn->isReady() == false )
		{
			// Make sure that connection has FeatureRoster
			// Execution will not continue, until becomeReady has finished
			Tp::Features feats;
			feats.insert( Tp::Connection::FeatureCore );
			feats.insert( Tp::Connection::FeatureSelfContact );
			feats.insert( Tp::Connection::FeatureSimplePresence );
			feats.insert( Tp::Connection::FeatureRoster );
			feats.insert( Tp::Connection::FeatureRosterGroups );
			if ( (*mWaiter)( conn->becomeReady( feats ) ) == false )
			{
				qWarning() << mWaiter->lastErrorName()
					<< ": " << mWaiter->lastErrorMessage();
				MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
				MWTS_LEAVE;
				return false;
			}
		}
		
		Tp::UIntList handles;
		Tp::PendingHandles* ph = conn->requestHandles( Tp::HandleTypeContact, names );
		// Synchronize PendingOperation returned for handle request.
		// Execution will not continue, until requestHandles has returned
		ret = mWaiter->requestHandles( ph, handles );
		if ( ret == false )
		{
			qWarning() << "Failed to request handles!";
			qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
			MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
			MWTS_LEAVE;
			return ret;
		}
		else
		{
			qDebug() << "Handles received:";
			foreach ( uint handle, handles )
			{
				qDebug() << "  " << handle;
			}
		}
		
		Tp::ContactManager* contactManager = conn->contactManager();
		if ( contactManager )
		{
			QList<Tp::ContactPtr> contacts;
			// Synchronize PendingOperation returned for handle to contacts request.
			// Execution will not continue, until handlesToContacts has returned
			ret = mWaiter->handlesToContacts( contactManager->contactsForHandles( handles ), contacts );
			if ( ret == false )
			{
				qWarning() << "Failed to convert handles to contacts!";
				qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
				MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
				MWTS_LEAVE;
				return ret;
			}
			qDebug() << "Contacts received:";
			foreach ( Tp::ContactPtr contact, contacts )
			{
				if ( contact.isNull() == true )
				{
					qDebug() << "Contact is NULL!!";
				}
				else
				{
					qDebug() << "  " << contact->id();
				}
			}
			mContacts = Tp::Contacts::fromList( contacts );
		}
		else
		{
			qWarning() << "No contact manager!";
			MWTS_ERROR("No contact manager!");
			MWTS_LEAVE;
			return false;
		}
	}
	else
	{
		qWarning() <<"Account does not have connection!";
		MWTS_ERROR("Account does not have connection!");
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::GetContacts( const QStringList& names )
{
	MWTS_ENTER;
	mContacts.clear();
	bool ret = false;
	if ( mHaveConnection )
	{
		Tp::ConnectionPtr conn = mAccount->connection();
		
		if ( conn->isReady() == false )
		{
			// Make sure that connection has FeatureRoster
			// Execution will not continue, until becomeReady has finished
			Tp::Features feats;
			feats.insert( Tp::Connection::FeatureCore );
			feats.insert( Tp::Connection::FeatureSelfContact );
			feats.insert( Tp::Connection::FeatureSimplePresence );
			feats.insert( Tp::Connection::FeatureRoster );
			feats.insert( Tp::Connection::FeatureRosterGroups );
			if ( (*mWaiter)( conn->becomeReady( feats ) ) == false )
			{
				qWarning() << mWaiter->lastErrorName()
					<< ": " << mWaiter->lastErrorMessage();
				MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
				MWTS_LEAVE;
				return false;
			}
		}
		
		Tp::UIntList handles;
		Tp::PendingHandles* ph = conn->requestHandles( Tp::HandleTypeContact, names );
		// Synchronize PendingOperation returned for handle request.
		// Execution will not continue, until requestHandles has returned
		ret = mWaiter->requestHandles( ph, handles );
		if ( ret == false )
		{
			qWarning() << "Failed to request handles!";
			qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
			MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
			MWTS_LEAVE;
			return ret;
		}
		
		Tp::ContactManager* contactManager = conn->contactManager();
		if ( contactManager )
		{
			// Look up given handles from "buddy list"
			QList<Tp::ContactPtr> contacts;
			foreach ( uint handle, handles )
			{
				Tp::ContactPtr contact = contactManager->lookupContactByHandle( handle );
				mContacts.insert( contact );
				qDebug() << "Contact received: " << contact->id();
			}
		}
		else
		{
			qWarning() << "No contact manager!";
			MWTS_ERROR("No contact manager!");
			MWTS_LEAVE;
			return false;
		}
	}
	else
	{
		qWarning() <<"Account does not have connection!";
		MWTS_ERROR("Account does not have connection!");
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::EnsureMediaCall(	const QString& contactIdentifier,
					const QDateTime& userActionTime,
					const QString& preferredHandler )
{
	MWTS_ENTER;
	if ( mAccount.isNull() )
	{
		qWarning() << "No account!";
		MWTS_ERROR("No account!");
		MWTS_LEAVE;
		return false;
	}
	
	if ( mHaveConnection )
	{
		// Connect to NewChannels from TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS
		// This way we will be notified for new channels
		Tp::ConnectionPtr conn = mAccount->connection();
		if ( conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
		{
			connect(conn->requestsInterface(),
				SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
				SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
		}
	}
	else
	{
		qDebug() << "Account has no connection. Unable to connect to NewChannels.";
	}
	
	MWTS_DEBUG( QString( "Reading from file %1" ).arg( g_pConfig->fileName() ) );
	MWTS_DEBUG( QString( "Reading from group %1" ).arg( contactIdentifier ) );

	// Read the actual contact name from configuration
	QString contactName =
			g_pConfig->value( QString( contactIdentifier ), QString( "" ) ).toString();
	MWTS_DEBUG( QString( "Contact name: %1" ).arg( contactName ) );

	// Synchronize the call to ensureMediaCall
	// Execution will not continue, until ensureMediaCall has finished.
	// During that time, our ChannelHandler will be offered the newly requested channel.
	mOutgoingCallMeasure->start();
        bool ret = (*mWaiter)( mAccount->ensureMediaCall( contactName,
							  userActionTime,
							  preferredHandler ) );
	if ( ret == true )
	{
		if ( mChannelHandler->mChannels.size() > 0 )
		{
			// AccountManager is now ready
			qDebug() << "Succesfully requested media call.";
			Tp::StreamedMediaChannelPtr chan =
				Tp::StreamedMediaChannelPtr( qobject_cast<Tp::StreamedMediaChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
			mCall = new Call( chan );
			mChannelHandler->mChannels.removeAt( 0 );
			mCalls.append( mCall );
		}
	}
	else
	{
		qWarning() << "Failed to request media call!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::RemoveAccount()
{
	MWTS_ENTER;
	if ( mAccount.isNull() )
	{
		qWarning() << "No account!";
		MWTS_LEAVE;
		return false;
	}
	// Synchronize the call to remove
	// Execution will not continue, untill remove has finished
	bool ret = (*mWaiter)( mAccount->remove() );
	if ( ret == true )
	{
		// AccountManager is now ready
		qDebug() << "Successfully removed account.";
	}
	else
	{
		qWarning() << "Failed to remove account!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return ret;
}



bool TelepathyTest::ActivateCall()
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		if ( mContacts.size() > 0 )
		{
			Tp::Contacts::iterator i;
			i = mContacts.begin();
			qDebug() << "Calling to " << (*i)->id();
			
			// Ask the call object to activate outgoing call
			ret = mCall->OpenAudioStream( (*i) );
			mOutgoingCallLatency = mOutgoingCallMeasure->elapsed();
		}
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::EndCall()
{
	MWTS_ENTER;
	bool endCall = true;
	if ( mCalls.size() > 0 )
	{
		qDebug() << "Ending " << mCalls.size() << " calls!";
		QList<Call*>::iterator i;
		for ( i = mCalls.begin(); i != mCalls.end(); ++i )
		{
			Call* c = *i;
			qDebug() << c;
			bool ret = c->Close();
			// Only change the return value if any of the calls fails to end.
			if ( ret == false )
			{
				endCall = false;
			}
			delete c;
		}
		mCalls.clear();
		mCall = NULL;
	}
	else
	{
		qDebug() << "No calls to end...";
		endCall = false;
	}

	MWTS_LEAVE;
	return endCall;
}


bool TelepathyTest::WaitForCallEnded()
{
	MWTS_ENTER;
	mRetValue = false;
	qDebug() << "Waiting for call to end...";

	if ( mEventLoop->isRunning() )
	{
		qCritical() << "Event loop already running!";
		MWTS_LEAVE;
		return false;
	}

	connect( mCall,
			 SIGNAL(callEnded(const QString&,const QString&)),
			 SLOT(onCallEnded(const QString&,const QString&)) );

	mTimer->start( TEST_TIMEOUT );
	mEventLoop->exec();

	disconnect( mCall,
				SIGNAL(callEnded(const QString&,const QString&)),
				this,
				SLOT(onCallEnded(const QString&,const QString&)) );

	MWTS_LEAVE;
	return mRetValue;
}


bool TelepathyTest::WaitForIncomingCall()
{
	MWTS_ENTER;
	mRetValue = false;
	qDebug() << "Waiting for incoming call...";
	
    if ( mHaveConnection )
	{
		// Connect to NewChannels from TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS
		// This way we will be notified for new channels
		Tp::ConnectionPtr conn = mAccount->connection();
		if ( conn.data() && conn->isValid() &&conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
		{
			qDebug() << "Connecting to NewChannels";
			connect(conn->requestsInterface(),
				SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
				SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
		}
	}
	else
	{
		qDebug() << "Account has no connection. Unable to connect to NewChannels.";
    }
	qDebug() << "Connected to NewChannels";
	
	if ( mEventLoop->isRunning() )
	{
		qWarning() << "Event loop already running!";
		MWTS_LEAVE;
		return false;
	}
	
	// Set up our internal waiting loop to wait for the incoming channel
	// Execution will not continue until incoming channel or timeout
	qDebug() << "Starting timer";
	mTimer->start( TEST_TIMEOUT );
	qDebug() << "Starting eventloop";
	qDebug() << mEventLoop;
	mEventLoop->exec();

	qDebug() << "Event loop done.";
	
	if ( mChannelHandler->mChannels.size() > 0 )
	{
		// We have new channel, create a call object
		Tp::StreamedMediaChannelPtr chan =
			Tp::StreamedMediaChannelPtr( qobject_cast<Tp::StreamedMediaChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
		mCall = new Call( chan );
		mChannelHandler->mChannels.removeAt( 0 );
		mCalls.append( mCall );
	}
	else
	{
		mCall = NULL;
	}
	
        if ( mHaveConnection )
        {
		qDebug() << "Disconnecting from NewChannels...";
		// Connect to NewChannels from TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS
		// This way we will be notified for new channels
		Tp::ConnectionPtr conn = mAccount->connection();
		if ( conn.data() && conn->isValid() &&conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
		{
			qDebug() << "Disconnecting from NewChannels";
			disconnect(conn->requestsInterface(),
				SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
				this,
				SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
		}
	}

        MWTS_LEAVE;
	return mRetValue;
}


bool TelepathyTest::CreateConferenceCall()
{
	MWTS_ENTER;
	mRetValue = false;

     if ( mConferenceCall.isNull() == false )
     {
    	 mConferenceCall.reset();
     }

	qDebug() << "Creating confernce call...";


	QList<Tp::ChannelPtr> chans;

	qDebug() << "Number of calls: " << mCalls.size();

	foreach ( Call* call, mCalls )
	{
		Tp::StreamedMediaChannelPtr smchan = call->Channel();
		Tp::ChannelPtr chan = Tp::ChannelPtr( qobject_cast<Tp::Channel*>( smchan.data() ) );
		chans.append( chan );
	}


	Tp::PendingChannelRequest* pcr =
			mAccount->createConferenceMediaCall( chans,
				QStringList(), QDateTime::currentDateTime(),
				"org.freedesktop.Telepathy.Client.CommonClientHandler" );
	mRetValue = (*mWaiter)( pcr );
	if ( mRetValue == true )
	{
		qDebug() << "Successfully initialised conference call!";
		if ( mChannelHandler->mChannels.size() > 0 )
		{
			// We have new channel, create a call object
			mConferenceCall =
				Tp::StreamedMediaChannelPtr( qobject_cast<Tp::StreamedMediaChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
			mChannelHandler->mChannels.removeAt( 0 );
		}
	}
	else
	{
		qDebug() << "Failed to initialise conference call!";
	}

	MWTS_LEAVE;
	return mRetValue;
}


bool TelepathyTest::AcceptCall()
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		// Ask the call object to accept the incoming call
		ret = mCall->AcceptCall();
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::HoldCall()
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		// Ask the call object to hold the call
		ret = mCall->Hold();
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::UnholdCall()
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		// Ask the call object to unhold the call
		ret = mCall->Unhold();
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::SendDTMFTone( Tp::DTMFEvent event )
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		// Ask the call object to send a DTMF tone
		ret = mCall->SendDTMFTone( event );
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::Wait( int msec )
{
	MWTS_ENTER;
	// Use the instance variable for this, because all the error handling
	// set this variable to false.
	mRetValue = true;
	qDebug() << mTimer;
	qDebug() << mEventLoop;

	qDebug() << "Is timer active? " << mTimer->isActive();
	if ( mTimer->isActive() )
	{
		qDebug() << "Timer active, closing...";
		mTimer->stop();
	}

	qDebug() << "Is eventloop running? " << mEventLoop->isRunning();
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop is running, closing...";
		mEventLoop->exit();
	}

	qDebug () << "Starting again...";
	mTimer->start( msec );
	mEventLoop->exec();

	MWTS_LEAVE;
	// It is safe to return true, since there is no way this function could fail
	return true;
}


bool TelepathyTest::StayOnLine( int msec )
{
	MWTS_ENTER;
	bool ret = false;
	if ( mCall )
	{
		// Ask the call to stay on the line
		ret = mCall->StayOnLine( msec );
	}
	MWTS_LEAVE;
	return ret;
}


bool TelepathyTest::isValidAccountPath( const QString& path )
{
	MWTS_ENTER;
	qDebug() << "Checking validity of " << path;
	// Check that the given path is in account managers list of valid account paths
	foreach ( const QString& validpath, mAccountMgr->validAccountPaths() )
	{
		if ( validpath.compare( path ) == 0 )
		{
			qDebug() << "Found " << path << " from valid paths.";
			MWTS_LEAVE;
			return true;
		}
	}
	qDebug() << "Did not find " << path << " from valid paths.";
	MWTS_LEAVE;
	return false;
}


void TelepathyTest::onCallEnded( const QString& errorName, const QString& errorMessage )
{
	MWTS_ENTER;
	qDebug() << "Call ended!\n" << errorName << ": " << errorMessage;
	if ( mTimer->isActive() )
	{
		mTimer->stop();
	}
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = true;
		mEventLoop->exit();
	}
	MWTS_LEAVE;
}


void TelepathyTest::testTimeout()
{
	MWTS_ENTER;
	// If timeout occurs, stop the loop and set return value to false
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mTimer->stop();
		mEventLoop->exit();
	}
	MWTS_LEAVE;
}


void TelepathyTest::OnIdle()
{
}


void TelepathyTest::onAccountCreated( const QString& path )
{
	MWTS_ENTER;
	qDebug() << "New account created to path: " << path;
	// Account is created, so open it
	mAccount = mAccountMgr->accountForPath( path );
	if ( mAccount.isNull() )
	{
		qWarning() << "Failed to create account from path: " << path;
		MWTS_LEAVE;
		return;
	}
	
	connect( mAccount.data(), SIGNAL(stateChanged(bool)),
		 SLOT(onStateChanged(bool)) );
	connect( mAccount.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)),
		 SLOT(onConnectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)) );
	connect( mAccount.data(), SIGNAL(haveConnectionChanged(bool)),
		 SLOT(onHaveConnectionChanged(bool)) );
	
	// Synchronize the call to accounts becomeReady
	// Execution will not continue, until becomeReady is finished
	Tp::Features feats;
	feats.insert( Tp::Account::FeatureCore );
	feats.insert( Tp::Account::FeatureAvatar );
	feats.insert( Tp::Account::FeatureProtocolInfo );
	bool ret = (*mWaiter)( mAccount->becomeReady( feats ) );
	if ( ret == true )
	{
		qDebug() << "Account successfully became ready.";
	}
	else
	{
		qWarning() << "Account failed to become ready!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
}


void TelepathyTest::onAccountRemoved( const QString& path )
{
	MWTS_ENTER;
	qDebug() << "Account in path " << path << " was removed.";
	MWTS_LEAVE;
}


void TelepathyTest::onAccountValidityChanged( const QString& path, bool valid )
{
	MWTS_ENTER;
	qDebug() << "Account: " << path << "\nValidity: " << ( valid ? "true" : "false" );
	MWTS_LEAVE;
}


void TelepathyTest::onNewAccount( const Tp::AccountPtr& account )
{
	MWTS_ENTER;
	MWTS_ENTER;
	qDebug() << "New account created with display name: " << account->displayName();
	mAccount = account;
	// Account is created, so open it
	if ( mAccount->isValidAccount() == false )
	{
		qCritical() << "Account is invalid!";
		MWTS_LEAVE;
		return;
	}

	connect( mAccount.data(), SIGNAL(stateChanged(bool)),
		 SLOT(onStateChanged(bool)) );
	connect( mAccount.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)),
		 SLOT(onConnectionStatusChanged(Tp::ConnectionStatus,Tp::ConnectionStatusReason)) );
	connect( mAccount.data(), SIGNAL(haveConnectionChanged(bool)),
		 SLOT(onHaveConnectionChanged(bool)) );

	// Synchronize the call to accounts becomeReady
	// Execution will not continue, until becomeReady is finished
	Tp::Features feats;
	feats.insert( Tp::Account::FeatureCore );
	feats.insert( Tp::Account::FeatureAvatar );
	feats.insert( Tp::Account::FeatureProtocolInfo );
	bool ret = (*mWaiter)( mAccount->becomeReady( feats ) );
	if ( ret == true )
	{
		qDebug() << "Account successfully became ready.";
	}
	else
	{
		qWarning() << "Account failed to become ready!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage();
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
}


void TelepathyTest::onStateChanged( bool state )
{
	MWTS_ENTER;
	mAccountState = state;
	qDebug() << "Account state: " << ( mAccountState ? "enabled" : "disabled" );
	MWTS_LEAVE;
}


void TelepathyTest::onConnectionStatusChanged( Tp::ConnectionStatus status, Tp::ConnectionStatusReason statusReason )
{
	MWTS_ENTER;
	mConnectionStatus = status;
	mConnectionStatusReason = statusReason;
	switch( mConnectionStatus )
	{
		case Tp::ConnectionStatusDisconnected:
			qDebug() << "Connection status: Disconnected";
			if ( mEventLoop->isRunning() )
			{
				qDebug() << "Event loop running! Stopping...";
				mRetValue = false;
				mTimer->stop();
				mEventLoop->exit();
				
			}
			break;
		case Tp::ConnectionStatusConnecting:
			qDebug() << "Connection status: Connecting";
			break;
		case Tp::ConnectionStatusConnected:
			qDebug() << "Connection status: Connected";
			if ( mEventLoop->isRunning() )
			{
				qDebug() << "Event loop running! Stopping...";
				mRetValue = true;
				mTimer->stop();
				mEventLoop->exit();
				
			}
			break;
	}
	switch( mConnectionStatusReason )
	{
		case Tp::ConnectionStatusReasonNoneSpecified:
			qDebug() << "Connection status reason: There is no reason set for this state change.";
			break;
		case Tp::ConnectionStatusReasonRequested:
			qDebug() << "Connection status reason: The change is in response to a user request.";
			break;
		case Tp::ConnectionStatusReasonNetworkError:
			qDebug() << "Connection status reason: There was an error sending or receiving on the network socket.";
			break;
		case Tp::ConnectionStatusReasonAuthenticationFailed:
			qDebug() << "Connection status reason: The username or password was invalid.";
			break;
		case Tp::ConnectionStatusReasonEncryptionError:
			qDebug() << "Connection status reason: There was an error negotiating SSL on this connection, or encryption was unavailable and require-encryption was set when the connection was created.";
			break;
		case Tp::ConnectionStatusReasonNameInUse:
			qDebug() << "Connection status reason: In general, this reason indicates that the requested account name or other identification could not be used due to conflict with another connection.";
			break;
		case Tp::ConnectionStatusReasonCertNotProvided:
			qDebug() << "Connection status reason: The server did not provide a SSL certificate.";
			break;
		case Tp::ConnectionStatusReasonCertUntrusted:
			qDebug() << "Connection status reason: The server's SSL certificate is signed by an untrusted certifying authority.";
			break;
		case Tp::ConnectionStatusReasonCertExpired:
			qDebug() << "Connection status reason: The server's SSL certificate has expired.";
			break;
		case Tp::ConnectionStatusReasonCertNotActivated:
			qDebug() << "Connection status reason: The server's SSL certificate is not yet valid.";
			break;
		case Tp::ConnectionStatusReasonCertHostnameMismatch:
			qDebug() << "Connection status reason: The server's SSL certificate did not match its hostname.";
			break;
		case Tp::ConnectionStatusReasonCertFingerprintMismatch:
			qDebug() << "Connection status reason: The server's SSL certificate does not have the expected fingerprint.";
			break;
		case Tp::ConnectionStatusReasonCertSelfSigned:
			qDebug() << "Connection status reason: The server's SSL certificate is self-signed.";
			break;
		case Tp::ConnectionStatusReasonCertOtherError:
			qDebug() << "Connection status reason: There was some other error validating the server's SSL certificate.";
			break;
	}
	MWTS_LEAVE;
}


void TelepathyTest::onHaveConnectionChanged( bool haveConnection )
{
	MWTS_ENTER;
	mHaveConnection = haveConnection;
	qDebug() << "Have connection? " << ( mHaveConnection ? "Yes." : "No." );
	MWTS_LEAVE;
}


void TelepathyTest::onNewChannels( const Tp::ChannelDetailsList& channels )
{
	MWTS_ENTER;
	mIncomingCallMeasure->start();
	foreach ( const Tp::ChannelDetails& details, channels )
	{
		QString channelType =
			details.properties.value( QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType" ) ).toString();
        QString targetId =
            details.properties.value( QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetID" ) ).toString();
        int targetHandle =
            details.properties.value( QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle" ) ).toInt();
        int targetHandleType =
            details.properties.value( QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType" ) ).toInt();
        bool requested =
			details.properties.value( QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".Requested" ) ).toBool();
		qDebug() << " channelType:" << channelType;
		qDebug() << " requested  :" << requested;
        qDebug() << " targetID :" << targetId;
        qDebug() << " targetHandle :" << targetHandle;
        qDebug() << " targetHanfleType :" << targetHandleType;
	}

	MWTS_LEAVE;
}


void TelepathyTest::onIncomingChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel )
{
	MWTS_ENTER;
	Q_UNUSED( channelType );
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ) == 0 )
	{
		mIncomingCallLatency = mIncomingCallMeasure->elapsed();
		qDebug() << "Received a streamed media channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
			
		}
	}
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT ) == 0 )
	{
		qDebug() << "Received a text channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
		}
	}
	MWTS_LEAVE;
}


void TelepathyTest::onIncomingStreamedMediaChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel )
{
	MWTS_ENTER;
	Q_UNUSED( channelType );
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ) == 0 )
	{
		mIncomingCallLatency = mIncomingCallMeasure->elapsed();
		qDebug() << "Received a streamed media channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
			
		}
	}
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT ) == 0 )
	{
		qDebug() << "Received a text channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
		}
	}
	MWTS_LEAVE;
}


void TelepathyTest::onIncomingTextChannelReceived(
			const QString& channelType,
			const Tp::ChannelPtr& aChannel )
{
	MWTS_ENTER;
	Q_UNUSED( channelType );
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA ) == 0 )
	{
		mIncomingCallLatency = mIncomingCallMeasure->elapsed();
		qDebug() << "Received a streamed media channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
			
		}
	}
	
	if ( aChannel->channelType().compare( TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT ) == 0 )
	{
		qDebug() << "Received a text channel.";
		if ( mEventLoop->isRunning() )
		{
			qDebug() << "Event loop running! Stopping...";
			mRetValue = true;
			mTimer->stop();
			mEventLoop->exit();
		}
	}
	MWTS_LEAVE;
}


bool TelepathyTest::RequestHandles( uint handleType, const QStringList& names )
{
	MWTS_ENTER;
	bool ret = false;

	if ( mAccount->haveConnection() )
	{
		Tp::ConnectionPtr conn = mAccount->connection();
		connect( conn->requestHandles( handleType, names ),
			 SIGNAL( finished( Tp::PendingOperation* ) ),
			 this,
			 SLOT( onRequestHandlesReady( Tp::PendingOperation* ) ) );
		ret = true;
	}
	else
	{
		MWTS_DEBUG("Account does not have connection!");
		ret = false;
	}
	MWTS_LEAVE;
	return ret;
}

/************ Text Message specific functionality *********************************/

/**
 * EnsureTextChat
 *
 * Gets text channel pointer from telepathy-ring created account.
 * Gives it to mwts-telepathy internal TextChat object
 * 
 * @param contactIdentifier number or email type identification where to connect or send message
 * @param userActionTime timestamp (current)
 * @param preferredHandler dbus service where our client is registered
 * @return bool success of the method
 */
bool TelepathyTest::EnsureTextChat(	const QString& contactIdentifier,
					const QDateTime& userActionTime,
					const QString& preferredHandler )
{
	MWTS_ENTER;
	if ( mAccount.isNull() )
	{
		qWarning() << "No account!";
		MWTS_LEAVE;
		return false;
	}

	MWTS_DEBUG( QString( "Reading from file %1" ).arg( g_pConfig->fileName() ) );
	MWTS_DEBUG( QString( "Reading from group %1" ).arg( contactIdentifier ) );

	// Read the actual contact name from configuration
	QString contactName =
			g_pConfig->value( QString( contactIdentifier ), QString( "" ) ).toString();
	MWTS_DEBUG( QString( "Contact name: %1" ).arg( contactName ) );

	bool ret = (*mWaiter)( mAccount->ensureTextChat( contactName,
							  userActionTime,
							  preferredHandler ) );
	if ( ret == true )
	{
		// AccountManager is now ready
		qDebug() << "Succesfully requested TextChat.";
		if ( mChannelHandler->mChannels.size() > 0 )
		{
			Tp::TextChannelPtr chan =
				Tp::TextChannelPtr( qobject_cast<Tp::TextChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
			mTextChat = new TextChat( chan );
			mChannelHandler->mChannels.removeAt( 0 );
		}
	}
	else
	{
		qWarning() << "Failed to request TextChat!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_ERROR(QString("%1: %2").arg(mWaiter->lastErrorName()).arg(mWaiter->lastErrorMessage()));
	}
	MWTS_LEAVE;
	return ret;
	
	
}


/**
 * SendMessage
 *
 * Gives attributes what kind of message is sent. 
 * 
 * @param length Length of the message to be sent. Only few alternatives
 * @param coding arab, hebrew, chinese, unicode
 * @param classtype class of the message
 * @return bool success of the method
 */
bool TelepathyTest::SendMessage(const QString& message,
				const QString& length,
				const QString& coding,
				const QString& classtype)
{
	MWTS_ENTER;
	bool retValue = false;
	if ( mTextChat )
	{
		retValue = mTextChat->SendMessage( message, length, coding, classtype );

		// because channels are contact-specific,
		// this chat has done its course and
		// old chat and channel are not needed
		mSentMessageContent = mTextChat->SentMessageText();
		mSentMessageLength = length;
		mSentMessageCoding = coding;
		mSentMessageClassType = classtype;

		delete mTextChat;
		mTextChat = NULL;
		
		mChannelHandler->CloseChannels();
	}
	MWTS_LEAVE;
	return retValue;
}


/**
 * WaitForIncomingMessage
 *
 * Waits for new channel to be created.
 * If it is text channel, it gets the message form the text channel.
 *
 * @return bool success of the method
 */
bool TelepathyTest::WaitForIncomingMessage()
{
	MWTS_ENTER;
	mRetValue = false;
	qDebug() << "Waiting for incoming message...";

	if ( mHaveConnection )
	{
		Tp::ConnectionPtr conn = mAccount->connection();
		if ( conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
		{
			qDebug() << "Connecting to NewChannels...";
			connect(conn->requestsInterface(),
					SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
					SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
		}
	}

	// Timeout for handling error situations where channels are not got
	mTimer->start( TEST_TIMEOUT );
	mEventLoop->exec();

	// After event loop is exited, there should be proper channel creater or already existing
	if(!mTextChat && mChannelHandler->mChannels.size() > 0 )
	{
	  	qDebug() << "Creating new textchat. Getting new text channel.";
		Tp::TextChannelPtr chan =
			Tp::TextChannelPtr( qobject_cast<Tp::TextChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
		mTextChat = new TextChat( chan );

        qDebug() << "Channel amount == " << mChannelHandler->mChannels.size();
        //qDebug() << "TextChannel amount == " << mTextHandler->mChannels.size();

		mChannelHandler->mChannels.removeAt( 0 );

        qDebug() << "Has message interface == "<< chan.data()->hasMessagesInterface();

        mRetValue = mTextChat->VerifyReceivedMessage(mSentMessageContent,
                        mSentMessageLength,
                        mSentMessageCoding,
                        mSentMessageClassType);

        delete mTextChat;
		mTextChat = NULL;

	}
	// Here is the message requested and processed.
	// Finally determines the success of the case

	Tp::ConnectionPtr conn = mAccount->connection();
	if ( conn.data() && conn->isValid() &&conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
	{
		qDebug() << "Disconnecting from NewChannels";
		disconnect(conn->requestsInterface(),
			SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
			this,
			SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
	}

	MWTS_LEAVE;
	return mRetValue;
}


bool TelepathyTest::WaitForIncomingIM()
{
	MWTS_ENTER;
	mRetValue = false;
	qDebug() << "Waiting for incoming instant message...";

	if ( mHaveConnection )
	{
		Tp::ConnectionPtr conn = mAccount->connection();
		if ( conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
		{
			qDebug() << "Connecting to NewChannels...";
			connect(conn->requestsInterface(),
					SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
					SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
		}
     }

	// Timeout for handling error situations where channels are not got
	mTimer->start( TEST_TIMEOUT );
	mEventLoop->exec();

	// After event loop is exited, there should be proper channel creater or already existing
	if( !mTextChat && mChannelHandler->mChannels.size() > 0 )
	{
	  	qDebug() << "Creating new textchat. Getting new text channel.";
		Tp::TextChannelPtr chan =
			Tp::TextChannelPtr( qobject_cast<Tp::TextChannel*>( mChannelHandler->mChannels.at( 0 ).data() ) );
		mTextChat = new TextChat( chan );

		mChannelHandler->mChannels.removeAt( 0 );

        mRetValue = mTextChat->VerifyReceivedIM();

        delete mTextChat;
		mTextChat = NULL;

	}

	Tp::ConnectionPtr conn = mAccount->connection();
	if ( conn.data() && conn->isValid() &&conn->interfaces().contains( TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS ) )
	{
		qDebug() << "Disconnecting from NewChannels";
		disconnect(conn->requestsInterface(),
			SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
			this,
			SLOT(onNewChannels(const Tp::ChannelDetailsList&)));
	}

    MWTS_LEAVE;
	return mRetValue;
}

