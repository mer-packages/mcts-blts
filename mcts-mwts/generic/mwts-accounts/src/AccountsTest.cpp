/* AccountsTest.cpp -- Accounts test asset implementation

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
#include "AccountsTest.h"

/**
 * Constructor for template test class
 */
AccountsTest::AccountsTest() 
{
    MWTS_ENTER;

    qDebug() << "Creating the manager";
    manager = new Manager();

    qDebug() << "Creating the ssoCLient";
    ssoClient = new SignonClient(this);

    m_bSuccess = false;

    // set logging on
    g_pLog->EnableDebug(true);
    g_pLog->EnableTrace(true);

    MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
AccountsTest::~AccountsTest()
{
    MWTS_ENTER;

    qDebug() << "Destroying objects";

    if(manager)
    {
        qDebug() << "Destroyed manager.";
        delete manager;
        manager = NULL;
    }

    if(ssoClient)
    {
        qDebug() << "Destroyed ssoClient.";
        delete ssoClient;
        ssoClient = NULL;
    }

    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void AccountsTest::OnInitialize()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void AccountsTest::OnUninitialize()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

bool AccountsTest::ListAccounts()
{
    MWTS_ENTER;

    qDebug() << "Querying accounts from the manager...";
    AccountIdList list = manager->accountList(NULL);

    qDebug() << "Got the list";
    qDebug() << "Listing available accounts on device";

    const int size = list.size();
    for (int i = 0; i < size; ++i)
    {
        qDebug ("///////////////////////////");
        qDebug ( "Account: %u", list.at(i));
        Account* acc = manager->account(list.at(i));
        if (acc!=NULL)
        {

            qDebug() << "Id: " << acc->id();
            qDebug() << "DisplayName       : " << acc->displayName();
            qDebug() << "Provider is       : " << acc->providerName();
            qDebug() << "SSO credentials Id: " << acc->credentialsId();

            qDebug() << "Account supports following services: ";

            ServiceList list = acc->services(NULL);
            for (int i = 0; i < list.size(); ++i)
            {
                qDebug () << "Service nro:" << i;
                Service* service = manager->service(list.at(i)->name());
                if (!service)
                {
                    qDebug () << "---> Service     : " << service->name();
                    qDebug () << "     DisplayName : " << service->displayName();
                    qDebug () << "     Type        : " << service->serviceType();
                    qDebug () << "     Provider    : " << service->provider();
                }
                service=NULL;
            }
        }
        acc=NULL;
    }

    MWTS_LEAVE;
    return true;
}

// lists all the available services from all providers
bool AccountsTest::ListServices()
{
    MWTS_ENTER;

    ServiceList list = manager->serviceList(NULL);

    qDebug() << "Listing available services on device";
    const int size = list.size();
    for (int i = 0; i < size; ++i)
    {
        qDebug () << "Service nro:" << i;
        Service* service = manager->service(list.at(i)->name());
        if (service!=NULL)
        {
            qDebug ("   /////");
            qDebug () << "   Service: " << service->name();
            qDebug () << "   DisplayName: " << service->displayName();
            qDebug () << "   Type: " << service->serviceType();
            qDebug () << "   Provider: " << service->provider();
        }
        service=NULL;
    }

    MWTS_LEAVE;
    return true;
}

bool AccountsTest::CreateAccount(const QString provider_name)
{
    MWTS_ENTER;

    qDebug() << "Creating SSO-credentials...";

    int credId = 0;

    credId = ssoClient->CreateIdentity(provider_name);

    if(credId == 0)
    {
        qCritical() << "sso credential creation failed, aborting";
        return false;
    }

    qDebug() << "Id of newly created SSO credential is: " << credId;

    qDebug() << "Checking conf-file for: " << provider_name;
    QString provider = g_pConfig->value(provider_name + "/provider").toString();
    QString username = g_pConfig->value(provider_name + "/username").toString();
    QString password = g_pConfig->value(provider_name + "/password").toString();
    QString displayname = g_pConfig->value(provider_name + "/displayname").toString();

    if(provider.isNull())
    {
        qDebug() << "Found no such provider: " << provider_name << ". Aborting..";
        return false;
    }

    qDebug() << "Adding account: " << username << " with provider: " << provider;

    Account* account = manager->createAccount(provider);
    account->sync();
    if(account == NULL)
    {
        qDebug() << "Failed! Quitting...";
        return false;
    }

    qDebug() << "Account was created. Setting account variables...";
    account->setDisplayName(username);
    account->setCredentialsId(credId);

    account->setEnabled(true);
    account->sync();

    qDebug() << "DisplayName        : " << account->displayName();
    qDebug() << "ProviderName       : " << account->providerName();
    qDebug() << "SSO Credentials id : " << account->credentialsId();


    qDebug() << "Enabling accounts services...";

    ServiceList list = account->services(NULL);

    const int size = list.size();
    for (int i = 0; i < list.size(); ++i)
    {
        qDebug () << "Service nro:" << i;
        Service* service = manager->service(list.at(i)->name());

        if(service->name() == "nmem-gmail")
        {
            account->selectService(service);
            account->setEnabled(false);
            account->sync();
        }
        else
        {
            account->selectService(service);
            account->setEnabled(true);
            account->sync();
        }
        service=NULL;
    }

    // clean up
    delete account;
    account == NULL;

    MWTS_LEAVE;
    return true;
}

bool AccountsTest::RemoveAccount(const QString strName)
{
    MWTS_ENTER;

    qDebug() << "Trying to remove account: " << strName;

    AccountIdList acc_list = manager->accountList(NULL);

    qDebug() << "Listing available accounts on device";
    const int size = acc_list.size();
    for (int i = 0; i < size; ++i)
    {
        Account* acc = manager->account(acc_list.at(i));

        if(acc->displayName() == strName)
        {
            acc->remove();
            acc->sync();
            qDebug() << "Found account: " << strName << " and removed it.";
            acc=NULL;
            return true;
        }
        acc=NULL;
    }

    MWTS_LEAVE;
    return true;
}

bool AccountsTest::ClearAccounts()
{
    MWTS_ENTER;

    qDebug() << "Trying to clear all accounts.";

    AccountIdList acc_list = manager->accountList(NULL);

    qDebug() << "Listing available accounts on device";
    const int size = acc_list.size();
    for (int i = 0; i < size; ++i)
    {
        Account* acc = manager->account(acc_list.at(i));
        acc->remove();
        acc->sync();
        acc=NULL;
    }

    MWTS_LEAVE;
    return true;
}

/** SSO FUNCTIONS **/

bool AccountsTest::ListIdentities()
{
    MWTS_ENTER;

    qDebug() << "Listing identities from sso...";

    return ssoClient->queryIdents();
    MWTS_LEAVE;
}

bool AccountsTest::ClearIdentities()
{
    MWTS_ENTER;

    qDebug() << "Clearing identities from sso...";

    bool retval = false;
    retval = ssoClient->clearCredentials();

    return retval;
    MWTS_LEAVE;
}



bool AccountsTest::CreateIdentity(const QString strName)
{
    MWTS_ENTER;

    bool retval = false;
    retval = ssoClient->CreateIdentity(strName);

    return retval;
    MWTS_LEAVE;
}

bool AccountsTest::CreateSession(const QString strName)
{
    MWTS_ENTER;

    bool retval = false;

    qDebug() << "Creating account";

    retval = CreateAccount(strName);
    retval = ssoClient->CreateSession(strName);

    //retval = ssoClient->CreateIdentity(strName);
    //retval = ssoClient->CreateSession(strName);

    return retval;
    MWTS_LEAVE;
}

void AccountsTest::slotAccountCreated(Accounts::AccountId id)
{
    this->Stop();
    Q_UNUSED(id);
    qDebug() << "Account created";
}


