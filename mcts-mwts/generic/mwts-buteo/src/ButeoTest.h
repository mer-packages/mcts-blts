/* AccountsTest.h -- min interface implementation

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

#include <QObject>
#include <QString>

#include <SyncClientInterface.h>
#include <libsyncprofile/ProfileManager.h>

#include <QDomElement>
#include <QDomDocument>

class QDomDocument;
class QDomElement;

using namespace Buteo;

class ButeoTest : public MwtsTest
{
	Q_OBJECT
public:

	/**
	 * Constructor for Accounts test class
	 */
        ButeoTest();

	/**
	 * Destructor for template test class
	 */
        virtual ~ButeoTest();

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

	/*
	* Starts syncing with given profile
	*/
        bool Sync(const QString profile_name);

	/*
	* Creates bluetooth specifig profile
	* @param profile_path path of profile template xml
	*/ 
        bool CreateBtProfile(const QString profile_path);

	/*
	* Creates google specific profile
	* @param profile_path path of profile template xml
	*/ 
        bool CreateGoogleProfile(const QString profile_path);

	/*
	* Removes profile
	* @param profile_name name of the removed profile
	*/ 
        bool RemoveProfile(const QString profile_name);

	/*
	* Removes all profiles from device
	*/ 
        bool RemoveProfiles();

	/*
	* Lists available sync profiles
	*/ 
        bool ListProfiles();


private:
        /*
         * helper function to load profile from given template xml. Same functionality is also in the ProfileManager-class
         */
        SyncProfile *loadProfileFromXml(QString filename);

        SyncClientInterface *sci;
        ProfileManager *pm;

        bool m_bSuccess;

protected slots:
        void slotSyncStatus(QString aProfileId, int aStatus, QString aMessage, int aErrorCode);
        void slotProfileChanged(QString aProfileId,int aChangeType, QString aChangedProfile);
        void slotResultsAvailable(QString aProfileId  , Buteo::SyncResults aResults);
        void slotTransferProgress(QString aProfileId, int aTransferDatabase, int aTransferType , QString aMimeType, int aCommittedItems);
};

#endif //#ifndef _INCLUDED_TEMPLATE_TEST_H

