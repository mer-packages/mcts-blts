/* functions.cpp -- min interface implementation

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
#include "AccountsTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
AccountsTest Test;

/**
 * Accounts test function. Doesn't do anything,
 * just demonstrating how to create MIN functions.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ListAccounts (MinItemParser * item)
{
	MWTS_ENTER;
        Test.ListAccounts();
	return ENOERR;
}

LOCAL int ListServices (MinItemParser * item)
{
        MWTS_ENTER;
        Test.ListServices();
        return ENOERR;
}

LOCAL int CreateAccount (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;
        //char* account_name = NULL;

        char* provider_name = NULL;
        /*
        if (ENOERR != mip_get_next_string(item, &account_name))
        {
                qCritical() << "Missing parameter: account_name";
                MWTS_LEAVE;
                return 1;
        } */

        if (ENOERR != mip_get_next_string(item, &provider_name))
        {
                qCritical() << "Missing parameter: provider name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateAccount(provider_name);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int RemoveAccount (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;
        char* account_name = NULL;

        if (ENOERR != mip_get_next_string(item, &account_name))
        {
                qCritical() << "Missing parameter: account_name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.RemoveAccount(account_name);

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
}

LOCAL int ClearAccounts (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        success = Test.ClearAccounts();

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
}

LOCAL int ListIdentities (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        success = Test.ListIdentities();

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
}

LOCAL int CreateIdentity (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* identity_name = NULL;

        if (ENOERR != mip_get_next_string(item, &identity_name))
        {
                qCritical() << "Missing parameter: identity_name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateIdentity(identity_name);

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
}

LOCAL int CreateSession (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* session_name = NULL;

        if (ENOERR != mip_get_next_string(item, &session_name))
        {
                qCritical() << "Missing parameter: session_name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateSession(session_name);

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
}

LOCAL int ClearIdentities (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        success = Test.ClearIdentities();

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;
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
        // accounts
        ENTRYTC (*list, "ListAccounts", ListAccounts);
        ENTRYTC (*list, "ListServices", ListServices);
        ENTRYTC (*list, "CreateAccount", CreateAccount);
        ENTRYTC (*list, "RemoveAccount", RemoveAccount);
        ENTRYTC (*list, "ClearAccounts", ClearAccounts);
        // sso
        ENTRYTC (*list, "ListIdentities", ListIdentities);
        ENTRYTC (*list, "ClearIdentities", ClearIdentities);
        ENTRYTC (*list, "CreateIdentity", CreateIdentity);
        ENTRYTC (*list, "CreateSession", CreateSession);





	return ENOERR;
}

