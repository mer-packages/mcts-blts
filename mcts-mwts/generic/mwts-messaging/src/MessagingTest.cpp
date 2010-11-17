/* MessagingTest.cpp -- Source code for Messaging test class

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

#include "stable.h"
#include "MessagingTest.h"


QTM_USE_NAMESPACE

/**
 * Constructor for Messaging test class
 */
MessagingTest::MessagingTest()
	: mMsgService( NULL ),
	  mMsgManager( NULL ),
	  mSelfNumber( "" ),
	  mMsgBody( "" ),
	  mRetValue( false ),
	  mSmsFilterId( 0 ),
	  mSendLatency( 0 ),
	  mReceiveLatency( 0 )
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Destructor for Messaging test class
 */
MessagingTest::~MessagingTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void MessagingTest::OnInitialize()
{
	MWTS_ENTER;

	mMsgService = new QMessageService;
	connect( mMsgService, SIGNAL(messagesCounted(int)),
			 this, SLOT(onMessagesCounted(int)) );
	connect( mMsgService, SIGNAL(messagesFound(const QMessageIdList&)),
			 this, SLOT(onMessagesFound(const QMessageIdList&)) );
	connect( mMsgService, SIGNAL(progressChanged(uint,uint)),
			 this, SLOT(onProgressChanged(uint,uint)) );
	connect( mMsgService, SIGNAL(stateChanged(QMessageService::State)),
			 this, SLOT(onStateChanged(QMessageService::State)) );

	mMsgManager = new QMessageManager;
	connect( mMsgManager, SIGNAL(messageAdded(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)),
			 this, SLOT(onMessageAdded(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)) );
	connect( mMsgManager, SIGNAL(messageRemoved(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)),
			 this, SLOT(onMessageRemoved(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)) );
	connect( mMsgManager, SIGNAL(messageUpdated(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)),
			 this, SLOT(onMessageUpdated(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)) );

	QMessageFilter smsFilter = QMessageFilter::byType( QMessage::Sms );
	mSmsFilterId = mMsgManager->registerNotificationFilter( smsFilter );

	// Read DUT number from conf.
	QVariant v = g_pConfig->value( "self-number", "" );
	if ( v.canConvert( QVariant::String ) )
	{
		mSelfNumber = v.value<QString>();
	}
	if ( mSelfNumber.length() == 0 )
	{
		qCritical() << "Failed to read configuration.";
	}
	else
	{
		qDebug() << "Self number: " << mSelfNumber;
	}

	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void MessagingTest::OnUninitialize()
{
	MWTS_ENTER;

	mMsgManager->unregisterNotificationFilter( mSmsFilterId );

	disconnect();

	if ( mMsgManager )
	{
		delete mMsgManager;
		mMsgManager = NULL;
	}

	if ( mMsgService )
	{
		delete mMsgService;
		mMsgService = NULL;
	}

	MWTS_LEAVE;
}


void MessagingTest::ReportLatencies()
{
	MWTS_ENTER;
	qDebug() << "Send latency: " << mSendLatency << " msec";
	qDebug() << "Receive latency: " << mReceiveLatency << " msec";
	g_pResult->AddMeasure( "Send latency", mSendLatency, "msec" );
	g_pResult->AddMeasure( "Receive latency", mReceiveLatency, "msec" );
	MWTS_LEAVE;
}


/**
 * Queries all accounts available for Qt Mobility Messaging
 */
void MessagingTest::QueryAllAccounts()
{
	MWTS_ENTER;
	
	QMessageAccountIdList accIds = mMsgManager->queryAccounts();

	foreach ( QMessageAccountId id, accIds )
	{
		QMessageAccount account = mMsgManager->account( id );

		qDebug() << "Account name: " << account.name();
		QMessage::TypeFlags msgTypes = account.messageTypes();
		qDebug() << "Message type flags: " << msgTypes;
		qDebug() << "Supported message types: ";

		if ( msgTypes & QMessage::NoType )
		{
			qDebug() << "  The message type is not defined.";
		}
		if ( msgTypes & QMessage::Mms )
		{
			qDebug() << "  The message is an MMS, Multimedia Messaging Service object.";
		}
		if ( msgTypes & QMessage::Sms )
		{
			qDebug() << "  The message is an SMS, Short Message Service object.";
		}
		if ( msgTypes & QMessage::Email )
		{
			qDebug() << "  The message is an Email, Internet Message Format object.";
		}
		if ( msgTypes & QMessage::InstantMessage )
		{
			qDebug() << "  The message is an instant message object, such as XMPP.";
		}
		if ( msgTypes & QMessage::AnyType )
		{
			qDebug() << "  Bitflag value that matches any message type defined.";
		}
	}

	// write results. the first one is just informal if you want to.
	g_pResult->Write( "Succesfully queried acounts!" );
	g_pResult->StepPassed( "QueryAllAccounts", true );

	MWTS_LEAVE;
}


/**
 * Prints preferred charsets for messages
 */
void MessagingTest::PreferredCharsets()
{
	MWTS_ENTER;
	QList<QByteArray> charSets = QMessage::preferredCharsets();
	QList<QByteArray> codecs = QTextCodec::availableCodecs();

	qDebug() << "List of all available codecs: ";
	foreach ( QByteArray codec, codecs )
	{
		qDebug() << "  " << codec;
	}

	qDebug() << "List of charset names to use: ";
	foreach ( QByteArray charSet, charSets )
	{
		qDebug() << "  " << charSet;
	}

	qDebug() << "Required character set: ISO-10646-UCS-2";
	QTextCodec* ucs2Codec = QTextCodec::codecForName( "ISO-10646-UCS-2" );
	if ( ucs2Codec == NULL )
	{
		qWarning() << "No codec for ISO-10646-UCS-2";
	}
	else
	{
		qDebug() << "Found codec for ISO-10646-UCS-2";
		qDebug() << "Using codec with name: " << ucs2Codec->name();
	}

	qDebug() << "Required MIBenum: 1000";
	QTextCodec* mibCodec = QTextCodec::codecForMib( 1000 );
	if ( mibCodec == NULL )
	{
		qWarning() << "No codec for MIBenum 1000";
	}
	else
	{
		qDebug() << "Found codec for MIBenum 1000";
		qDebug() << "Using codec with name: " << mibCodec->name();
	}

	g_pResult->Write( "Succesfully printed charsets!" );
	g_pResult->StepPassed( "PreferredCharsets", true );
	MWTS_LEAVE;
}


/**
 * Send SMS to given number
 */
void MessagingTest::SendSMS( const QString& message )
{
	MWTS_ENTER;

	QString dataPath = g_pConfig->value( "data-file-path", "" ).toString();
	QString dataFile = g_pConfig->value( message, "" ).toString();
	if ( dataPath.length() == 0 || dataFile.length() == 0 )
	{
		qCritical() << "Failed to read configuration.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}
	else
	{
		qDebug() << "Data path: " << dataPath;
		qDebug() << "Data file: " << dataFile;
	}


	QFile data( dataPath + dataFile );
	if ( data.open( QIODevice::Text | QIODevice::ReadOnly ) == false )
	{
		qCritical() << "Failed to read data file.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}

	QTextStream textStream( &data );

	QMessageAccountId smsAccountId =
			QMessageAccount::defaultAccount( QMessage::Sms );

	qDebug() << "SMS Account ID: " << smsAccountId.toString();

	QMessageAddress address( QMessageAddress::Phone, mSelfNumber );
	QMessage msg;

	msg.setType( QMessage::Sms );
	msg.setTo( address );
	mMsgBody = textStream.readAll();
	msg.setBody( mMsgBody );
	msg.setParentAccountId( smsAccountId );

	bool ret = sendMessage( msg );

	g_pResult->StepPassed( "SendSMS", ret );

	data.close();

	MWTS_LEAVE;
}


/**
 * Send SMS to self number with given encoding
 */
void MessagingTest::SendSMS( const QString& message, const QString& encoding )
{
	MWTS_ENTER;

	QString dataPath = g_pConfig->value( "data-file-path", "" ).toString();
	QString dataFile = g_pConfig->value( message, "" ).toString();
	if ( dataPath.length() == 0 || dataFile.length() == 0 )
	{
		qCritical() << "Failed to read configuration.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}
	else
	{
		qDebug() << "Data path: " << dataPath;
		qDebug() << "Data file: " << dataFile;
	}


	QFile data( dataPath + dataFile );
	if ( data.open( QIODevice::Text | QIODevice::ReadOnly ) == false )
	{
		qCritical() << "Failed to read data file.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}

	QTextStream textStream( &data );

	qDebug() << "Required character set: " << encoding;
	QTextCodec* codec = QTextCodec::codecForName( encoding.toAscii() );
	if ( codec == NULL )
	{
		qCritical() << "No codec for " << encoding;
		g_pResult->StepPassed( "SendSMS", false );
		data.close();
		MWTS_LEAVE;
		return;
	}
	else
	{
		qDebug() << "Found codec for " << encoding;
		qDebug() << "Using codec with name: " << codec->name();
	}
	textStream.setCodec( codec );

	QMessageAccountId smsAccountId =
			QMessageAccount::defaultAccount( QMessage::Sms );

	qDebug() << "SMS Account ID: " << smsAccountId.toString();

	QMessageAddress address( QMessageAddress::Phone, mSelfNumber );
	QMessage msg;

	msg.setType( QMessage::Sms );
	mMsgBody = textStream.readAll();
	msg.setTo( address );
	msg.setBody( mMsgBody );
	msg.setParentAccountId( smsAccountId );

	bool ret = sendMessage( msg );

	g_pResult->StepPassed( "SendSMS", ret );

	data.close();

	MWTS_LEAVE;
}


/**
 * Send SMS to self number with given MIB of a known encoding
 */
void MessagingTest::SendSMS( const QString& message, int mib )
{
	MWTS_ENTER;

	QString dataPath = g_pConfig->value( "data-file-path", "" ).toString();
	QString dataFile = g_pConfig->value( message, "" ).toString();
	if ( dataPath.length() == 0 || dataFile.length() == 0 )
	{
		qCritical() << "Failed to read configuration.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}
	else
	{
		qDebug() << "Data path: " << dataPath;
		qDebug() << "Data file: " << dataFile;
	}


	QFile data( dataPath + dataFile );
	if ( data.open( QIODevice::Text | QIODevice::ReadOnly ) == false )
	{
		qCritical() << "Failed to read data file.";
		g_pResult->StepPassed( "SendSMS", false );
		MWTS_LEAVE;
		return;
	}

	QTextStream textStream( &data );

	qDebug() << "Required MIBenum: " << mib;
	QTextCodec* codec = QTextCodec::codecForMib( mib );
	if ( codec == NULL )
	{
		qCritical() << "No codec for MIBenum " << mib;
		g_pResult->StepPassed( "SendSMS", false );
		data.close();
		MWTS_LEAVE;
		return;
	}
	else
	{
		qDebug() << "Found codec for MIBenum " << mib;
		qDebug() << "Using codec with name: " << codec->name();
	}
	textStream.setCodec( codec );

	QMessageAccountId smsAccountId =
			QMessageAccount::defaultAccount( QMessage::Sms );

	qDebug() << "SMS Account ID: " << smsAccountId.toString();

	QMessageAddress address( QMessageAddress::Phone, mSelfNumber );
	QMessage msg;

	msg.setType( QMessage::Sms );
	msg.setTo( address );
	mMsgBody = textStream.readAll();
	msg.setBody( mMsgBody );
	msg.setParentAccountId( smsAccountId );

	bool ret = sendMessage( msg );

	Start();

	g_pResult->StepPassed( "SendSMS", ret );

	data.close();

	MWTS_LEAVE;
}


/**
 * Waits for incoming message
 */
void MessagingTest::WaitForIncomingMessage()
{
	MWTS_ENTER;

	mRetValue = false;

	qDebug() << "Waiting for incoming message...";

	mReceiveTime.start();
	Start();

	if ( mRetValue == false )
	{
		qCritical() << "Failed to verify message!";
	}
	else
	{
		qDebug() << "Message verified!";
	}

	g_pResult->StepPassed( "WaitForIncomingMessage", mRetValue );
	MWTS_LEAVE;
}


bool MessagingTest::sendMessage( QMessage& message )
{
	MWTS_ENTER;

	mSendTime.start();
	bool ret = mMsgService->send( message );
	Start();

	if ( ret == true )
	{
		qDebug() << "Succesfully sent message!";
	}
	else
	{
		qCritical() << "Failed to send message!";

		QString errStr;
		QMessageManager::Error error = mMsgService->error();
		switch ( error )
		{
			case QMessageManager::NoError:
				errStr = "The operation was successfully performed.";
				break;
			case QMessageManager::InvalidId:
				errStr = "The operation failed due to the specification of an invalid identifier.";
				break;
			case QMessageManager::ConstraintFailure:
				errStr = "The operation failed due to a constraint violation.";
				break;
			case QMessageManager::ContentInaccessible:
				errStr = "The operation failed because the content data cannot be accessed by the messaging store.";
				break;
			case QMessageManager::NotYetImplemented:
				errStr = "The operation failed because the messaging store does not yet implement the operation.";
				break;
			case QMessageManager::FrameworkFault:
				errStr = "The operation failed because the messaging store encountered an error in performing the operation.";
				break;
			case QMessageManager::WorkingMemoryOverflow:
				errStr = "The operation failed because the messaging store exhausted all memory available for evaluating queries.";
				break;
			case QMessageManager::Busy:
				errStr = "The operation failed because the messaging store is being used by another thread.";
				break;
			case QMessageManager::RequestIncomplete:
				errStr = "The operation failed to report successful completion, although no specific error was reported.";
				break;
		}
		if ( error == QMessageManager::NoError )
		{
			qDebug() << errStr;
		}
		else
		{
			qCritical() << errStr;
		}
	}

	MWTS_LEAVE;
	return ret;
}


void MessagingTest::onMessageAdded( const QMessageId& id,
		 const QMessageManager::NotificationFilterIdSet& matchingFilterIds )
{
	Q_UNUSED( matchingFilterIds );
	MWTS_ENTER;

	qDebug() << "Message Added: " << id.toString();

	QMessage msg( id );

	if ( msg.standardFolder() == QMessage::InboxFolder )
	{
		mReceiveLatency = mReceiveTime.elapsed();

		QMessageContentContainerId bodyId = msg.bodyId();
		QMessageContentContainer body = msg.find( bodyId );
		QString bodyStr = body.textContent();

		qDebug() << "From: " << msg.from().addressee();
		qDebug() << "Subject: " << msg.subject();
		qDebug() << "Body content: " << bodyStr;

		switch ( msg.type() )
		{
			case QMessage::NoType:
				qDebug() << "The message type is not defined.";
				break;
			case QMessage::Mms:
				qDebug() << "The message is an MMS, Multimedia Messaging Service object.";
				break;
			case QMessage::Sms:
				qDebug() << "The message is an SMS, Short Message Service object.";
				break;
			case QMessage::Email:
				qDebug() << "The message is an Email, Internet Message Format object.";
				break;
			case QMessage::InstantMessage:
				qDebug() << "The message is an instant message object, such as XMPP.";
				break;
		}

		QMessage::StatusFlags status = msg.status();
		if ( status & QMessage::Read )
		{
			qDebug() << "The content of this message has been displayed to the user.";
		}
		if ( status & QMessage::HasAttachments )
		{
			qDebug() << "The message contains at least one sub-part with 'Attachment' disposition.";
		}
		if ( status & QMessage::Incoming )
		{
			qDebug() << "The message has been sent from an external source.";
		}
		if ( status & QMessage::Removed )
		{
			qDebug() << "The message has been deleted from or moved on the originating server.";
		}

		switch ( msg.standardFolder() )
		{
			case QMessage::InboxFolder:
				qDebug() << "The message is in the standard inbox folder.";
				break;
			case QMessage::DraftsFolder:
				qDebug() << "The message is in the standard drafts folder.";
				break;
			case QMessage::OutboxFolder:
				qDebug() << "The message is in the standard outbox folder.";
				break;
			case QMessage::SentFolder:
				qDebug() << "The message is in the standard sent folder.";
				break;
			case QMessage::TrashFolder:
				qDebug() << "The message is in the standard trash folder.";
				break;
		}

		mRetValue = ( mMsgBody.compare( bodyStr ) == 0 );

		Stop();
	}
	else if ( msg.standardFolder() == QMessage::SentFolder )
	{
		qDebug() << "Message was placed to the sent folder.";

		mSendLatency = mSendTime.elapsed();

		Stop();
	}

	MWTS_LEAVE;
}


void MessagingTest::onMessageRemoved( const QMessageId& id,
		 const QMessageManager::NotificationFilterIdSet& matchingFilterIds )
{
	Q_UNUSED( matchingFilterIds );
	MWTS_ENTER;
	qDebug() << "Message Removed: " << id.toString();
	MWTS_LEAVE;
}


void MessagingTest::onMessageUpdated( const QMessageId& id,
		 const QMessageManager::NotificationFilterIdSet& matchingFilterIds )
{
	Q_UNUSED( matchingFilterIds );
	MWTS_ENTER;
	qDebug() << "Message Updated: " << id.toString();
	MWTS_LEAVE;
}


void MessagingTest::onMessagesCounted( int count )
{
	MWTS_ENTER;
	qDebug() << "Number of messages counted: " << count;
	MWTS_LEAVE;
}


void MessagingTest::onMessagesFound( const QMessageIdList& ids )
{
	MWTS_ENTER;
	foreach( QMessageId id, ids )
	{
		qDebug() << "Found message with id " << id.toString();
	}
	MWTS_LEAVE;
}


void MessagingTest::onProgressChanged( uint value, uint total )
{
	MWTS_ENTER;
	if ( total == 0 )
	{
		qDebug() << "Steps completed: " << value;
	}
	else
	{
		qDebug() << "Progress: " << value << " / " << total;
	}
	MWTS_LEAVE;
}


void MessagingTest::onStateChanged( QMessageService::State newState )
{
	MWTS_ENTER;
	qDebug() << "Current state of operation:";
	switch ( newState )
	{
		case QMessageService::InactiveState:
			qDebug() << "  The operation has not yet begun execution.";
			break;
		case QMessageService::ActiveState:
			qDebug() << "  The operation is currently executing.";
			break;
		case QMessageService::CanceledState:
			qDebug() << "  The operation was canceled.";
			break;
		case QMessageService::FinishedState:
			qDebug() << "  The operation has finished execution;"
					 << "succesfully completed or otherwise.";
			break;
	}
	MWTS_LEAVE;
}

