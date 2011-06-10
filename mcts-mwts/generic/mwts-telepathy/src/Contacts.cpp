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
#include "Contacts.h"
#include "PendingOperationWaiter.h"


const int CONTACT_TIMEOUT = 300000;




ContactManagement::ContactManagement( const Tp::ConnectionPtr& aConnection, QObject* pParent )
 : QObject( pParent ),
   mConnection( aConnection ),
   mContactManager( NULL )
{
	MWTS_ENTER;
	
	if ( mConnection.isNull() )
	{
		qWarning() << "NULL connection pointer!";
		MWTS_LEAVE;
		return;
	}
	
	mWaiter = PendingOperationWaiter::instance();
	
	if ( mConnection->isReady() == false )
	{
		// Make sure that connection has FeatureRoster
		// Execution will not continue, until becomeReady has finished
		Tp::Features feats;
		feats.insert( Tp::Connection::FeatureCore );
		feats.insert( Tp::Connection::FeatureSelfContact );
		feats.insert( Tp::Connection::FeatureSimplePresence );
		feats.insert( Tp::Connection::FeatureRoster );
		feats.insert( Tp::Connection::FeatureRosterGroups );
		if ( (*mWaiter)( mConnection->becomeReady( feats ) ) == false )
		{
			qWarning() << mWaiter->lastErrorName()
				<< ": " << mWaiter->lastErrorMessage();
			MWTS_LEAVE;
			return;
		}
	}
	
        mContactManager = mConnection->contactManager().data();
	if ( mContactManager )
	{
		connect( mContactManager,
			 SIGNAL(presencePublicationRequested(const Tp::Contacts&)),
			 SLOT(onPresencePublicationRequested(const Tp::Contacts&)) );
		connect( mContactManager,
			 SIGNAL(groupAdded(const QString&)),
			 SLOT(onGroupAdded(const QString&)) );
		connect( mContactManager,
			 SIGNAL(groupRemoved(const QString&)),
			 SLOT(onGroupRemoved(const QString&)) );
		connect( mContactManager,
			 SIGNAL(groupMembersChanged(const QString&,const Tp::Contacts&,const Tp::Contacts&)),
			 SLOT(onGroupMembersChanged(const QString&,const Tp::Contacts&,const Tp::Contacts&)) );
	}
	else
	{
		qWarning() << "No contact manager!";
		MWTS_LEAVE;
		return;
	}
	
	MWTS_LEAVE;
}


ContactManagement::~ContactManagement()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}


bool ContactManagement::contactsForIdentifiers( const QStringList& identifiers, Tp::Contacts& contacts )
{
	MWTS_ENTER;
	QList<Tp::ContactPtr> contactList;
        QSet<Tp::Feature> features;
	
	features.insert( Tp::Contact::FeatureAlias );
	features.insert( Tp::Contact::FeatureAvatarToken );
	features.insert( Tp::Contact::FeatureSimplePresence );
	features.insert( Tp::Contact::FeatureCapabilities );
	
	Tp::PendingContacts* pc = mContactManager->contactsForIdentifiers( identifiers, features );
	bool ret = mWaiter->handlesToContacts( pc, contactList );
	if ( ret == false )
	{
		qWarning() << "Failed to request contacts!";
		qWarning() << mWaiter->lastErrorName() << mWaiter->lastErrorMessage(); 
		MWTS_LEAVE;
		return false;
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
	contacts = Tp::Contacts::fromList( contactList );
	MWTS_LEAVE;
	return true;
}


void ContactManagement::onPresencePublicationRequested( const Tp::Contacts& contacts )
{
	MWTS_ENTER;
	foreach ( Tp::ContactPtr contact, contacts )
	{
		qDebug() << contact->id() << " requested presence publication.";
	}
	MWTS_LEAVE;
}


void ContactManagement::onGroupAdded( const QString& group )
{
	MWTS_ENTER;
	qDebug() << "Group added: " << group;
	MWTS_LEAVE;
}


void ContactManagement::onGroupRemoved( const QString& group )
{
	MWTS_ENTER;
	qDebug() << "Group removed: " << group;
	MWTS_ENTER;
}


void ContactManagement::onGroupMembersChanged(	const QString& group,
						const Tp::Contacts& groupMembersAdded,
						const Tp::Contacts& groupMembersRemoved )
{
	MWTS_ENTER;
	qDebug() << "Group members changed for group: " << group;
	qDebug() << "Group members added: ";
	foreach ( Tp::ContactPtr contact, groupMembersAdded )
	{
		qDebug() << contact->id();
	}
	qDebug() << "Group members removed: ";
	foreach ( Tp::ContactPtr contact, groupMembersRemoved )
	{
		qDebug() << contact->id();
	}
}




Contacts::Contacts( const Tp::PendingContacts* aPendingContacts, QObject* pParent )
 : QObject( pParent )
{
	MWTS_ENTER;
	
	mWaiter = PendingOperationWaiter::instance();
	mEventLoop = new QEventLoop;
	
	connect( aPendingContacts,
		 SIGNAL(finished(Tp::PendingOperation*)),
		 SLOT(onPendingContactsFinished(Tp::PendingOperation*)) );
	
	QTimer::singleShot( CONTACT_TIMEOUT, this, SLOT(contactTimeout()) );
	mEventLoop->exec();
	
	MWTS_LEAVE;
}


Contacts::~Contacts()
{
	MWTS_ENTER;
	if ( mEventLoop )
	{
		if ( mEventLoop->isRunning() )
		{
			mEventLoop->quit();
		}
		delete mEventLoop;
		mEventLoop = NULL;
	}
	while ( mContacts.empty() == false )
	{
		Contact* c = mContacts.takeFirst();
		delete c;
		c = NULL;
	}
	disconnect();
	MWTS_LEAVE;
}


void Contacts::onPendingContactsFinished( Tp::PendingOperation* operation )
{
	qDebug() << ">>> " << __PRETTY_FUNCTION__;
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mEventLoop->quit();
	}
	if ( operation->isFinished() == false )
	{
		qDebug() << "Operation has not finished yet!";
		mRetValue = false;
		qDebug() << "<<< " << __PRETTY_FUNCTION__;
		return;
	}
	if ( operation->isValid() == false )
	{
		qDebug() << "Operation is not valid!";
		mRetValue = false;
		qDebug() << "<<< " << __PRETTY_FUNCTION__;
		return;
	}
	if ( operation->isError() == true )
	{
		qWarning() << operation->errorName() << ": " << operation->errorMessage();
		mRetValue = false;
		qDebug() << "<<< " << __PRETTY_FUNCTION__;
		return;
	}
	
	// Get the contacts
	Tp::PendingContacts* pc = qobject_cast<Tp::PendingContacts*>( operation );
	QList<Tp::ContactPtr> contacts = pc->contacts();
	foreach ( Tp::ContactPtr contact, contacts )
	{
		if ( contact.isNull() )
		{
			qDebug() << "Contact is NULL.";
		}
		else
		{
			qDebug() << "Contact id: " << contact->id();
			Contact* c = new Contact( contact );
			mContacts.append( c );
		}
	}
	
	mRetValue = true;
	qDebug() << "<<< " << __PRETTY_FUNCTION__;
}


void Contacts::contactTimeout()
{
	MWTS_ENTER;
	if ( mEventLoop->isRunning() )
	{
		qDebug() << "Event loop running! Stopping...";
		mRetValue = false;
		mEventLoop->quit();
	}
	MWTS_LEAVE;
}


Contact::Contact( const Tp::ContactPtr& aContact, QObject* pParent )
 : QObject( pParent ), mContact( aContact )
{
	MWTS_ENTER;
	connect( mContact.data(),
		 SIGNAL(aliasChanged(const QString&)),
		 SLOT(onAliasChanged(const QString&)) );
	connect( mContact.data(),
		 SIGNAL(simplePresenceChanged(const QString&,uint,const QString&)),
		 SLOT(onSimplePresenceChanged(const QString&,uint,const QString&)) );
	connect( mContact.data(),
		 SIGNAL(capabilitiesChanged(Tp::ContactCapabilities*)),
		 SLOT(onCapabilitiesChanged(Tp::ContactCapabilities*)) );
		
	connect( mContact.data(),
		 SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
		 SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)) );
	connect( mContact.data(),
		 SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
		 SLOT(onPublishStateChanged(Tp::Contact::PresenceState)) );
	connect( mContact.data(),
		 SIGNAL(blockStatusChanged(bool)),
		 SLOT(onBlockStatusChanged(bool)) );
		
	connect( mContact.data(),
		 SIGNAL(addedToGroup(const QString&)),
		 SLOT(onAddedToGroup(const QString&)) );
	connect( mContact.data(),
		 SIGNAL(removedFromGroup(const QString&)),
		 SLOT(onRemovedFromGroup(const QString&)) );
	MWTS_LEAVE;
}

Contact::~Contact()
{
	MWTS_ENTER;
        //mContact.clear();
	disconnect();
	MWTS_LEAVE;
}


void Contact::onAliasChanged( const QString& alias )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Contacts alias was changed to " << alias;
	MWTS_LEAVE;
}


void Contact::onAvatarTokenChanged( const QString& avatarToken )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Contacts avatas token was changed to " << avatarToken;
	MWTS_LEAVE;
}


void Contact::onSimplePresenceChanged( const QString& status, uint type, const QString& presenceMessage )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Presence status: " << status;
	qDebug() << "Presence message: " << presenceMessage;
	switch ( type )
	{
		case Tp::ConnectionPresenceTypeUnset:
			qDebug() << "Presence type: An invalid presence type used as a null value.";
			break;
		case Tp::ConnectionPresenceTypeOffline:
			qDebug() << "Presence type: Offline.";
			break;
		case Tp::ConnectionPresenceTypeAvailable:
			qDebug() << "Presence type: Available.";
			break;
		case Tp::ConnectionPresenceTypeAway:
			qDebug() << "Presence type: Away.";
			break;
		case Tp::ConnectionPresenceTypeExtendedAway:
			qDebug() << "Presence type: Away for an extended time.";
			break;
		case Tp::ConnectionPresenceTypeHidden:
			qDebug() << "Presence type: Hidden (invisible).";
			break;
		case Tp::ConnectionPresenceTypeBusy:
			qDebug() << "Presence type: Busy, Do Not Disturb.";
			break;
		case Tp::ConnectionPresenceTypeUnknown:
			qDebug() << "Presence type: Unknown, unable to determine presence for this contact.";
			break;
		case Tp::ConnectionPresenceTypeError:
			qDebug() << "Presence type: Error, an error occurred while trying to determine presence.";
			break;
	}
	MWTS_LEAVE;
}


void Contact::onCapabilitiesChanged( Tp::ContactCapabilities* caps )
{
	MWTS_ENTER;
	Q_UNUSED(caps);
	qDebug() << "Contact ID: " << mContact->id();
	MWTS_LEAVE;
}


void Contact::onSubscriptionStateChanged( Tp::Contact::PresenceState state )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	switch ( state )
	{
		case Tp::Contact::PresenceStateNo:
			qDebug() << "Subscription state: No";
			break;
		case Tp::Contact::PresenceStateAsk:
			qDebug() << "Subscription state: Ask";
			break;
		case Tp::Contact::PresenceStateYes:
			qDebug() << "Subscription state: Yes";
			break;
	}
	MWTS_LEAVE;
}


void Contact::onPublishStateChanged( Tp::Contact::PresenceState state )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	switch ( state )
	{
		case Tp::Contact::PresenceStateNo:
			qDebug() << "Publish state: No";
			break;
		case Tp::Contact::PresenceStateAsk:
			qDebug() << "Publish state: Ask";
			break;
		case Tp::Contact::PresenceStateYes:
			qDebug() << "Publish state: Yes";
			break;
	}
	MWTS_LEAVE;
}


void Contact::onBlockStatusChanged( bool blocked )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Is contact blocked? " << ( blocked ? "Yes" : "No" );
	MWTS_LEAVE;
}


void Contact::onAddedToGroup( const QString& group )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Contact was added to group " << group;
	MWTS_LEAVE;
}


void Contact::onRemovedFromGroup( const QString& group )
{
	MWTS_ENTER;
	qDebug() << "Contact ID: " << mContact->id();
	qDebug() << "Contact was removed from group " << group;
	MWTS_LEAVE;
}


