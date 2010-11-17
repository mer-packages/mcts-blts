/* functions.cpp -- Test assets MIN functions

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
#include "interface.h"
#include "MessagingTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
MessagingTest Test;


/**
 * Messaging test function. Doesn't do anything,
 * just demonstrating how to create MIN functions.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int QueryAllAccounts (MinItemParser * item)
{
	MWTS_ENTER;
	Test.QueryAllAccounts();
	return ENOERR;
}


/**
 * Messaging test function. Doesn't do anything,
 * just demonstrating how to create MIN functions.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int PreferredCharsets (MinItemParser * item)
{
	MWTS_ENTER;
	Test.PreferredCharsets();
	return ENOERR;
}


/**
 * Send "Hello, World!!!" to given number
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SendSMS( MinItemParser * item )
{
	MWTS_ENTER;

	char *arg = NULL;
	char *length = NULL;
	char *encoding = NULL;
	char *mib = NULL;
	int iret = ENOERR;

	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "length") == 0)
		{
			iret = mip_get_next_string(item, &length);
			if(iret != ENOERR)
				return iret;
		}
		else if(strcmp(arg, "encoding") == 0)
		{
			iret = mip_get_next_string(item, &encoding);
			if(iret != ENOERR)
				return iret;
		}
		if(strcmp(arg, "mib") == 0)
		{
			iret = mip_get_next_string(item, &mib);
			if(iret != ENOERR)
				return iret;
		}
	}

	if ( length == NULL )
	{
		if ( encoding != NULL )
		{
			Test.SendSMS( "data-file-message-default", encoding );
		}
		else if ( mib != NULL )
		{
			Test.SendSMS( "data-file-message-default", atoi( mib ) );
		}
		else
		{
			Test.SendSMS( "data-file-message-default" );
		}
	}
	else if ( strcmp( length, "0" ) == 0 )
	{
		if ( encoding != NULL )
		{
			Test.SendSMS( "data-file-message-0", encoding );
		}
		else if ( mib != NULL )
		{
			Test.SendSMS( "data-file-message-0", atoi( mib ) );
		}
		else
		{
			Test.SendSMS( "data-file-message-0" );
		}
	}
	else if ( strcmp( length, "140" ) == 0 )
	{
		if ( encoding != NULL )
		{
			Test.SendSMS( "data-file-message-140", encoding );
		}
		else if ( mib != NULL )
		{
			Test.SendSMS( "data-file-message-140", atoi( mib ) );
		}
		else
		{
			Test.SendSMS( "data-file-message-140" );
		}
	}
	else if ( strcmp( length, "180" ) == 0 )
	{
		if ( encoding != NULL )
		{
			Test.SendSMS( "data-file-message-180", encoding );
		}
		else if ( mib != NULL )
		{
			Test.SendSMS( "data-file-message-180", atoi( mib ) );
		}
		else
		{
			Test.SendSMS( "data-file-message-180" );
		}
	}
	else if ( strcmp( length, "380" ) == 0 )
	{
		if ( encoding != NULL )
		{
			Test.SendSMS( "data-file-message-380", encoding );
		}
		else if ( mib != NULL )
		{
			Test.SendSMS( "data-file-message-380", atoi( mib ) );
		}
		else
		{
			Test.SendSMS( "data-file-message-380" );
		}
	}
	else
	{
		Test.SendSMS( "data-file-message-default" );
	}

	return ENOERR;
}


/**
 * Wait for incoming messages.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int WaitForIncomingMessage (MinItemParser * item)
{
	MWTS_ENTER;
	Test.WaitForIncomingMessage();
	return ENOERR;
}


/**
 * Write send and receive latencies to report
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ReportLatencies (MinItemParser * item)
{
	MWTS_ENTER;
	Test.ReportLatencies();
	return ENOERR;
}


/**
 * Function for MIN to gather our test case functions.
 * @param list	Functio pointer list, out
 * @return		ENOERR
 */
int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

	ENTRYTC (*list, "QueryAllAccounts", QueryAllAccounts);
	ENTRYTC (*list, "PreferredCharsets", PreferredCharsets);
	ENTRYTC (*list, "SendSMS", SendSMS);
	ENTRYTC (*list, "WaitForIncomingMessage", WaitForIncomingMessage);
	ENTRYTC (*list, "ReportLatencies", ReportLatencies);
	return ENOERR;
}

