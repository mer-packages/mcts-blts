/* signonclient.h -- Singon test definition

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

#ifndef SIGNONCLIENT_H
#define SIGNONCLIENT_H

#include <QObject>
#include <MwtsCommon>

#include <AuthService>
#include <Identity>

using namespace SignOn;

class SignonClient : public QObject
{
    Q_OBJECT
public:
    SignonClient(MwtsTest* testObj);
    virtual ~SignonClient();

    /*
     * Creates identity and stores its credentials to sso db
     */
    int  CreateIdentity(const QString provider);

    /*
     * Creates/signs on a session to a service with created identity
     */
    bool CreateSession(const QString name);

    /*
     * used by ListIdents-function. Requests list of idents from auth service
     */
    bool queryIdents();

    /*
     * clears credentials from sso db
     */
    bool clearCredentials();
private:

    bool m_bSuccess; // test verdict

    QStringList m_strListMechanisms;

    /*
     * Pointer to MwtsTest object. Needed for event loop
     */
    MwtsTest *m_testObj;

    /*
     * Base service to query identities, mechanisms and service methods
     */
    AuthService *m_service;


    /*
     * Authentication identity
     */
    Identity *m_identity;

    /*
     * Helper class to create Identity
     */
    IdentityInfo *m_info;


    /*
     * Service session
     */
    AuthSession *m_session;


public slots:
    /*
     * AuthService slots
     */
    void methodsAvailable(const QStringList &mechs);
    void mechanismsAvailable(const QString &method, const QStringList &mechs);
    void identities(const QList<SignOn::IdentityInfo> &identityList);

    /*
     * Identity queryInfo() slot
     */
    void slotInfo(const SignOn::IdentityInfo &info);

    /*
     * Auth service database credentials cleared slot
     */
    void clear();

    /*
     * error slot
     */
    void error(const SignOn::Error &error);

    /*
     * Authentication credentials stored slot
     */
    void credentialsStored(const quint32 id);

    /*
     * Slot for session response
     */
    void response(const SignOn::SessionData &sessionData);

    /*
     * Session state change slot
     */
    void slotStateChanged(AuthSession::AuthSessionState state, const QString &message);

    /*
     * Session error slot
     */
    void sessionError(const SignOn::Error &error);


    void slotMechanismsAvailable(const QStringList &mechanisms);

};

#endif // SIGNONCLIENT_H
