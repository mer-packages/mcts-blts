
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

    connect(m_service, SIGNAL(mechanismsAvailable(const QString &, const QStringList &)),
        this, SLOT(mechanismsAvailable(const QString &, const QStringList &)));

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

<<<<<<< HEAD
=======
    connect(m_service, SIGNAL(mechanismsAvailable(const QString &, const QStringList &)),
        this, SLOT(mechanismsAvailable(const QString &, const QStringList &)));


>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
    qDebug() << "Querying identities...";
    m_service->queryIdentities();
    m_testObj->Start();

    qDebug() << "Querying methods...";
    m_service->queryMethods();
    m_testObj->Start();

<<<<<<< HEAD
=======
    qDebug() << "Querying mechanisms for facebook...";
    m_service->queryMechanisms("facebook");
    m_testObj->Start();

    qDebug() << "Querying mechanisms for googlek...";
    m_service->queryMechanisms("google");
    m_testObj->Start();

>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
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
        qCritical() << "No method found with name: " << provider;
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

    qDebug() << "Querying mechanisms for " << method;
    m_service->queryMechanisms(method);
    m_testObj->Start();

    qDebug() << "Mechanism available for " << method << " are: " << m_strListMechanisms;
    methods.insert(method, this->m_strListMechanisms);

    // caption, username, methods
    qDebug() << "Creating new Identity with params caption: " << caption << " . username: " << username;

    m_info = new IdentityInfo(caption, username, methods);

    // store password with identity
    m_info->setSecret(password);

    QStringList realms = QStringList() << realm;
    //QStringList realms = QStringList() << QString::fromLatin1("google.com");
                         //<< QString::fromLatin1("example.com")
                         //<< QString::fromLatin1("example2.com");

    m_info->setRealms(realms);

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
    QString mechanism = g_pConfig->value(name + "/mechanism").toString();

    // some account providers names differ with method names. A little fix here
    if(method == "ovi")
        method = "oviauth";

    qDebug() << "Seems you have created identity, so proceeding..";

    qDebug() << "Method    : "  << method;
    qDebug() << "Mechanism : "  << mechanism;
    qDebug() << "Username  : "  << username;
    qDebug() << "Password  : "  << password;
    qDebug() << "Identity Id is: " << m_identity->id();

    if(mechanism.isNull())
    {
        qCritical() << "No mechanism to signin with this provider. It is not currently supported!";
        return 0;
    }

    SessionData data;

    //provide session with auth credentials
    //data.setSecret(password);
    //data.setUserName(username);

    // session is created based on a method name which should already be inserted to created identity
    if (!m_session) {

        // start timer
        g_pTime->start();

        m_session=m_identity->createSession(method);

        qDebug() << "Session object created.....look UI!";
        qDebug() << "Method is                   : " << m_session->name();
        qDebug() << "Authentication mechanism is : " << mechanism;

        //m_testObj->Start();

<<<<<<< HEAD
=======

        m_session->queryAvailableMechanisms();

        connect(m_session, SIGNAL(mechanismsAvailable(const QStringList &)),
            this, SLOT(slotMechanismsAvailable(const QStringList &)));

        m_testObj->Start();

>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
        connect(m_session, SIGNAL(response(const SignOn::SessionData&)),
            this, SLOT(response(const SignOn::SessionData&)));

        connect(m_session, SIGNAL(error(const SignOn::Error &)),
            this, SLOT(sessionError(const SignOn::Error &)));

        connect(m_session, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString &)),
            this, SLOT(slotStateChanged(AuthSession::AuthSessionState, const QString &)));

    }
    m_session->process(data , mechanism);
    //m_session->process(data , QLatin1String("signin"));

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

void SignonClient::slotMechanismsAvailable(const QStringList &mechanisms)
{
    MWTS_ENTER;
    m_testObj->Stop();
    qDebug() << "LIST OF AVAILABLE MECHANISMS FOR THE CURRENT SESSION: " << mechanisms;

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
    qDebug() << "Listing Mechanisms Available for method: " << method;
    qDebug() << "#############################";

    for (int i = 0; i < mechs.size(); ++i) {

        qDebug() << mechs.at(i).toLocal8Bit().constData() << endl;
    }

    this->m_strListMechanisms << mechs;

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


/*
 * the sign on response
 */
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

    double elapsed=g_pTime->elapsed();
    qDebug() << "Signon response took: " << elapsed << " ms";

    g_pResult->AddMeasure("SignOn latency: ", elapsed, "ms");

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

