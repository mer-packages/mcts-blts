/* AccountsTest.cpp -- min interface implementation

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
#include "ButeoTest.h"

/**
 * Constructor for template test class
 */
ButeoTest::ButeoTest()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

/**
 * Destructor for template test class
 */
ButeoTest::~ButeoTest()
{
    MWTS_ENTER;

    /*
    if(sci)
    {
        delete sci;
        sci = NULL;
    } */

    if(pm)
    {
        delete pm;
        pm = NULL;
    }

    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void ButeoTest::OnInitialize()
{
    MWTS_ENTER;

    g_pLog->EnableDebug(true);
    g_pLog->EnableTrace(true);

    qDebug() << "CONNECTING SIGNALS!!!";

    sci = new SyncClientInterface();

    // test verdict
    m_bSuccess = false;

    connect(sci, SIGNAL(syncStatus(QString,int,QString,int)), this, SLOT(slotSyncStatus(QString,int,QString,int)));
    connect(sci, SIGNAL(resultsAvailable(QString,Buteo::SyncResults)), this, SLOT(slotResultsAvailable(QString,Buteo::SyncResults)));
    connect(sci, SIGNAL(transferProgress(QString,int,int,QString,int)), this, SLOT(slotTransferProgress(QString,int,int,QString,int)));

    QString sync_folder  = g_pConfig->value("sync_home").toString();

    if(sync_folder.isNull())
    {
        qDebug() << "No sync folder path defined, using /home/meego/.sync/profiles";
        //sync_folder = "/home/meego/.sync/profiles/";
        pm = new ProfileManager("/home/meego/.sync/profiles/");
    }

    pm = new ProfileManager(sync_folder);

    MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void ButeoTest::OnUninitialize()
{
    MWTS_ENTER;

    MWTS_LEAVE;
}

void ButeoTest::slotSyncStatus(QString aProfileId, int aStatus, QString aMessage, int aErrorCode)
{
    MWTS_ENTER;
    QString stat = "";
    if (aStatus == 0) {
      stat = "QUEUED";
    } else if (aStatus == 1) {
      stat = "RUNNING";
    } else if (aStatus == 2) {
      stat = "PROGRESSING";
    } else if (aStatus == 3) {
      stat = "ERROR";
      this->Stop();
      //emit syncDone(-3);
    } else if (aStatus == 4) {
      stat = "DONE";
      //this->Stop();
    } else if (aStatus == 5) {
      stat = "ABORTED";
    }

    qDebug() << "STATUS CHANGE:" << aProfileId << stat << aMessage << aErrorCode;
    MWTS_LEAVE;
}

void ButeoTest::slotProfileChanged(QString aProfileId,int aChangeType, QString aChangedProfile)
{
    MWTS_ENTER;

    qDebug() << "Profile changed to: " << aProfileId << " and : " << aChangedProfile;
    qDebug() << "Changetyp: " << aChangeType << " And change profile: " << aChangedProfile;

    MWTS_LEAVE;
}

void ButeoTest::slotResultsAvailable(QString aProfileId, SyncResults aResults)
{
    MWTS_ENTER;
    this->Stop();
    qDebug() << "SYNC Complete!";
    qDebug() << "Results available for profile: " << aProfileId;
    if (aResults.majorCode() == SyncResults::SYNC_RESULT_SUCCESS)
    {
        qDebug() << "Sync succeeded!";

        double elapsed=g_pTime->elapsed();
        qDebug() << "Sync took: " << elapsed << " ms";

        g_pResult->AddMeasure("Sync lasted: ", elapsed, "ms");

        m_bSuccess = true;

    } else if (aResults.majorCode() == SyncResults::SYNC_RESULT_FAILED)
    {

        qDebug() << "Minor code is: " << aResults.minorCode();
        switch(aResults.minorCode())
        {
            case SyncResults::SYNC_FINISHED:
                qCritical() << "SYNC_FINISHED";
                break;
            case SyncResults::NO_ERROR:
                qCritical() << "NO_ERROR";
                break;
            case SyncResults::INTERNAL_ERROR:
                qCritical() << "INTERNAL_ERROR";
                break;
            case SyncResults::AUTHENTICATION_FAILURE:
                qDebug() << "AUTHENTICATION_FAILURE";
                qCritical() << "Check the conf file if the credentials are right.";
                break;
            case SyncResults::DATABASE_FAILURE:
                qCritical() << "DATABASE_FAILURE";
                break;
            case SyncResults::ABORTED:
                qCritical() << "ABORTED";
                break;
            case SyncResults::CONNECTION_ERROR:
                qCritical() << "CONNECTION_ERROR";
                break;
            case SyncResults::INVALID_SYNCML_MESSAGE:
                qCritical() << "INVALID_SYNCML_MESSAGE";
                break;
            case SyncResults::UNSUPPORTED_SYNC_TYPE:
                qCritical() << "UNSUPPORTED_SYNC_TYPE";
                break;
        }
        qCritical() << "Sync failed...";

        m_bSuccess = false;
    }

    MWTS_LEAVE;
}


void ButeoTest::slotTransferProgress(QString aProfileId, int aTransferDatabase, int aTransferType , QString aMimeType, int aCommittedItems )
{
    MWTS_ENTER;

    qDebug() << "Progress:" << aProfileId << aTransferDatabase << aTransferType << aMimeType;

    MWTS_LEAVE;
}

/*
  * loads a file contents to a QDomDocument, creates and returns a SyncProfile pointer based on that xml
  */
SyncProfile *ButeoTest::loadProfileFromXml(QString filename)
{

    //QFile file(PROFILE_DIR + "/" + aType + "/" + aName + ".xml");

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed opening file: " << filename;
        //return NULL;
    } // no else

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        qCritical() << "Failed setting the contents of profile xml";
        //return NULL;
    } // no else
    file.close();

    return new SyncProfile(doc.documentElement());

}

bool ButeoTest::CreateBtProfile(const QString profile_path)
{
    MWTS_ENTER;
    qDebug() << "Creating a bluetooth profile based on: " << profile_path;

    SyncProfile *syncProfile = loadProfileFromXml(profile_path);

    //Profile *syncProfile = pm->profileFromXml(profile_path);

    if(syncProfile == NULL)
    {
        qCritical() << "Creation of bt_template.xml failed aborting. Filepath propably wrong.";
        return false;
    }

    QString bt_name      = g_pConfig->value("bt_profile/bt_name").toString();
    QString bt_address   = g_pConfig->value("bt_profile/bt_address").toString();
    QString profile_name = g_pConfig->value("bt_profile/profile_name").toString();

    if(bt_name.isNull())
    {
        qCritical() << "No bluetooth parameters in config file, aborting...";
        return false;
    }

    if(bt_address.isNull())
        bt_address = "00:00:00:00:00:00";

    qDebug() << "Profile loaded!";
    syncProfile->setName(profile_name);
    qDebug() << "Name            : " << syncProfile->name();
    qDebug() << "Type of profile : " << syncProfile->type();

    // set needed parameters for the profile root
    syncProfile->setKey("displayname", "btprofile");
    syncProfile->setKey("hidden", "false");

    // create sub profiles
    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;
    qDebug() << "Creating sub profile for bluetooth (bt) and setting parameters";

    Profile *btProfile;

    btProfile = syncProfile->subProfile("bt");

    qDebug() << "Using parameters. bt_name: " << bt_name << " . and bt_address: " << bt_address;
    btProfile->setKey("bt_name", bt_name);
    btProfile->setKey("bt_address", bt_address);
    btProfile->setKey("device", "destinationtype");

    /*
    qDebug() << "Creating sub profile for sync ml client and setting parameters";
    Profile *syncmlProfile;

    syncmlProfile = syncProfile->subProfile("syncml");

    syncmlProfile->setKey("Sync Transport", "OBEX");
    syncmlProfile->setKey("Sync Protocol", "SyncML11");
    syncmlProfile->setKey("use_wbxml", "true");
    syncmlProfile->setKey("conflictpolicy", "prefer remote");
    syncmlProfile->setKey("Sync Direction", "two-way");

    qDebug() << "Creating sub profile for hcontacts and setting parameters";
    Profile *hcontacts;

    hcontacts = syncProfile->subProfile("hcontacts");

    hcontacts->setKey("Local URI", "./contacts");
    hcontacts->setKey("Target URI", "./contacts");
    hcontacts->setKey("Type", "text/x-vcard");
    hcontacts->setKey("Version", "2.1");
    hcontacts->setKey("enabled", "true"); */


    qDebug() << "Updating and enabling bluetooth profile...";
    syncProfile->setEnabled(true);
    pm->updateProfile(*syncProfile);

    delete syncProfile;
    syncProfile = NULL;

    return true;
    MWTS_LEAVE;
}


bool ButeoTest::CreateGoogleProfile(const QString profile_path)
{
    MWTS_ENTER;
    qDebug() << "Creating a cloud service profile based on: " << profile_path;
    qDebug() << "Profile type is: google_profile";

    QString profile_name = g_pConfig->value("google_profile/profile_name").toString();
    QString username     = g_pConfig->value("google_profile/username").toString();
    QString password     = g_pConfig->value("google_profile/password").toString();

    SyncProfile *syncProfile = loadProfileFromXml(profile_path);

    if(syncProfile == NULL)
    {
        qCritical() << "Loading of " << profile_path << "  failed aborting. Filepath propably wrong.";
        return false;
    }

    qDebug() << "Creating profile: " << profile_name << " : " << username << " : " << password;

    syncProfile->setName(profile_name);

    qDebug() << "Name            : " << syncProfile->name();
    qDebug() << "Type of profile : " << syncProfile->type();

    // set needed parameters for the profile root
    syncProfile->setKey("displayname", profile_name);
    syncProfile->setKey("hidden", "false");
    syncProfile->setKey("enabled", "true");

    // create sub profiles
    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;

    Profile *google_profile;

    google_profile = syncProfile->subProfile("google.com");
    google_profile->setKey("Username", username);
    google_profile->setKey("Password", password);
    google_profile->setKey("destinationtype", "online");

    qDebug() << "Creating sub profile for hcontacts and setting parameters";
    Profile *hcontacts;

    hcontacts = syncProfile->subProfile("hcontacts");
    /*
    hcontacts->setKey("Local URI", "./contacts");
    hcontacts->setKey("Target URI", "./contacts");
    hcontacts->setKey("Type", "text/x-vcard");
    hcontacts->setKey("Version", "2.1"); */
    hcontacts->setKey("enabled", "true");

    qDebug() << "Updating and enabling " << profile_name << " profile...";
    syncProfile->setEnabled(true);
    pm->updateProfile(*syncProfile);

    delete syncProfile;
    syncProfile = NULL;

    return true;
    MWTS_LEAVE;
}

bool ButeoTest::CreateMemotooProfile(const QString profile_path)
{
    MWTS_ENTER;
    qDebug() << "Creating a cloud service profile based on: " << profile_path;
    qDebug() << "Profile type is: memotoo_profile";

    QString profile_name = g_pConfig->value("memotoo_profile/profile_name").toString();
    QString username     = g_pConfig->value("memotoo_profile/username").toString();
    QString password     = g_pConfig->value("memotoo_profile/password").toString();

    SyncProfile *syncProfile = loadProfileFromXml(profile_path);

    if(syncProfile == NULL)
    {
        qCritical() << "Loading of " << profile_path << "  failed aborting. Filepath propably wrong.";
        return false;
    }

    qDebug() << "Creating profile: " << profile_name << " : " << username << " : " << password;

    syncProfile->setName(profile_name);

    qDebug() << "Name            : " << syncProfile->name();
    qDebug() << "Type of profile : " << syncProfile->type();

    // set needed parameters for the profile root
    syncProfile->setKey("displayname", profile_name);
    syncProfile->setKey("hidden", "false");
    syncProfile->setKey("enabled", "true");

    // create sub profiles
    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;

    Profile *memotoo_profile;

    memotoo_profile = syncProfile->subProfile("memotoo.com");
    memotoo_profile->setKey("Username", username);
    memotoo_profile->setKey("Password", password);
    memotoo_profile->setKey("destinationtype", "online");

    qDebug() << "Updating and enabling " << profile_name << " profile...";
    syncProfile->setEnabled(true);
    pm->updateProfile(*syncProfile);

    delete syncProfile;
    syncProfile = NULL;

    return true;
    MWTS_LEAVE;
}

bool ButeoTest::CreateMobicalProfile(const QString profile_path)
{
    MWTS_ENTER;
    qDebug() << "Creating a cloud service profile based on: " << profile_path;
    qDebug() << "Profile type is: mobical_profile";

    QString profile_name = g_pConfig->value("mobical_profile/profile_name").toString();
    QString username     = g_pConfig->value("mobical_profile/username").toString();
    QString password     = g_pConfig->value("memotoo_profile/password").toString();

    SyncProfile *syncProfile = loadProfileFromXml(profile_path);

    if(syncProfile == NULL)
    {
        qCritical() << "Loading of " << profile_path << "  failed aborting. Filepath propably wrong.";
        return false;
    }

    qDebug() << "Creating profile: " << profile_name << " : " << username << " : " << password;

    syncProfile->setName(profile_name);

    qDebug() << "Name            : " << syncProfile->name();
    qDebug() << "Type of profile : " << syncProfile->type();

    // set needed parameters for the profile root
    syncProfile->setKey("displayname", profile_name);
    syncProfile->setKey("hidden", "false");
    syncProfile->setKey("enabled", "true");

    // create sub profiles
    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;

    Profile *mobical_profile;

    mobical_profile = syncProfile->subProfile("mobical.com");
    mobical_profile->setKey("Username", username);
    mobical_profile->setKey("Password", password);
    mobical_profile->setKey("destinationtype", "online");

    qDebug() << "Updating and enabling " << profile_name << " profile...";
    syncProfile->setEnabled(true);
    pm->updateProfile(*syncProfile);

    delete syncProfile;
    syncProfile = NULL;

    return true;
    MWTS_LEAVE;
}

bool ButeoTest::CreateOviProfile(const QString profile_path)
{
    MWTS_ENTER;
    qDebug() << "Creating a cloud service profile based on: " << profile_path;
    qDebug() << "Profile type is: ovi_profile ";

    QString profile_name = g_pConfig->value("ovi_profile/profile_name").toString();
    QString username     = g_pConfig->value("ovi_profile/username").toString();
    QString password     = g_pConfig->value("ovi_profile/password").toString();

    SyncProfile *syncProfile = loadProfileFromXml(profile_path);

    if(syncProfile == NULL)
    {
        qCritical() << "Loading of " << profile_path << " failed aborting. Filepath propably wrong.";
        return false;
    }

    qDebug() << "Creating profile: " << profile_name << " : " << username << " : " << password;

    syncProfile->setName(profile_name);

    qDebug() << "Name            : " << syncProfile->name();
    qDebug() << "Type of profile : " << syncProfile->type();

    // set needed parameters for the profile root
    syncProfile->setKey("displayname", profile_name);
    syncProfile->setKey("hidden", "false");
    syncProfile->setKey("enabled", "true");

    // create sub profiles
    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;

    Profile *ovi_profile;

    ovi_profile = syncProfile->subProfile("ovi-contact");
    ovi_profile->setKey("Username", username);
    ovi_profile->setKey("Password", password);
    ovi_profile->setKey("destinationtype", "online");

    qDebug() << "Updating and enabling " << profile_name << " profile...";
    syncProfile->setEnabled(true);

    pm->updateProfile(*syncProfile);

    delete syncProfile;
    syncProfile = NULL;

    return true;
    MWTS_LEAVE;
}


bool ButeoTest::ListProfiles()
{
    QList<SyncProfile*> profiles = pm->allSyncProfiles();

    m_bSuccess = true;

    qDebug() << "List from Profile Manager:";
    const int size = profiles.size();

    for(int i=0; i<size;i++)
    {
       qDebug() << "/// Profile " << i;
       qDebug() << "profile name: " << profiles.at(i)->name();
       qDebug() << "display name: " << profiles.at(i)->displayname();
       qDebug() << "type        : " << profiles.at(i)->type();
       qDebug() << "is valid    : " << profiles.at(i)->isValid();
    }

    return m_bSuccess;
}


bool ButeoTest::RemoveProfile(const QString profile_name)
{
    return pm->removeProfile(profile_name);
}

bool ButeoTest::RemoveProfiles()
{
    MWTS_ENTER;

    QList<SyncProfile*> profiles = pm->allSyncProfiles();

    SyncProfile *temp;

    bool bReturn = false;

    const int size = profiles.size();

    for(int i=0; i<size;i++)
    {
        temp = profiles.at(i);
        qDebug() << "Removing profile: " << temp->name();
        bReturn = pm->removeProfile(temp->name());
    }

    temp = NULL;

    return true;
    MWTS_LEAVE;
}


bool ButeoTest::Sync(const QString profile_name)
{
    MWTS_ENTER;

    SyncProfile *syncProfile = new SyncProfile(profile_name);
    qDebug() << "Name: " << syncProfile->name();
    qDebug() << "Type of profile: " << syncProfile->type();

    QList<QString> subProfileNames = syncProfile->subProfileNames();
    qDebug() << "SubProfiles: " << subProfileNames;

    qDebug() << "Creating sync client interface and setting parameters...";
    sci = new SyncClientInterface();

    qDebug() << "Verify dbus connection to msync: " << sci->isValid();
    if(!sci->isValid())
    {
        qCritical() << "Connection to msyncd is not valid. Check the state of daemon. Aborting...";
        return false;
    }

    sci->startSync(profile_name);

    // start timer
    g_pTime->start();

    qDebug() << "Starting the sync!";
    this->Start();

    QStringList lst = sci->getRunningSyncList();
    qDebug() << "Running syncs:" << lst.length();
    const int size = lst.length();
    for(int i = 0; i < size; i++) {
        qDebug() << "Running" << i << ":" << lst.at(i);
    }

    delete syncProfile;
    syncProfile = NULL;

    delete sci;
    sci = NULL;

    return m_bSuccess;
    MWTS_LEAVE;
}



