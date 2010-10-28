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
#include "PendingOperationWaiter.h"

// initialise the singleton pointer
PendingOperationWaiter* PendingOperationWaiter::mInstance = NULL;


PendingOperationWaiter* PendingOperationWaiter::instance()
{
	MWTS_ENTER;
	if ( mInstance == NULL )
	{
		MWTS_DEBUG("Creating helper singleton...");
		mInstance = new PendingOperationWaiter();
	}
	qDebug() << "mInstance == " << mInstance;
	MWTS_LEAVE;
	return mInstance;
}


PendingOperationWaiter::PendingOperationWaiter()
: QObject( NULL )
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

PendingOperationWaiter::~PendingOperationWaiter()
{
	MWTS_ENTER;
	mInstance = NULL;
	MWTS_LEAVE;
}



bool PendingOperationWaiter::doPendingOperation( Tp::PendingOperation* operation )
{
	MWTS_ENTER;
	qDebug() << "this == " << this;
	bool ok = false;
	
	if ( !operation->isFinished() )
	{
		QEventLoop loop( this );
		
		// We might have multiple event loops waiting for the same
		// PendingOperation to finish. We need to store the operation info so
		// we can quit the loop and update the status in slotOperationFinished.
		qDebug() << "Creating new data object for operation...";
		PendingOperationData data( operation, &loop );
		qDebug() << "Storing the object...";
		mPendingOperations.insert( operation, &data );
		
		// handle the finished operation in slotOperationFinished
		// NOTE: connecting Tp::PendingOperation::finished with QEventLoop::quit
		// might cause aPendingOperation to be destroyed by the time the loop
		// actually exits...
		qDebug() << "Connecting signals...";
		connect( operation,
			SIGNAL(finished(Tp::PendingOperation*)),
			this,
			SLOT(onOperationFinished(Tp::PendingOperation*)),
			Qt::DirectConnection );	// Must be direct. Tp::PendingOperation
							// is automatically deleted after the
							// finished() signal is emitted!
		
		// exit the loop after a timeout
		// NOTE: This is just a backup if we fail to receive the "finished"
		//       signal for some reason.
		qDebug() << "Waiting for the operation to finish...";
		QTimer::singleShot(PENDING_OPERATION_TIMEOUT, &loop, SLOT(quit()));
		
		loop.exec();
		
		// NOTE: aPendingOperation is not a valid pointer anymore!
		MWTS_DEBUG(QString( "error: %1" ).arg( data.mError ) );
		ok = !data.mError;
		setLastError( data.mErrorName, data.mErrorMessage );
		
		// cleanup after the operation has finished
		int count = mPendingOperations.remove( operation, &data );
		Q_ASSERT( count == 1 );
		Q_UNUSED( count );
	}
	else
	{
		MWTS_DEBUG( "Pending opweration was already finished!" );
		MWTS_DEBUG( QString( "%1: %2" ).arg( operation->errorName() ).arg( operation->errorMessage() ) );
		ok = operation->isValid();
		setLastError(	operation->errorName(),
				operation->errorMessage() );
	}

	MWTS_DEBUG( QString( "Pending operation done: %1" ).arg( ok ) );
	MWTS_LEAVE;
	return ok;
}


bool PendingOperationWaiter::doPendingOperation( Tp::PendingChannelRequest* operation )
{
	MWTS_ENTER;
	bool ok = false;
	
	if ( !operation->isFinished() )
	{
		QEventLoop loop( this );
		
		// We might have multiple event loops waiting for the same
		// PendingOperation to finish. We need to store the operation info so
		// we can quit the loop and update the status in slotOperationFinished.
		PendingOperationData data( operation, &loop );
		mPendingOperations.insert( operation, &data );
		
		// handle the finished operation in slotOperationFinished
		// NOTE: connecting Tp::PendingOperation::finished with QEventLoop::quit
		// might cause aPendingOperation to be destroyed by the time the loop
		// actually exits...
		connect( operation,
			SIGNAL(finished(Tp::PendingOperation*)),
			this,
			SLOT(onOperationFinished(Tp::PendingOperation*)),
			Qt::DirectConnection );	// Must be direct. Tp::PendingOperation
							// is automatically deleted after the
							// finished() signal is emitted!
		
		// exit the loop after a timeout
		// NOTE: This is just a backup if we fail to receive the "finished"
		//       signal for some reason.
		QTimer::singleShot(PENDING_OPERATION_TIMEOUT, &loop, SLOT(quit()));
		
		loop.exec();
		
		// NOTE: aPendingOperation is not a valid pointer anymore!
		MWTS_DEBUG(QString( "error: %1" ).arg( data.mError ) );
		ok = !data.mError;
		setLastError( data.mErrorName, data.mErrorMessage );
		
		// cleanup after the operation has finished
		int count = mPendingOperations.remove( operation, &data );
		Q_ASSERT( count == 1 );
		Q_UNUSED( count );
	}
	else
	{
		MWTS_DEBUG( "Pending opweration was already finished!" );
		MWTS_DEBUG( QString( "%1: %2" ).arg( operation->errorName() ).arg( operation->errorMessage() ) );
		ok = operation->isValid();
		setLastError(	operation->errorName(),
				operation->errorMessage() );
	}

	MWTS_DEBUG( QString( "Pending operation done: %1" ).arg( ok ) );
	MWTS_LEAVE;
	return ok;
}


bool PendingOperationWaiter::handlesToContacts( Tp::PendingContacts* pendingContacts, QList<Tp::ContactPtr>& contacts )
{
	MWTS_ENTER;
	bool ok = false;
	
	if ( !pendingContacts->isFinished() )
	{
		QEventLoop loop( this );
		
		// We might have multiple event loops waiting for the same
		// PendingOperation to finish. We need to store the operation info so
		// we can quit the loop and update the status in slotOperationFinished.
		PendingContactsData data( pendingContacts, &loop );
		mPendingOperations.insert( pendingContacts, &data );
		
		// handle the finished operation in slotOperationFinished
		// NOTE: connecting Tp::PendingOperation::finished with QEventLoop::quit
		// might cause aPendingOperation to be destroyed by the time the loop
		// actually exits...
		connect( pendingContacts,
			SIGNAL(finished(Tp::PendingOperation*)),
			this,
			SLOT(onPendingContactsFinished(Tp::PendingOperation*)),
			Qt::DirectConnection );	// Must be direct. Tp::PendingOperation
							// is automatically deleted after the
							// finished() signal is emitted!
		
		// exit the loop after a timeout
		// NOTE: This is just a backup if we fail to receive the "finished"
		//       signal for some reason.
		QTimer::singleShot(PENDING_OPERATION_TIMEOUT, &loop, SLOT(quit()));
		
		loop.exec();
		
		// NOTE: aPendingOperation is not a valid pointer anymore!
		MWTS_DEBUG(QString( "error: %1" ).arg( data.mError ) );
		ok = !data.mError;
		setLastError( data.mErrorName, data.mErrorMessage );
		
		contacts = data.mContacts;
		
		// cleanup after the operation has finished
		int count = mPendingOperations.remove( pendingContacts, &data );
		Q_ASSERT( count == 1 );
		Q_UNUSED( count );
	}
	else
	{
		MWTS_DEBUG( "Pending opweration was already finished!" );
		MWTS_DEBUG( QString( "%1: %2" ).arg( pendingContacts->errorName() ).arg( pendingContacts->errorMessage() ) );
		ok = pendingContacts->isValid();
		setLastError(	pendingContacts->errorName(),
				pendingContacts->errorMessage() );
	}

	MWTS_DEBUG( QString( "Pending operation done: %1" ).arg( ok ) );
	MWTS_LEAVE;
	return ok;
}


bool PendingOperationWaiter::requestHandles( Tp::PendingHandles* pendingHandles, Tp::UIntList& handles )
{
	MWTS_ENTER;
	bool ok = false;
	
	if ( !pendingHandles->isFinished() )
	{
		QEventLoop loop( this );
		
		// We might have multiple event loops waiting for the same
		// PendingOperation to finish. We need to store the operation info so
		// we can quit the loop and update the status in slotOperationFinished.
		PendingHandlesData data( pendingHandles, &loop );
		mPendingOperations.insert( pendingHandles, &data );
		
		// handle the finished operation in slotOperationFinished
		// NOTE: connecting Tp::PendingOperation::finished with QEventLoop::quit
		// might cause aPendingOperation to be destroyed by the time the loop
		// actually exits...
		connect( pendingHandles,
			SIGNAL(finished(Tp::PendingOperation*)),
			this,
			SLOT(onPendingHandlesFinished(Tp::PendingOperation*)),
			Qt::DirectConnection );	// Must be direct. Tp::PendingOperation
							// is automatically deleted after the
							// finished() signal is emitted!
		
		// exit the loop after a timeout
		// NOTE: This is just a backup if we fail to receive the "finished"
		//       signal for some reason.
		QTimer::singleShot(PENDING_OPERATION_TIMEOUT, &loop, SLOT(quit()));
		
		loop.exec();
		
		// NOTE: aPendingOperation is not a valid pointer anymore!
		MWTS_DEBUG(QString( "error: %1" ).arg( data.mError ) );
		ok = !data.mError;
		setLastError( data.mErrorName, data.mErrorMessage );
		
		handles = data.mHandles;
		
		// cleanup after the operation has finished
		int count = mPendingOperations.remove( pendingHandles, &data );
		Q_ASSERT( count == 1 );
		Q_UNUSED( count );
	}
	else
	{
		MWTS_DEBUG( "Pending operation was already finished!" );
		MWTS_DEBUG( QString( "%1: %2" ).arg( pendingHandles->errorName() ).arg( pendingHandles->errorMessage() ) );
		ok = pendingHandles->isValid();
		setLastError(	pendingHandles->errorName(),
				pendingHandles->errorMessage() );
	}

	MWTS_DEBUG( QString( "Pending operation done: %1" ).arg( ok ) );
	MWTS_LEAVE;
	return ok;
}


bool PendingOperationWaiter::createAccount( Tp::PendingAccount* pendingAccount, Tp::AccountPtr& account )
{
	MWTS_ENTER;
	bool ok = false;

	if ( !pendingAccount->isFinished() )
	{
		QEventLoop loop( this );

		// We might have multiple event loops waiting for the same
		// PendingOperation to finish. We need to store the operation info so
		// we can quit the loop and update the status in slotOperationFinished.
		PendingAccountData data( pendingAccount, &loop );
		mPendingOperations.insert( pendingAccount, &data );

		// handle the finished operation in slotOperationFinished
		// NOTE: connecting Tp::PendingOperation::finished with QEventLoop::quit
		// might cause aPendingOperation to be destroyed by the time the loop
		// actually exits...
		connect( pendingAccount,
			SIGNAL(finished(Tp::PendingOperation*)),
			this,
			SLOT(onPendingAccountFinished(Tp::PendingOperation*)),
			Qt::DirectConnection );	// Must be direct. Tp::PendingOperation
							// is automatically deleted after the
							// finished() signal is emitted!

		// exit the loop after a timeout
		// NOTE: This is just a backup if we fail to receive the "finished"
		//       signal for some reason.
		QTimer::singleShot(PENDING_OPERATION_TIMEOUT, &loop, SLOT(quit()));

		loop.exec();

		// NOTE: aPendingOperation is not a valid pointer anymore!
		MWTS_DEBUG(QString( "error: %1" ).arg( data.mError ) );
		ok = !data.mError;
		setLastError( data.mErrorName, data.mErrorMessage );

		account = data.mAccount;

		// cleanup after the operation has finished
		int count = mPendingOperations.remove( pendingAccount, &data );
		Q_ASSERT( count == 1 );
		Q_UNUSED( count );
	}
	else
	{
		MWTS_DEBUG( "Pending operation was already finished!" );
		MWTS_DEBUG( QString( "%1: %2" ).arg( pendingAccount->errorName() ).arg( pendingAccount->errorMessage() ) );
		ok = pendingAccount->isValid();
		setLastError(	pendingAccount->errorName(),
				pendingAccount->errorMessage() );
	}

	MWTS_DEBUG( QString( "Pending operation done: %1" ).arg( ok ) );
	MWTS_LEAVE;
	return ok;
}



void PendingOperationWaiter::onOperationFinished( Tp::PendingOperation* operation )
{
	MWTS_ENTER;
	
	if ( !operation->isFinished() )
	{
		MWTS_DEBUG( "Pending operation was not finished!" );
	}
	
	if ( operation->isError() )
	{
		MWTS_DEBUG( "Pending operation error!" );
		MWTS_DEBUG( operation->errorName() );
		MWTS_DEBUG( operation->errorMessage() );
	}
	
	if ( operation->isValid() )
	{
		MWTS_DEBUG( "Pending operation OK." );
	}
	
	// get all items for this pending operation and quit the loops
	QList<PendingOperationData*> ops = mPendingOperations.values( operation );
	
	foreach ( PendingOperationData *data, ops )
	{
		data->mError = operation->isError();
		if ( data->mError )
		{
			data->mErrorName = operation->errorName();
			data->mErrorMessage = operation->errorMessage();
		}
		QTimer::singleShot( 0, data->mLoop, SLOT(quit()));
	}
	MWTS_LEAVE;
}


void PendingOperationWaiter::onPendingContactsFinished( Tp::PendingOperation* operation )
{
	MWTS_ENTER;
	
	if ( !operation->isFinished() )
	{
		MWTS_DEBUG( "Pending operation was not finished!" );
	}
	
	if ( operation->isError() )
	{
		MWTS_DEBUG( "Pending operation error!" );
		MWTS_DEBUG( operation->errorName() );
		MWTS_DEBUG( operation->errorMessage() );
	}
	
	if ( operation->isValid() )
	{
		MWTS_DEBUG( "Pending operation OK." );
	}
	
	// get all items for this pending operation and quit the loops
	QList<PendingOperationData*> ops = mPendingOperations.values( operation );
	
	foreach ( PendingOperationData *data, ops )
	{
		data->mError = operation->isError();
		if ( data->mError )
		{
			data->mErrorName = operation->errorName();
			data->mErrorMessage = operation->errorMessage();
		}
		Tp::PendingContacts* pc = qobject_cast<Tp::PendingContacts*>( operation );
		PendingContactsData* pcd = static_cast<PendingContactsData*>( data );
		pcd->mContacts = pc->contacts();
		foreach ( Tp::ContactPtr contact, pcd->mContacts )
		{
			if ( contact.isNull() )
			{
				qDebug() << "Contact is NULL.";
			}
			else
			{
				qDebug() << "Contact id: " << contact->id();
			}
		}
		QTimer::singleShot( 0, data->mLoop, SLOT(quit()));
	}
	MWTS_LEAVE;
}


void PendingOperationWaiter::onPendingHandlesFinished( Tp::PendingOperation* operation )
{
	MWTS_ENTER;
	
	if ( !operation->isFinished() )
	{
		MWTS_DEBUG( "Pending operation was not finished!" );
	}
	
	if ( operation->isError() )
	{
		MWTS_DEBUG( "Pending operation error!" );
		MWTS_DEBUG( operation->errorName() );
		MWTS_DEBUG( operation->errorMessage() );
	}
	
	if ( operation->isValid() )
	{
		MWTS_DEBUG( "Pending operation OK." );
	}
	
	// get all items for this pending operation and quit the loops
	QList<PendingOperationData*> ops = mPendingOperations.values( operation );
	
	foreach ( PendingOperationData *data, ops )
	{
		data->mError = operation->isError();
		if ( data->mError )
		{
			data->mErrorName = operation->errorName();
			data->mErrorMessage = operation->errorMessage();
		}
		Tp::PendingHandles* ph = qobject_cast<Tp::PendingHandles*>( operation );
		PendingHandlesData* phd = static_cast<PendingHandlesData*>( data );
		phd->mHandles = ph->handles().toList();
		foreach ( uint handle, phd->mHandles )
		{
			qDebug() << "Handle received: " << handle;
		}
		QTimer::singleShot( 0, data->mLoop, SLOT(quit()));
	}
	MWTS_LEAVE;
}


void PendingOperationWaiter::onPendingAccountFinished( Tp::PendingOperation* operation )
{
	MWTS_ENTER;

	if ( !operation->isFinished() )
	{
		MWTS_DEBUG( "Pending operation was not finished!" );
	}

	if ( operation->isError() )
	{
		MWTS_DEBUG( "Pending operation error!" );
		MWTS_DEBUG( operation->errorName() );
		MWTS_DEBUG( operation->errorMessage() );
	}

	if ( operation->isValid() )
	{
		MWTS_DEBUG( "Pending operation OK." );
	}

	// get all items for this pending operation and quit the loops
	QList<PendingOperationData*> ops = mPendingOperations.values( operation );

	foreach ( PendingOperationData *data, ops )
	{
		data->mError = operation->isError();
		if ( data->mError )
		{
			data->mErrorName = operation->errorName();
			data->mErrorMessage = operation->errorMessage();
		}
		Tp::PendingAccount* pa = qobject_cast<Tp::PendingAccount*>( operation );
		PendingAccountData* pad = static_cast<PendingAccountData*>( data );
		pad->mAccount = pa->account();

		qDebug() << "Account " << pad->mAccount->displayName()
				 << " is valid? " <<  pad->mAccount->isValidAccount();

		QTimer::singleShot( 0, data->mLoop, SLOT(quit()));
	}
	MWTS_LEAVE;
}


void PendingOperationWaiter::setLastError( const QString& errorName, const QString& errorMsg )
{
	MWTS_ENTER;
	MWTS_DEBUG( QString( "%1 - %2" ).arg( errorName ).arg( errorMsg ) );
	mLastErrorName = errorName;
	mLastErrorMessage = errorMsg;
	MWTS_LEAVE;
}

