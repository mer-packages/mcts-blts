/* AccountsTest.h -- Accounts test asset definition

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

#ifndef _INCLUDED_TEMPLATE_TEST_H
#define _INCLUDED_TEMPLATE_TEST_H


#include <MwtsCommon>
#include <QtCore>
#include "signonclient.h"


#include <Accounts/manager.h>

using namespace Accounts;

class AccountsTest : public MwtsTest
{
	Q_OBJECT
public:

	/**
	 * Constructor for Accounts test class
	 */
	AccountsTest();

	/**
	 * Destructor for template test class
	 */
	virtual ~AccountsTest();

	/**
	 * Overridden functions for MwtsTest class
	 * OnInitialize is called before test execution
	 */
	void OnInitialize();

	/**
	 * Overridden functions for MwtsTest class
	 * OnUninitialize is called after test execution
	 */
	void OnUninitialize();

	/**
         * List added accounts
	 */
        bool ListAccounts();

        /**
         * List available services
         */
        bool ListServices();

        /**
         * Create account
         */
        bool CreateAccount(const QString provider_name);

        /**
         * Remove account
         */
        bool RemoveAccount(const QString strName);

        /**
         * Removes all accounts
         */
        bool ClearAccounts();

        /**
         * Lists identities / credentials from sso db
         */
        bool ListIdentities();

        /**
         * Creates identity/credential
         */
        bool CreateIdentity(const QString strName);

        /**
         * Open session (sign in with chosen service. Identity needs to be created first)
         */
        bool CreateSession(const QString strName);

        /**
         * Clears all credentials from sso db
         */
        bool ClearIdentities();


private:
        bool m_bSuccess;

        SignonClient *ssoClient;

        /*
         * Accounts Manager
         */
        Manager* manager;
        
protected slots:
        void slotAccountCreated(AccountId id);
};

#endif //#ifndef _INCLUDED_TEMPLATE_TEST_H

