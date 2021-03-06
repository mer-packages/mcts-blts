/* main.cpp -- Implementation of command line interface.

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
//QTM_USE_NAMESPACE

#include "stable.h"
#include "MessagingTest.h"

int main( int argc, char* argv[] )
{
	MessagingTest test;

	test.Initialize();

	if ( argc < 2 )
	{
		test.QueryAllAccounts();
	}
	else
	{
		if ( QString::compare( argv[ 1 ], "send_email" ) == 0 )
		{
			if ( argc < 4 )
			{
				qCritical() << "Usage: mwts-messaging-cli send_email <subect> <body>";
				test.Uninitialize();
				return 1;
			}
			test.SetMessageType( QMessage::Email );
			test.SendEmail( argv[ 2 ], argv[ 3 ] );
		}
	}
	test.Uninitialize();

	return 0;
}
