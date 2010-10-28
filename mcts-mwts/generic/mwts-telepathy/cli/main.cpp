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


const QString availableStatus( "available" );
const QString availableStatusMessage( "I'm available" );

const QString offlineStatus( "offline" );
const QString offlineStatusMessage( "I'm offline" );

const QString usage( "Usage: mwts-telepathy-cli voip <account>" );




bool ConnectVoIP( TelepathyTest* asset, const QString& account )
{
	if ( asset == NULL )
	{
		qCritical() << "NULL test asset pointer!";
		return false;
	}

	bool bret = asset->CreateAccountFromConf( account );
	if ( bret ) bret = asset->SetEnabled( true );
	Tp::SimplePresence presence;
	presence.type = Tp::ConnectionPresenceTypeAvailable;
	presence.status = availableStatus;
	presence.statusMessage = availableStatusMessage;
	if ( bret ) bret = asset->SetRequestedPresence( presence );
	if ( bret ) bret = asset->WaitForAccountConnected();
	return bret;
}


bool DisconnectVoIP( TelepathyTest* asset )
{
	if ( asset == NULL )
	{
		qCritical() << "NULL test asset pointer!";
		return false;
	}
	Tp::SimplePresence presence;
	presence.type = Tp::ConnectionPresenceTypeOffline;
	presence.status = offlineStatus;
	presence.statusMessage = offlineStatusMessage;

	bool bret = asset->SetRequestedPresence( presence );
	asset->SetEnabled( false );
	asset->RemoveAccount();
	return bret;
}


int main( int argc, char** argv )
{
	TelepathyTest telepathyTest( true );
	telepathyTest.Initialize();

	if ( argc < 2 )
	{
		qDebug() << usage;
		if ( g_pResult )
		{
			g_pResult->Write( usage );
		}
		return 0;
	}
	else
	{
		if ( QString( argv[ 1 ] ).compare( "voip" ) == 0 )
		{
			bool bret = ConnectVoIP( &telepathyTest, argv[ 2 ] );
			if ( bret == false )
			{
				qDebug() << "VoIP account " << argv[ 2 ] << " failed to connect!";
			}
			DisconnectVoIP( &telepathyTest );
		}
	}

	telepathyTest.Uninitialize();

	return 0;
}
