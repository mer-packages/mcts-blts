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
 *  @file       PendingOperationWaiter.h
 *  @brief      Helper class for synchronizing calls that return Tp::PendingOperation
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#ifndef _INCLUDED_PENDING_OPERATION_WAITER_H
#define _INCLUDED_PENDING_OPERATION_WAITER_H


#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingHandles>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/PendingAccount>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Account>
#include <QObject>
#include <QMultiHash>
#include <MwtsCommon>


#define PENDING_OPERATION_TIMEOUT 300000


class QEventLoop;


/**
 *  @class PendingOperationWaiter
 *  @brief Implements a simple helper class to synchronize any call that return a Tp::PendingOperation
 *  
 *  This class is a simple helper class for asynchronous function synchronization.
 */
class PendingOperationWaiter : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(PendingOperationWaiter)
	
	public: // API
		
		/**
		 *  @fn ~PendingOperationWaiter()
		 *  @brief Destructor for PendingOperationWaiter.
		 */
		~PendingOperationWaiter();
		
		/**
		 *  @fn static PendingOperationWaiter* instance()
		 *  @brief Static member function that returns the pointer to singleton PendingOperationWaiter object.
		 *  @return Pointer to singleton PendingOperationWaiter object.
		 */
		static PendingOperationWaiter* instance();
		
		/**
		 *  @fn bool doPendingOperation( Tp::PendingOperation* operation )
		 *  @brief Synchronizes a Tp::PendingOperation object.
		 *  @param operation Pointer to Tp::PendingOperation object
		 *  @return True, if successfull. False otherwise.
		 */
		bool doPendingOperation( Tp::PendingOperation* operation );
		
		/**
		 *  @fn bool doPendingOperation( Tp::PendingChannelRequest* operation )
		 *  @brief Synchronizes a Tp::PendingChannelRequest object.
		 *  @param operation Pointer to Tp::PendingChannelRequest object
		 *  @return True, if successfull. False otherwise.
		 */
		bool doPendingOperation( Tp::PendingChannelRequest* operation );
		
		/**
		 *  @fn bool handlesToContacts( Tp::PendingContacts* pendingContacts, QList<Tp::ContactPtr>& contacts )
		 *  @brief Synchronizes a Tp::PendingContacts object.
		 *  @param pendingContacts Pointer to Tp::PendingContacts object
		 *  @param contacts QList object, if pending contacts operation is successfull, contains requsted contacts
		 *  @return True, if successfull. False otherwise.
		 */
		bool handlesToContacts( Tp::PendingContacts* pendingContacts, QList<Tp::ContactPtr>& contacts );
		
		/**
		 *  @fn bool requestHandles( Tp::PendingHandles* pendingHandles, Tp::UIntList& handles )
		 *  @brief Synchronizes a Tp::Pendinghandles object.
		 *  @param pendingHandles Pointer to Tp::PendingHandles object
		 *  @param handles  QList object, if pending handles operation is successfull, contains requsted handles
		 *  @return True, if successfull. False otherwise.
		 */
		bool requestHandles( Tp::PendingHandles* pendingHandles, Tp::UIntList& handles );
		
		/**
		 *  @fn bool createAccount( const QString& connectionManager,
							const QString& protocol,
							const QString& displayName,
							const QVariantMap& parameters,
							const QVariantMap& properties = QVariantMap() )
		 *  @brief Synchronizes an account creation.
		 *  @param pendingHandles 	Pointer to Tp::PendingAccount object
		 *  @param account			Reference to account variable to be filled with newlt created account
		 *  @return True, if successfull. False otherwise.
		 */
		bool createAccount( Tp::PendingAccount* pendingAccount, Tp::AccountPtr& account );

		/**
		 *  @fn bool operator()( Tp::PendingOperation* operation )
		 *  @brief Parenthesis operator, that simplifies calls PendingOperationWaiter.
		 *  @param operation Pointer to Tp::PendingOperation object
		 *  @return True, if successfull. False otherwise.
		 */
		bool operator()( Tp::PendingOperation* operation )
		{
			return doPendingOperation( operation );
		}
		
		/**
		 *  @fn bool operator()( Tp::PendingChannelRequest* operation )
		 *  @brief Parenthesis operator, that simplifies calls PendingOperationWaiter.
		 *  @param operation Pointer to Tp::PendingChannelRequest object
		 *  @return True, if successfull. False otherwise.
		 */
		bool operator()( Tp::PendingChannelRequest* operation )
		{
			return doPendingOperation( operation );
		}
		
		/**
		 *  @fn QString lastErrorName() const
		 *  @brief Retuerns the name of the latest error.
		 *  @return Name of the latest error.
		 */
		QString lastErrorName() const
		{
			return mLastErrorName;
		}
		
		/**
		 *  @fn QString lastErrorMessage() const
		 *  @brief Returns the latest error message.
		 *  @return Latest error message.
		 */
		QString lastErrorMessage() const
		{
			return mLastErrorMessage;
		}
		
	private Q_SLOTS:
		void onOperationFinished( Tp::PendingOperation* operation );
		void onPendingContactsFinished( Tp::PendingOperation* operation );
		void onPendingHandlesFinished( Tp::PendingOperation* operation );
		void onPendingAccountFinished( Tp::PendingOperation* operation );

	private: // Private type declarations
		/**
		*  @class PendingOperationWaiter::PendingOperationData
		*  @brief Data container class for PendingOperation synchronization
		*  
		*  This class is a data contaioner class used in Tp::PendingOperation synchronization.
		*/
		class PendingOperationData
		{
			public:
				PendingOperationData()
				: mOperation( NULL ), mLoop( NULL ), mError( true )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				PendingOperationData(	Tp::PendingOperation* operation,
							QEventLoop* loop )
				: mOperation( operation ), mLoop( loop ), mError( true )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				virtual ~PendingOperationData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				//! == operator for the QHash
				bool operator==( const PendingOperationData& rhs )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
					return ( mLoop == rhs.mLoop );
				}
				
				Tp::PendingOperation* mOperation;
				QEventLoop* mLoop;
				bool mError;
				QString mErrorName;
				QString mErrorMessage;
		};
		
		/**
		*  @class PendingOperationWaiter::PendingContactsData
		*  @brief Data container class for PendingContacts synchronization
		*  
		*  This class is a data container class used in Tp::PendingContacts synchronization.
		*/
		class PendingContactsData : public PendingOperationData
		{
			public:
				PendingContactsData()
				: PendingOperationData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				PendingContactsData(	Tp::PendingOperation* operation,
							QEventLoop* loop )
				: PendingOperationData( operation, loop )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				virtual ~PendingContactsData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				//! == operator for the QHash
				bool operator==( const PendingContactsData& rhs )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
					return ( mLoop == rhs.mLoop );
				}

				QList<Tp::ContactPtr> mContacts;
		};
		
		/**
		*  @class PendingOperationWaiter::PendingHandlesData
		*  @brief Data container class for PendingHandles synchronization
		*  
		*  This class is a data container class used in Tp::PendingHandles synchronization.
		*/
		class PendingHandlesData : public PendingOperationData
		{
			public:
				PendingHandlesData()
				: PendingOperationData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				PendingHandlesData(	Tp::PendingOperation* operation,
							QEventLoop* loop )
				: PendingOperationData( operation, loop )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				virtual ~PendingHandlesData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}
				
				//! == operator for the QHash
				bool operator==( const PendingHandlesData& rhs )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
					return ( mLoop == rhs.mLoop );
				}

				Tp::UIntList mHandles;
		};

		/**
		*  @class PendingOperationWaiter::PendingAccountData
		*  @brief Data container class for PendingAccount synchronization
		*
		*  This class is a data container class used in Tp::PendingAccount synchronization.
		*/
		class PendingAccountData : public PendingOperationData
		{
			public:
				PendingAccountData()
				: PendingOperationData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}

				PendingAccountData(	Tp::PendingOperation* operation,
							QEventLoop* loop )
				: PendingOperationData( operation, loop )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}

				virtual ~PendingAccountData()
				{
					MWTS_ENTER;
					MWTS_LEAVE;
				}

				//! == operator for the QHash
				bool operator==( const PendingAccountData& rhs )
				{
					MWTS_ENTER;
					MWTS_LEAVE;
					return ( mLoop == rhs.mLoop );
				}

				Tp::AccountPtr mAccount;
		};


	private: // Data
		PendingOperationWaiter();
		
		void setLastError(	const QString& errorName = QString(),
					const QString& errorMsg = QString() );
		
		
		static PendingOperationWaiter* mInstance;
		
		QMultiHash<Tp::PendingOperation*, PendingOperationData*> mPendingOperations;
		
		QString mLastErrorName;
		QString mLastErrorMessage;
};

/**
 * @}
 */

#endif // _INCLUDED_PENDING_OPERATION_WAITER_H


