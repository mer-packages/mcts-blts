
/* signonclient.cpp -- Signon asset functionality

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


#include <QEventLoop>
#include "signonclient.h"

using namespace SignOn;


SignonClient::SignonClient(MwtsTest *testObj) : m_testObj(testObj),
                                                m_identity(NULL),
                                                m_info(NULL),
                                                m_session(NULL)

{
    MWTS_ENTER;

    // set logging on
    g_pLog->EnableDebug(true);
    g_pLog->EnableTrace(true);

    m_service = new AuthService();

    // test verdict
    m_bSuccess = false;

    connect(m_service, SIGNAL(methodsAvailable(const QStringList&)),
        this, SLOT(methodsAvailable(const QStringList&)));

    connect(m_service, SIGNAL(identities(const QList<SignOn::IdentityInfo>& )),
        this, SLOT(identities(const QList<SignOn::IdentityInfo>& )));

    MWTS_LEAVE;
}

SignonClient::~SignonClient()
{
    MWTS_ENTER;
    qDebug() << "Destroying objects";

    if(m_service)
    {
        qDebug() << "Destroying m_service";
        delete m_service;
        m_service = NULL;
    }

    if(m_identity)
    {
        qDebug() << "Destroying m_identity";
        delete m_identity;
        m_identity = NULL;
    }

    if(m_info)
    {
        qDebug() << "Destroying m_info";
        delete m_info;
        m_service = NULL;
    }

    /*
    if(m_session)
    {
        qDebug() << "Destroying m_session";
        delete m_session;
        m_session = NULL;
    } */

    MWTS_LEAVE;
}




bool SignonClient::clearCredentials()
{
   if(!m_service)
   {
       qCritical() << "No auth service, aborting";
       return false;
   }

   connect(m_service, SIGNAL(cleared()),
       this, SLOT(clear()));

   m_service->clear();
   m_testObj->Start();

   qDebug() << "Cleared: " << m_bSuccess;
   return m_bSuccess;
}

bool SignonClient::queryIdents()
{
    MWTS_ENTER;

    qDebug() << "Querying identities...";
    m_service->queryIdentities();
    m_testObj->Start();

    qDebug() << "Querying methods...";
    m_service->queryMethods();
    m_testObj->Start();

    return m_bSuccess;
    MWTS_LEAVE;
}


int SignonClient::CreateIdentity(const QString provider)
{
    MWTS_ENTER;

    QString username = g_pConfig->value(provider + "/username").toString();
    QString method = g_pConfig->value(provider + "/provider").toString();
    QString password = g_pConfig->value(provider + "/password").toString();
    QString caption = g_pConfig->value(provider + "/displayname").toString();
    QString realm = g_pConfig->value(provider + "/realm").toString();

    if(method.isNull())
    {
        qCritical() << "No method found with name: " << method;
        return 0;
    }

    // some account providers names differ with method names. A little fix here
    if(method == "ovi")
        method = "oviauth";

    qDebug() << "Creating a new identity....";
    qDebug() << "Method:   " << method;
    qDebug() << "Username: " << username;
    qDebug() << "Password: " << password;

    if ( m_identity ) delete m_identity;

    QMap<MethodName,MechanismsList> methods;

    QStringList mechs = QStringList() << QString("ClientLogin");

    methods.insert(method, mechs);

    // caption, username, methods
    qDebug() << "Creating new Identity with params caption: " << caption << " . username: " << username;

    m_info = new IdentityInfo(caption, username, methods);

    m_info->setSecret(password);
    //m_info->setSecret("test");

    QStringList realms = QStringList() << realm;
    //QStringList realms = QStringList() << QString::fromLatin1("google.com");
                         //<< QString::fromLatin1("example.com")
                         //<< QString::fromLatin1("example2.com");


    m_info->setRealms(realms);

    /*
    QString credName = "signon::";
    credName += username;
    QStringList acl = QStringList() << QString::fromLatin1("AID::10")
                      << credName;
                      //<< QString::fromLatin1("AID::10")

    m_info->setAccessControlList(acl);
    */

    /*
    int randomType = qrand() % 4;
    switch (randomType) {
    case 0:
        m_info->setType(IdentityInfo::Other);
        break;
    case 1:
        m_info->setType(IdentityInfo::Application);
        break;
    case 2:
        m_info->setType(IdentityInfo::Web);

        break;
    case 3:

        m_info->setType(IdentityInfo::Network);
        break;

    } */

    m_info->setType(IdentityInfo::Web);

    m_identity = Identity::newIdentity(*m_info);

    connect(m_identity, SIGNAL(credentialsStored(const quint32)),
            this, SLOT(credentialsStored(const quint32)));

    connect(m_identity, SIGNAL(error(const SignOn::Error &)),
            this, SLOT(error(const SignOn::Error &)));

    // save values to sso db
    m_identity->storeCredentials();


    qDebug() << "Wait for credentialStored-signal..";
    m_testObj->Start();

    MWTS_LEAVE;
    return m_identity->id();
}



bool SignonClient::CreateSession(const QString name)
{
    MWTS_ENTER;

    if (!m_identity) {
        qCritical() << "You have not created identity. Please run CreateIdentity first..";
        return false;
    }

    QString method = g_pConfig->value(name + "/provider").toString();
    QString username = g_pConfig->value(name + "/username").toString();
    QString password = g_pConfig->value(name + "/password").toString();


    // some account providers names differ with method names. A little fix here
    if(method == "ovi")
        method = "oviauth";

    qDebug() << "Seems you have created identity, so proceeding..";

    qDebug() << "Method: "   << method;
    qDebug() << "Username: " << username;
    qDebug() << "Password: " << password;
    qDebug() << "Identity Id is: " << m_identity->id();


    SessionData data;

    data.setSecret(password);
    //data.setSecret("test");
    data.setUserName(username);
    //data.setPassWord(password);

    // session is created based on a method name which should already be inserted to created identity
    if (!m_session) {
        m_session=m_identity->createSession(method);

        qDebug() << "Session object created.....look UI!";
        qDebug() << "METHOD: " << m_session->name();

        connect(m_session, SIGNAL(response(const SignOn::SessionData&)),
            this, SLOT(response(const SignOn::SessionData&)));

        connect(m_session, SIGNAL(error(const SignOn::Error &)),
            this, SLOT(sessionError(const SignOn::Error &)));

        connect(m_session, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString &)),
            this, SLOT(slotStateChanged(AuthSession::AuthSessionState, const QString &)));

    }

    m_session->process(data , QLatin1String("ClientLogin"));

    m_testObj->Start();

    return this->m_bSuccess;
    MWTS_LEAVE;
}


/*
 * SLOTS
 */

void SignonClient::clear()
{
    MWTS_ENTER;
    m_testObj->Stop();
    qDebug() << "Credentials database is clear now...";
    m_bSuccess = true;
    MWTS_LEAVE;
}

void SignonClient::slotStateChanged(AuthSession::AuthSessionState state, const QString &message)
{
    MWTS_ENTER;

    QString statestring = "SessionState: ";

    switch(state)
    {
        case AuthSession::SessionNotStarted:
            statestring += "Session not started";
            break;
        case AuthSession::HostResolving:
            statestring += "HostResolving";
            break;
        case AuthSession::ServerConnecting:
            statestring += "ServerConnecting";
            break;
        case AuthSession::DataSending:
            statestring += "DataSending";
            break;
        case AuthSession::ReplyWaiting:
            statestring += "ReplyWaiting";
            break;
        case AuthSession::UserPending:
            statestring += "UserPending";
            break;
        case AuthSession::UiRefreshing:
            statestring += "UiRefreshing";
            break;
        case AuthSession::ProcessPending:
            statestring += "ProcessPending";
            break;
        case AuthSession::ProcessDone:
            statestring += "ProcessDone";
            break;
        default:
            statestring += "default..";
    }

    qDebug() << statestring;

    MWTS_LEAVE;
}

void SignonClient::methodsAvailable(const QStringList &mechs)
{
    MWTS_ENTER;
    m_testObj->Stop();
    qDebug() << "Listing methods...";
    qDebug() << "##################";

    for (int i = 0; i < mechs.size(); ++i) {
        qDebug() << mechs.at(i).toLocal8Bit().constData() << endl;
        m_service->queryMechanisms(mechs.at(i));
    }

    MWTS_LEAVE;
    return;
}

void SignonClient::mechanismsAvailable(const QString &method, const QStringList &mechs)
{
    MWTS_ENTER;
    m_testObj->Stop();
    qDebug() << "Listing Mechanisms Available for method: ";
    qDebug() << "#############################";

    qDebug() << method;

    for (int i = 0; i < mechs.size(); ++i) {

        qDebug() << mechs.at(i).toLocal8Bit().constData() << endl;
    }

    MWTS_LEAVE;
    return;
}

void SignonClient::identities(const QList<IdentityInfo> &identityList)
{
    MWTS_ENTER;
    m_testObj->Stop();

    m_bSuccess = true;

    qDebug() << "Listing identities...";
    qDebug() << "#####################";

    for (int i = 0; i < identityList.size(); ++i) {
         //qDebug() << "Caption   : " << identityList.at(i).caption().toLocal8Bit().constData() << endl;
         qDebug() << "Caption   : " << identityList.at(i).caption();
         qDebug() << "Id        : " << identityList.at(i).id();
         qDebug() << "Has secret: " << identityList.at(i).isStoringSecret();
         qDebug() << "Username  : " << identityList.at(i).userName();
         qDebug() << "Realms    : " << identityList.at(i).realms();
         qDebug() << "AccessCtr : " << identityList.at(i).accessControlList();
         qDebug() << "Methods   : " << identityList.at(i).methods();
     }

    MWTS_LEAVE;
    return;
 }

void SignonClient::response(const SessionData &sessionData)
{
    MWTS_ENTER;
    m_testObj->Stop();
    Q_UNUSED(sessionData);

    QStringList property_names = sessionData.propertyNames();
    qDebug() << "Available properties for opened session: " << property_names;

    QVariant temp;

    for(int i=0; i<property_names.size(); i++)
    {
        temp = sessionData.getProperty(property_names.at(i));
        qDebug() << "Property: " << property_names.at(i) << " is: " << temp.toString();
    }

    // we got session response, test passes
    m_bSuccess = true;

    qDebug("Got session response.");
    MWTS_LEAVE;
}

void SignonClient::sessionError(const Error &error)
{
    MWTS_ENTER;
    m_testObj->Stop();

    qDebug("session Err: %d", error.type());
    qDebug() << error.message();
    m_bSuccess = false;

    MWTS_LEAVE;
}

void SignonClient::error(const Error &error)
{
    MWTS_ENTER;
    qDebug("identity Err: %d", error.type());
    qDebug() << error.message();
    m_bSuccess = false;

    MWTS_LEAVE;
}

void SignonClient::credentialsStored(const quint32 id)
{
    MWTS_ENTER;
    m_testObj->Stop();
    qDebug() << "Identity created!";
    qDebug() << "stored id: " << id;

    // the identity was created, test passes
    m_bSuccess = true;

    QString message;
    message.setNum(id);

    MWTS_LEAVE;

}

