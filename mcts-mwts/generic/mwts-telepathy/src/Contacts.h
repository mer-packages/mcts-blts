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
 *  @file       Contacts.h
 *  @brief      Methods for managing contacts.
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#ifndef _INCLUDED_TELEPATHY_CONTACTS_H
#define _INCLUDED_TELEPATHY_CONTACTS_H


#include <QObject>
#include <TelepathyQt4/Contact>


class PendingOperationWaiter;
namespace Tp
{
	class PendingOperation;
	class PendingContacts;
}
class QEventLoop;


class ContactManagement : public QObject
{
	Q_OBJECT
	
	public:
		ContactManagement( const Tp::ConnectionPtr& aConnection, QObject* pParent = 0 );
		~ContactManagement();
	
	public: // API
		
		bool contactsForIdentifiers( const QStringList& identifiers, Tp::Contacts& contacts );
		
	private Q_SLOTS:
		
		// From Contact Manager
		void onPresencePublicationRequested( const Tp::Contacts& contacts );
		void onGroupAdded( const QString& group );
		void onGroupRemoved( const QString& group );
		void onGroupMembersChanged(	const QString& group,
						const Tp::Contacts& groupMembersAdded,
						const Tp::Contacts& groupMembersRemoved );
		
	private: // Data
		
		// Connection for this session
		Tp::ConnectionPtr		mConnection;
		
		// Contact manager for this session
		Tp::ContactManager*		mContactManager;
		
		// Object, that syncronizes Tp::PendingOperation obejct signaling
		PendingOperationWaiter*		mWaiter;
};



/**
 *  @class Contact
 *  @brief Contact specific implementation
 *  
 *  Class Contact is built around contacts.
 *  It is capable of handling contact related functionality.
 */
class Contact : public QObject
{
	Q_OBJECT
	
	public:
		/**
		 *  @fn Contact( const Tp::ContactPtr aContact, QObject* pParent = 0 )
		 *  @brief Constructor for Contact.
		 *  @param aContact Tp::Contact object
		 *  @param pParent  Pointer to parent QObject. Default is NULL.
		 */
		Contact( const Tp::ContactPtr& aContact, QObject* pParent = 0 );
		
		/**
		 *  @fn virtual Contact()
		 *  @brief Destructor for Contact.
		 */
		virtual ~Contact();
		
		Tp::ContactPtr& contact() { return mContact; }
		
	private Q_SLOTS:
		
		// From Contact
		void onAliasChanged( const QString& alias );
		void onAvatarTokenChanged( const QString& avatarToken );
		void onSimplePresenceChanged( const QString& status, uint type, const QString& presenceMessage );
		void onCapabilitiesChanged( Tp::ContactCapabilities* caps );
		
		void onSubscriptionStateChanged( Tp::Contact::PresenceState state );
		void onPublishStateChanged( Tp::Contact::PresenceState state );
		void onBlockStatusChanged( bool blocked );
		
		void onAddedToGroup( const QString& group );
		void onRemovedFromGroup( const QString& group );
	
	private:
		
		// Contact to handle
		Tp::ContactPtr mContact;
};



/**
 *  @class Contacts
 *  @brief Contacts specific implementation
 *  
 *  Class Contacts is built around contacts.
 *  It is capable of handling contact related functionality.
 */
class Contacts : public QObject
{
	Q_OBJECT
	
	public:
		/**
		 *  @fn Contacts( const Tp::PendingContacts* aPendingContacts, QObject* pParent = 0 )
		 *  @brief Constructor for Contacts.
		 *  @param aPendingContacts Tp::PendingContacts object for requested contacts
		 *  @param pParent  Pointer to parent QObject. Default is NULL.
		 */
		Contacts( const Tp::PendingContacts* aPendingContacts, QObject* pParent = 0 );
		
		/**
		 *  @fn virtual ~Contacts()
		 *  @brief Destructor for Contacts.
		 */
		virtual ~Contacts();
		
		// List of contact objects
		QList<Contact*>			mContacts;
		
	public: // Call API
		
	private Q_SLOTS:
		
		// From PendingContacts
		void onPendingContactsFinished( Tp::PendingOperation* operation );
		
		// From timeout
		void contactTimeout();
		
	private:
		
		// Object, that syncronizes Tp::PendingOperation obejct signaling
		PendingOperationWaiter*		mWaiter;
		
		// For internal waiting
		QEventLoop*			mEventLoop;
		bool				mRetValue;
};

/**
 * @}
 */


#endif // _INCLUDED_TELEPATHY_CONTACTS_H
