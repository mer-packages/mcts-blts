/* PimTest.cpp -- Source code for Pim test class

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
#include "PimTest.h"
#include <qtorganizer.h>
#include <QStringList>
#include <QBuffer>
#include <QVersitReader>
#include <QVersitWriter>
#include <QVersitDocument>
#include <QVersitContactImporter>
#include <QVersitContactExporter>
#include <QContactDetail>
#include <QContactAddress>
// Contact details
#include <QContactAddress>
#include <QContactAnniversary>
#include <QContactAvatar>
#include <QContactBirthday>
#include <QContactEmailAddress>
#include <QContactFamily>
#include <QContactGender>
#include <QContactGeoLocation>
#include <QContactGlobalPresence>
#include <QContactGuid>
#include <QContactName>
#include <QContactNickname>
#include <QContactNote>
#include <QContactOnlineAccount>
#include <QContactOrganization>
#include <QContactPhoneNumber>
#include <QContactPresence>
#include <QContactRingtone>
#include <QContactSyncTarget>
#include <QContactTag>
#include <QContactThumbnail>
#include <QContactTimestamp>
#include <QContactType>
#include <QContactUrl>
// Contact details

/**
 * Constructor for Pim test class
 */
PimTest::PimTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Destructor for Pim test class
 */
PimTest::~PimTest()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnInitialize is called before test execution
 */
void PimTest::OnInitialize()
{
	MWTS_ENTER;

	// create objects for the test
	// connect wanted signals
	// do any kind of initialization here

        // one item is always createdQVersitDocument
        m_numberOfItems = 1;
        // add vCard
        m_inputVCard = "BEGIN:VCARD\r\nVERSION:3.0\r\nADR:pobox;;street;Locality;region;12345;Finland\r\n\r\nX-ANNIVERSARY:2010-11-22\r\nBDAY:2010-11-22\r\nEMAIL:tester@tesinghouse.fi\r\nX-SPOUSE:wife\r\nX-CHILDREN:child 1\r\nX-GENDER:Male\r\nGEO:142.5;60.7503\r\nUID:testguid\r\nN:Tester;John;Uber;prefix;suffix\r\nFN:custom label\r\nNICKNAME:T\r\nNOTE:this is test contact\r\nTITLE:Master tester\r\nORG:Test house;tes unit\r\nX-ASSISTANT:assistant\r\nROLE:Ruler\r\nTEL:+35801123123\r\nCATEGORIES:for testing\r\nREV:2010-11-22T09:04:56\r\nURL:contact url\r\nEND:VCARD\r\n";

        MWTS_LEAVE;
}

/**
 * Overridden functions for MwtsTest class
 * OnUninitialize is called after test execution
 */
void PimTest::OnUninitialize()
{
	MWTS_ENTER;

        // make sure that everything is cleared
        RemoveCreatedItemsFromContacts();
        RemoveCreatedItemsFromCalendar();
        ClearManagers();
        qDebug()<<"Datastore count after deletion: "<<m_dataStores.count();
        qDebug()<<"Item count after deletion: "<<m_items.count();


        MWTS_LEAVE;
}

/**
 * SetNumberOfCreatedItems function
 */
void PimTest::SetNumberOfCreatedItems(int n)
{
    MWTS_ENTER;
    m_numberOfItems = n;
    qDebug()<<"Number of wanted items set to: "<<m_numberOfItems;
}

/**
 * SetCalendarDatastore function
 */
void PimTest::SetCalendarDatastore(QString name)
{
    MWTS_ENTER;
    m_calendarStore = name;
    qDebug()<<"Calendar datastore set to: "<<m_calendarStore;
}

/**
 * SetContactDatastore function
 */
void PimTest::SetContactDatastore(QString name)
{
    MWTS_ENTER;
    m_contactStore = name;
    qDebug()<<"Contact datastore set to: "<<m_contactStore;
}

void PimTest::SetCalendarItemType(int type)
{
    m_calendarItemType = type;
}

/**
 * CreateAvailableCalendarDatastores function
 */
void PimTest::CreateAvailableCalendarDatastores()
{
        MWTS_ENTER;

        qDebug()<<"Fetching available calendar datastores";

        bool success=true;
        QStringList list = QOrganizerItemManager::availableManagers();

        qDebug()<<"Found datastores:";

        const int count = list.count();
        for(int i=0;i<count;++i) {
            qDebug()<<""<<i<<" "<<list[i];
        }

        // if there is available stores
        if(count>0) {
            QString name;
            QString uri;
            int version;
            QString parseName;
            QMap<QString,QString> parameters;
            QMap<QString,QString> parseParameters;
            QOrganizerItemManager *manager = 0;
            QOrganizerItemManager *uriManager = 0;
            qDebug()<<"Creating found datastores";

            for(int i=0;i<count;++i) {

               name = list[i].toLower();
               qDebug()<<"----------------";
               qDebug()<<i<<": Datastore name: "<<name;
               manager = new QOrganizerItemManager(name);

               if(manager) {

                   qDebug()<<"Manager created";
                   // validate name
                   if(name != manager->managerName().toLower()) {
                       qDebug()<<"Name "<<manager->managerName()<<" invalid";
                       success = false;
                       break;
                   }


                   qDebug()<<"Supported data types: "<<manager->supportedDataTypes();
                   qDebug()<<"Supported item types: "<<manager->supportedItemTypes();

                   // get datastore parameters
                   parameters = manager->managerParameters();
                   qDebug()<<"Parameters: "<<parameters;

                   // get datastore uri
                   uri = manager->buildUri(name,parameters);
                   qDebug()<<"Build uri: "<<uri;

                   // validate uri
                   bool parseOk = QOrganizerItemManager::parseUri(uri, &parseName, &parseParameters);
                   if(!parseOk ||
                      parseName != name ||
                      parseParameters != parameters) {
                       qDebug()<<"Uri parsing failed";
                       success = false;
                       break;
                   }

                   // get version
                   version = manager->managerVersion();
                   qDebug()<<"Version: "<<version;

                   qDebug()<<"Create manager again from uri: "<<uri;
                   uriManager = QOrganizerItemManager::fromUri(uri);
                   if(!uriManager) {
                       qDebug()<<"Manager is NULL";
                       success = false;
                       break;
                   } else {
                       qDebug()<<"Manager created";
                       // verify managers
                       if(manager->managerName().toLower() != uriManager->managerName().toLower() ||
                          manager->managerVersion() != uriManager->managerVersion() ||
                          manager->managerUri() != uriManager->managerUri() ||
                          manager->managerParameters() != uriManager->managerParameters()) {
                           qDebug()<<"Manager validation failed";
                           success = false;
                           break;
                       } else {
                           qDebug()<<"Uri creation validated";
                       }
                   }
               } else {
                   qDebug()<<"Manager is NULL";
                   success=false;
                   break;
               }
               delete manager;
               delete uriManager;
               manager=0;
               uriManager=0;
            } // for

            if(manager)
                delete manager;

            if(uriManager)
                delete uriManager;
        }   // if count

        // write the result
        g_pResult->StepPassed("Calendar datastore listing", success);
}


/**
 * CreateAvailableContactDatastores function
 */
void PimTest::CreateAvailableContactDatastores()
{
        MWTS_ENTER;

        qDebug()<<"Fetching available contact datastores";

        bool success=true;
        QStringList list = QContactManager::availableManagers();

        qDebug()<<"Found datastores:";

        const int count = list.count();
        for(int i=0;i<count;++i) {
            qDebug()<<""<<i<<" "<<list[i];
        }

        // if there is available stores
        if(count>0) {
            QString name;
            QString uri;
            int version;
            QString parseName;
            QMap<QString,QString> parameters;
            QMap<QString,QString> parseParameters;
            QContactManager *manager = 0;
            QContactManager *uriManager = 0;
            qDebug()<<"Creating found datastores";

            for(int i=0;i<count;++i) {

               name = list[i].toLower();
               qDebug()<<"----------------";
               qDebug()<<i<<": Datastore name: "<<name;
               manager = new QContactManager(name);

               if(manager) {

                   qDebug()<<"Manager created";
                   // validate name
                   if(name != manager->managerName().toLower()) {
                       qDebug()<<"Name "<<manager->managerName()<<" invalid";
                       success = false;
                       break;
                   }


                   qDebug()<<"Supported data types: "<<manager->supportedDataTypes();
                   qDebug()<<"Supported contact types: "<<manager->supportedContactTypes();

                   // get datastore parameters
                   parameters = manager->managerParameters();
                   qDebug()<<"Parameters: "<<parameters;

                   // get datastore uri
                   uri = manager->buildUri(name,parameters);
                   qDebug()<<"Build uri: "<<uri;

                   // validate uri
                   bool parseOk = QContactManager::parseUri(uri, &parseName, &parseParameters);
                   if(!parseOk ||
                      parseName != name ||
                      parseParameters != parameters) {
                       qDebug()<<"Uri parsing failed";
                       success = false;
                       break;
                   }

                   // get version
                   version = manager->managerVersion();
                   qDebug()<<"Version: "<<version;

                   qDebug()<<"Create manager again from uri: "<<uri;
                   uriManager = QContactManager::fromUri(uri);
                   if(!uriManager) {
                       qDebug()<<"Manager is NULL";
                       success = false;
                       break;
                   } else {
                       qDebug()<<"Manager created";
                       // verify managers
                       if(manager->managerName().toLower() != uriManager->managerName().toLower() ||
                          manager->managerVersion() != uriManager->managerVersion() ||
                          manager->managerUri() != uriManager->managerUri() ||
                          manager->managerParameters() != uriManager->managerParameters()) {
                           qDebug()<<"Manager validation failed";
                           success = false;
                           break;
                       } else {
                           qDebug()<<"Uri creation validated";
                       }
                   }
               } else {
                   qDebug()<<"Manager is NULL";
                   success=false;
                   break;
               }
               delete manager;
               delete uriManager;
               manager=0;
               uriManager=0;
            } // for

            if(manager)
                delete manager;

            if(uriManager)
                delete uriManager;
        }   // if count

        // write the result
        g_pResult->StepPassed("Contact datastore listing", success);
}




/**
 * TestItemCreation function
 */
void PimTest::TestCalendarItemCreation()
{
    MWTS_ENTER;

    bool result = CreateCalendarDatastore();
    if(result) {
         result = CreateCalendarItems();
    }
    // write the result
    g_pResult->StepPassed("ItemCreation", result);
}

/**
 * TestItemCreationDeleting function
 */
void PimTest::TestCalendarItemCreationDeleting()
{
    MWTS_ENTER;

    bool result = CreateCalendarDatastore();
    if(result) {
        result = CreateCalendarItems();
        if(result) {
            result = RemoveCreatedItemsFromCalendar();
        }
    }
    // write the result
    g_pResult->StepPassed("ItemCreationDeleting", result);
}

/**
 * TestItemCreationModifyingDeleting function
 */
void PimTest::TestCalendarItemCreationModifyingDeleting()
{
    MWTS_ENTER;

    bool result = CreateCalendarDatastore();
    if(result) {
        result = CreateCalendarItems();
        if(result) {
            result = ModifyCalendarItems();
            if(result) {
                result = RemoveCreatedItemsFromCalendar();
            }
        }
    }
    // write the result
    g_pResult->StepPassed("ItemCreationModifyingDeleting", result);
}

/**
 * TestItemSearching function
 *
 * NOT TESTED
 * QOrganizerItemIntersectionFilter, QOrganizerItemUnionFilter
 */
void PimTest::TestItemSearching()
{
    MWTS_ENTER;

    bool result = CreateCalendarDatastore();
    if(result) {

        QOrganizerItemManager *manager = FindCalendarDataStore();
        const int count = 6;
        // first create some items (check CalendarItemTypes form PimTest.h)
        for(int i=0;i<count;++i) {
            m_calendarItemType = i;
            result = CreateCalendarItems();
            if(!result) {
                break;
            } else {
                qDebug()<<"Item: "<<i+1<<", type: "<<m_calendarItemType<<" created";
            }
        }

        // define filters
        QDateTime time;
        time = time.currentDateTime();

        // test default filters
        QList<QOrganizerItemFilter> filterList;
        filterList.append(QOrganizerItemChangeLogFilter());

        QOrganizerItemDateTimePeriodFilter a;
        a.setStartPeriod(time);
        a.setEndPeriod(time.addDays(1));
        filterList.append(a);

        QOrganizerItemDetailFilter b;
        b.setDetailDefinitionName(QOrganizerItemDisplayLabel::DefinitionName);
        QVariant input("this is display label");
        b.setValue(input);
        filterList.append(b);

        QOrganizerItemDetailRangeFilter c;
        c.setDetailDefinitionName(QOrganizerItemRecurrence::DefinitionName);
        QVariant min(time);
        QVariant max(time.addDays(2));
        c.setRange(min,max);
        filterList.append(c);

        QOrganizerItemInvalidFilter e;
        filterList.append(e);

        QOrganizerItemLocalIdFilter f;
        f.insert(m_items[0].itemLocalId);
        f.insert(m_items[1].itemLocalId);
        filterList.append(f);

        QOrganizerItemFilter filter;

        QList<QOrganizerItemLocalId> list;
        const int filterCount = filterList.count();
        for(int i=0;i<filterCount;++i) {

            filter = filterList[i];
            // if filter is supported
            if(manager->isFilterSupported(filter)) {
                list = manager->itemIds(filter);
                // if the searching fails
                if(manager->error()!=QOrganizerItemManager::NoError) {
                    qDebug()<<"Search filter: "<<filter.type()<<" failed";
                    result = false;
                    break;
                } else {
                    qDebug()<<"Found: "<<list.count()<<" items with filter: "<<filter.type();
                }
            } else {
                qDebug()<<"Filter "<<filter.type()<<" not supported by the datastore: "<<m_calendarStore;
            }
        }
    } // datastore creation

    // write the result
    g_pResult->StepPassed("ItemSearching", result);
}


 /*
  * TestImportingVcardToContact function
  */
void PimTest::TestImportingVcardToContact()
{
    MWTS_ENTER;

    bool result = true;
    QContact contact;
    result = ImportContact(contact);
    if(!result) {
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }
    // create contact store
    result = CreateContactDatastore();
    if(!result) {
        qDebug()<<"Data store creation failed: "<<m_calendarStore;
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }
    // find created manager
    QContactManager *manager = FindContactDataStore();

    qDebug()<<"Make sure that contact is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<contact.details().count();

    contact = manager->compatibleContact(contact);

    qDebug()<<"Detail count after pruning: "<<contact.details().count();

    result = manager->saveContact(&contact);
    if(!result || manager->error() != QContactManager::NoError) {
        qDebug()<<"Contact saving failed from: "<<m_calendarStore<<" error: "<<manager->error();
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    qDebug()<<"Contact saving succeeded";

    QContact tmp = manager->contact(contact.localId());
    if(manager->error() != QContactManager::NoError) {
        qDebug()<<"Contact fetching failed from: "<<m_calendarStore<<" error: "<<manager->error();
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    qDebug()<<"Fetching saved contact and verifying";

    if(contact!=tmp) {
        qDebug()<<"Contact matching failed from: "<<m_calendarStore;
        g_pResult->StepPassed("ImportingVcardToContact", false);
        return;
    }

    qDebug()<<"Verified";

    // remove contact from database
    result = manager->removeContact(tmp.localId());
    if(!result || manager->error() != QContactManager::NoError) {
        qDebug()<<"Contact removing failed from: "<<m_calendarStore<<" error: "<<manager->error();
        g_pResult->StepPassed("ImportingVcardToContact", false);
        return;
    }

    qDebug()<<"Contact removed";

    g_pResult->StepPassed("ImportingVcardToContact", true);
}

/**
  * TestExportingVcardFromContact function
  */
void PimTest::TestExportingVcardFromContact()
{
    MWTS_ENTER;

    bool result = true;

    // Create datastore
    result = CreateContactDatastore();
    if(!result) {
        qDebug()<<"Data store creation failed: "<<m_contactStore;
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    // find created manager
    QContactManager *manager = FindContactDataStore();
    if(!manager) {
        qDebug()<<"Cannot find contact manager: "<<m_contactStore;
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    qDebug()<<"Creating new contact for exporting";

    // Create a new contact
    result = CreateContacts();
    if(!result) {
        qDebug()<<"Contact creation failed to "<<m_contactStore;
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    // verify that contact is added
    if(m_items.count()==0) {
        qDebug()<<"Created contact not added to m_items list";
        g_pResult->StepPassed("ImportingVcardToContact", result);
        return;
    }

    QContact createdContact = manager->contact(m_items[0].contactLocalId);
    if(manager->error()!=QContactManager::NoError) {
        qDebug()<<"Contact fetching failed";
        g_pResult->StepPassed("ImportingVcardToContact", false);
        return;
    }

    qDebug()<<"New contact created, trying to export to vCard";

    // Export contact
    QList<QContact> contacts;
    contacts.append(createdContact);
    QVersitContactExporter exporter;
    result = exporter.exportContacts(contacts, QVersitDocument::VCard30Type);
    if(!result) {
        qDebug()<<"Contact exporting failed";
        g_pResult->StepPassed("TestExportingVcardFromContact", result);
        return;
    }

    qDebug()<<"Writing exporting results";

    // get output
    QList<QVersitDocument> outputDocuments = exporter.documents();
    // write to buffer
    QBuffer output;
    output.open(QBuffer::ReadWrite);
    QVersitWriter writer;
    writer.setDevice(&output);
    result = writer.startWriting(outputDocuments); // Remember to check the return value
    writer.waitForFinished();
    if(!result) {
        qDebug()<<"Versit writing failed!";
        g_pResult->StepPassed("TestExportingVcardFromContact", result);
        return;
    }
    QString resultVcard(output.buffer());

    qDebug()<<"Exported contact: "<<resultVcard;

    // remove contact from store m_calendarStore
    result = manager->removeContact(createdContact.localId());
    if(!result || manager->error()!=QContactManager::NoError) {
        qDebug()<<"Contact removing failed, error: "<<manager->error();
        g_pResult->StepPassed("TestExportingVcardFromContact", result);
        return;
    }

    qDebug()<<"Contact removed";

    m_items.removeAt(0);
    g_pResult->StepPassed("TestExportingVcardFromContact", true);
}


/**
 * TestContactItemCreationDeleting function
 */
void PimTest::TestContactItemCreationDeleting()
{
    MWTS_ENTER;

    bool result = true;

    // Create datastore
    result = CreateContactDatastore();
    // contact store created
    if(result) {
        result = CreateContacts();
        // contacts created
        if(result) {
            result = RemoveCreatedItemsFromContacts();
        }
    }
    g_pResult->StepPassed("TestContactItemCreationDeleting", result);
}

/**
 * TestContactItemCreationModifyingDeleting function
 */
void PimTest::TestContactItemCreationModifyingDeleting()
{
    MWTS_ENTER;

    bool result = true;

    // Create datastore
    result = CreateContactDatastore();
    // contact store created
    if(result) {
        result = CreateContacts();
        // contacts created
        if(result) {
            result = ModifyContacts();
            // if modified
            if(result) {
                result = RemoveCreatedItemsFromContacts();
            }
        }
    }
    g_pResult->StepPassed("TestContactItemCreationDeleting", result);
}


/**
 * CreateContactDatastore function
 */
bool PimTest::CreateContactDatastore()
{
    MWTS_ENTER;

    int err = 0;
    QContactManager *manager = 0;
    if(m_contactStore == "default") {
        qDebug()<<"Creating default store";
        // create default datastore
        manager = new QContactManager();
        err = manager->error();
    } else {
        qDebug()<<"Creating store: "<<m_contactStore;
        manager = new QContactManager(m_contactStore);
        err = manager->error();
    }

    if(manager && err == 0) {
        qDebug()<<"Store created successfully";
        // add to datastore list
        m_dataStores.append(PimDataStore(0,manager,m_contactStore));
        return true;
    } else {
        return false;
    }
}

/*
 * CreateCalendarDatastore function
 */
bool PimTest::CreateCalendarDatastore()
{
    MWTS_ENTER;

    int err = 0;
    QOrganizerItemManager *manager = 0;
    if(m_calendarStore == "default") {
        qDebug()<<"Creating default store";
        // create default datastore
        manager = new QOrganizerItemManager();
        err = manager->error();
    } else {
        qDebug()<<"Creating store: "<<m_calendarStore;
        manager = new QOrganizerItemManager(m_calendarStore);
        err = manager->error();
    }

    if(manager && err == 0) {
        qDebug()<<"Store created successfully";
        // add to datastore list
        m_dataStores.append(PimDataStore(manager,0,m_calendarStore));
        return true;
    } else {
        return false;
    }
}

/*
 * CreateContact function
 */
bool PimTest::CreateContacts()
{
    MWTS_ENTER;

    bool result = true;
    bool success;

    // find manager
    QContactManager *manager = FindContactDataStore();

    for(int i=0;i<m_numberOfItems;++i) {

         qDebug()<<"Creating contact no. "<<(i+1);

         QContact contact;
         AddContactInformation(contact);

         qDebug()<<"Prune contact for the contact datastore (makes sure that contact can be added).";
         qDebug()<<"Detail count before pruning: "<<contact.details().count();

         contact = manager->compatibleContact(contact);

         qDebug()<<"Detail count after pruning: "<<contact.details().count();

         // Try to save the contact
         success = manager->saveContact(&contact);
         if(!success || manager->error()!=QContactManager::NoError) {
             qDebug()<<"Contact addition failed, error"<<manager->error();
             result = false;
         }

         qDebug()<<"Contact no. "<<(i+1)<<" saving succeeded";

         // add to cache
         m_items.append(PimItem(0,contact.localId(),m_contactStore));
    }
    qDebug()<<m_items.count()<<" contacts created";
    return result;
}

/*
 * Modify contact information
 */
bool PimTest::ModifyContacts()
{
    MWTS_ENTER;

    bool result = true;
    bool success;
    QContact storeContact;

    // get manager
    QContactManager *manager = FindContactDataStore();

    const int count = m_items.count();
    for(int i=0;i<count;++i) {
        // modify only items that this asset has added
        if(m_contactStore==m_items[i].dataStoreName) {

           qDebug()<<"Modifying contact no. "<<i+1;

           // fetch contact from the contact database
           storeContact = manager->contact(m_items[i].contactLocalId);

           // if error occurs
           if(manager->error()!=QContactManager::NoError) {
               qDebug()<<"Contact fetching failed!";
               result = false;
               break;
           }

           // clear all details
           storeContact.clearDetails();

           // add new details
           AddContactInformation(storeContact);

           qDebug()<<"Make sure that contact is suitable for the manager by pruning";
           qDebug()<<"Detail count before pruning: "<<storeContact.details().count();

           storeContact = manager->compatibleContact(storeContact);

           qDebug()<<"Detail count after pruning: "<<storeContact.details().count();

           success = manager->saveContact(&storeContact);
           if(!success || manager->error()!=QContactManager::NoError) {
               qDebug()<<"Contact information update failed, error"<<manager->error();
               result = false;
               break;
           }

           qDebug()<<"Contact no. "<<i+1<<" modification succeeded";
           } // if
    } // for
    return result;
}


/*
 * add contact information
 */
void PimTest::AddContactInformation(QContact &target)
{
    MWTS_ENTER;
    qDebug();

    QList<QContactDetail> contactDetails;

    QContactAddress address;
    address.setCountry("Finland");
    address.setLocality("Locality");
    address.setPostcode("12345");
    address.setPostOfficeBox("pobox");
    address.setRegion("region");
    address.setStreet("street");
    contactDetails.append(address);

    QContactAnniversary anniversary;
    QDateTime time;
    anniversary.setOriginalDate(time.currentDateTime().date());
    anniversary.setEvent("anniversary");
    contactDetails.append(anniversary);

    QContactAvatar avatar;
    avatar.setImageUrl(QUrl(":/qt.png"));
    contactDetails.append(avatar);

    QContactBirthday birthday;
    birthday.setDate(time.currentDateTime().date());
    contactDetails.append(birthday);

    // Not used QContactDisplayLabel display;
    // If detail is a QContactDisplayLabel, the contact will not be updated, and the function will return false.

    QContactEmailAddress emailAddress;
    emailAddress.setEmailAddress("tester@tesinghouse.fi");
    contactDetails.append(emailAddress);

    QContactFamily contactFamily;
    QStringList list;
    list.append("child 1");
    contactFamily.setChildren(list);
    contactFamily.setSpouse("wife");
    contactDetails.append(contactFamily);

    QContactGender gender;
    gender.setGender("Male");
    contactDetails.append(gender);

    QContactGeoLocation geoLocation;
    geoLocation.setAccuracy (1.0f);
    geoLocation.setAltitude (1.0f);
    geoLocation.setAltitudeAccuracy (1.0f);
    geoLocation.setHeading (20.0f);
    geoLocation.setLabel ("myGeoLoc");
    geoLocation.setLatitude (60.750274f);
    geoLocation.setLongitude (142.499852f);
    geoLocation.setSpeed (20.0f);
    geoLocation.setTimestamp (time.currentDateTime());
    contactDetails.append(geoLocation);

    QContactGlobalPresence globalPrecence;
    globalPrecence.setCustomMessage ("custom message");
    globalPrecence.setNickname("nick");
    globalPrecence.setPresenceState(QContactPresence::PresenceAvailable);
    globalPrecence.setPresenceStateImageUrl(QUrl(":/qt.png"));
    globalPrecence.setPresenceStateText ("state text");
    globalPrecence.setTimestamp(time.currentDateTime());
    contactDetails.append(globalPrecence);

    QContactGuid guid;
    guid.setGuid("testguid");
    contactDetails.append(guid);

    QContactName name;
    name.setCustomLabel("custom label");
    name.setFirstName ("John");
    name.setLastName ("Tester");
    name.setMiddleName ("Uber");
    name.setPrefix("prefix");
    name.setSuffix("suffix");
    contactDetails.append(name);

    QContactNickname nickName;
    nickName.setNickname("T");
    contactDetails.append(nickName);

    QContactNote note;
    note.setNote("this is test contact");
    contactDetails.append(note);

    QContactOnlineAccount onlineAccount;
    onlineAccount. setAccountUri("uri");
    QStringList cabList;
    onlineAccount.setCapabilities(cabList);
    onlineAccount.setServiceProvider("unknown");
    contactDetails.append(onlineAccount);

    QContactOrganization organization;
    organization.setAssistantName("assistant");
    QStringList depList;
    depList.append("tes unit");
    organization.setDepartment(depList);
    organization.setLocation("somewhere");
    organization.setLogoUrl(QUrl("logo"));
    organization.setName("Test house");
    organization.setRole("Ruler");
    organization.setTitle("Master tester");
    contactDetails.append(organization);

    QContactPhoneNumber phoneNumber;
    phoneNumber.setNumber("+35801123123");
    contactDetails.append(phoneNumber);

    QContactPresence precence;
    precence.setCustomMessage("custom message");
    precence.setNickname("nick");
    precence.setPresenceState(QContactPresence::PresenceAvailable);
    precence.setPresenceStateImageUrl(QUrl(":/qt.png"));
    precence.setPresenceStateText("precence state text");
    precence.setTimestamp(time.currentDateTime());
    contactDetails.append(precence);

    QContactRingtone ringtone;
    ringtone.setAudioRingtoneUrl(QUrl("audio ringtone"));
    contactDetails.append(ringtone);

    QContactSyncTarget syncTarget;
    syncTarget.setSyncTarget("sync target");
    contactDetails.append(syncTarget);

    QContactTag tag;
    tag.setTag("for testing");
    contactDetails.append(tag);

    QContactThumbnail thumbnail;
    thumbnail.setThumbnail(QImage("no_image"));
    contactDetails.append(thumbnail);

    QContactTimestamp timeStamp;
    timeStamp.setCreated(time.currentDateTime());
    timeStamp.setLastModified(time.currentDateTime());
    contactDetails.append(timeStamp);

    QContactType type;
    type.setType("Contact");
    contactDetails.append(type);

    QContactUrl url;
    url.setUrl("contact url");
    contactDetails.append(url);

    qDebug()<<"Detail count to be added: "<<contactDetails.count();

    bool success;
    const int count = contactDetails.count();
    for(int j=0;j<count;++j) {
        success = target.saveDetail(&contactDetails[j]);
        if(!success) {
            qDebug()<<"Detail: "<<contactDetails[j].definitionName()<<" addition failed!";
        } else {
            qDebug()<<"Detail: "<<contactDetails[j].definitionName()<<" succeeded!";
        }
    }
    contactDetails.clear();
}


/*
 * ImportContact function
 */
bool PimTest::ImportContact(QContact &contact)
{
    MWTS_ENTER;

    bool result;
    QBuffer input;
    input.open(QBuffer::ReadWrite);
    input.write(m_inputVCard);
    input.seek(0);

    qDebug()<<"Importing vCard:\n"<<m_inputVCard;

    // read vCard
    QVersitReader reader;
    reader.setDevice(&input);
    result = reader.startReading();
    reader.waitForFinished();
    if(!result) {
        qDebug()<<"vCard reading failed";
        return false;
    }

    qDebug()<<"Reading succeeded";

    // reading succeeded
    QList<QVersitDocument> inputDocuments = reader.results();

    // importer
    QVersitContactImporter importer;
    result = importer.importDocuments(inputDocuments);
    if(!result) {
        qDebug()<<"vCard importing failed";
        return false;
    }

    // get contacts
    QList<QContact> contacts = importer.contacts();
    if(contacts.count()==0) {
        qDebug()<<"0 contacts were imported";
        return false;
    }

    // vCard contains only 1 contact
    contact = contacts[0];
    return true;
}


/**
 * CreateCalendarItems function
 */
bool PimTest::CreateCalendarItems()
{
    MWTS_ENTER;

    bool success = true;
    QOrganizerItemLocalId createdId; // <- not used in this

    // find manager
    QOrganizerItemManager *manager = FindCalendarDataStore();

    // number of items
    for(int i=0;i<m_numberOfItems;++i) {

        qDebug()<<"Creating item no. "<<i+1;

        // define type
        switch(m_calendarItemType) {

        case Event:
            success = CreateEventItem(manager,createdId);
            break;
        case EventOccurrence:
            success = CreateEventOccurrenceItem(manager,createdId);
            break;
        case Journal:
            success = CreateJournalItem(manager,createdId);
            break;
        case Note:
            success = CreateNoteItem(manager,createdId);
            break;
        case Todo:
            success = CreateTodoItem(manager,createdId);
            break;
        case TodoOccurrence:
            success = CreateTodoOccurrenceItem(manager,createdId);
            break;
        default:
            qCritical()<<"Invalid calendar item type";
            success = false;
            break;
        }

        if(!success) {
            qDebug()<<"Item creation failed, datastore: "<<m_calendarStore;
            break;
        } else {
            qDebug()<<"Item was successfully created, datastore: "<<m_calendarStore;
        }
    } // for

    qDebug()<<m_items.count()<<" calendar items created";
    return success;
}

/**
 * ModifyItems function
 */
bool PimTest::ModifyCalendarItems()
{
    MWTS_ENTER;

    bool result = true;
    PimItem pimCalItem;

    // find datastore. If store is not in the dataStores list, it is created there.
    QOrganizerItemManager *manager = FindCalendarDataStore();

    // modify only items that this asset has added
    const int count = m_items.count();
    for(int i=0;i<count;++i) {

        pimCalItem = m_items[i];
        // modify items from one datastore  (type is defined in creation phase)
        if(pimCalItem.dataStoreName==m_calendarStore && pimCalItem.itemType==m_calendarItemType) {

            qDebug()<<"Modifying item: "<<i+1<<" type: "<<pimCalItem.itemType<<" in datastore: "<<pimCalItem.dataStoreName;

            switch(m_calendarItemType) {

            case Event:
                result = ModifyEventItem(pimCalItem.itemLocalId,manager);
                break;
            case EventOccurrence:
                result = ModifyEventOccurrenceItem(pimCalItem.itemLocalId,manager);
                break;
            case Journal:
                result = true;  // no difference to QOrganizerItem, modification is verified in creation phace
                qDebug()<<"Journal item modifying succeeded";
                break;
            case Note:
                result = true;  // no difference to QOrganizerItem, modification is verified in creation phace
                qDebug()<<"Note item modifying succeeded";
                break;
            case Todo:
                result = ModifyTodoItem(pimCalItem.itemLocalId,manager);
                break;
            case TodoOccurrence:
                result = ModifyTodoOccurrenceItem(pimCalItem.itemLocalId,manager);
                break;
            default:
                qCritical()<<"Invalid calendar item type";
                result = false;
                break;
            }

            // if item modification fails
            if(!result) {
                result = false;
                break;
            }
        } else {
            qDebug()<<"Current datastore: "<<m_calendarStore<<" and item type: "<<m_calendarItemType;
            qDebug()<<"Ignoring item: "<<i+1<<" wrong type: "<<pimCalItem.itemType<<" or datastore: "<<pimCalItem.dataStoreName;
        }
    } // for
    return result;
}

/**
 * RemoveCreatedItemsFromContacts function
 */
bool PimTest::RemoveCreatedItemsFromContacts()
{
    MWTS_ENTER;

    bool result = true;
    bool removeSuccess;

    // find datastore.
    QContactManager *manager = FindContactDataStore();
    if(!manager) {
        qDebug()<<"Contact store not created";
        return false;
    }

    // remove all items from this manager
    const int count = m_items.count();
    for(int i=0;i<count;++i) {
        if(m_contactStore==m_items[i].dataStoreName) {

            removeSuccess = manager->removeContact(m_items[i].contactLocalId);
            if(!removeSuccess || manager->error()!=QContactManager::NoError) {
                qDebug()<<"item removing failed from: "<<m_contactStore<<" error: "<<manager->error();
                result = false;
            }
            // don't stop even if the removing from the datastore fails
            qDebug()<<"Contact item: "<<i+1<<" removed";
        }
    }
    m_items.clear();
    return result;
}

/**
 * RemoveCreatedItemsFromCalendar function
 */
bool PimTest::RemoveCreatedItemsFromCalendar()
{
    MWTS_ENTER;

    bool result = true;
    bool removeSuccess;

    // find manager
    QOrganizerItemManager *manager = FindCalendarDataStore();
    if(!manager) {
        qDebug()<<"Calendar store not created";
        return false;
    }

    // remove all items from this manager
    const int count = m_items.count();
    for(int i=0;i<count;++i) {
        if(m_calendarStore==m_items[i].dataStoreName) {
            removeSuccess = manager->removeItem(m_items[i].itemLocalId);
            if(!removeSuccess || manager->error()!=QOrganizerItemManager::NoError) {
                qDebug()<<"Item removing failed from: "<<m_calendarStore<<" error: "<<manager->error();
                result = false;
            }
            // don't stop even if the removing from the datastore fails
            qDebug()<<"Calendar item: "<<i+1<<" removed";
        }
    }
    m_items.clear();
    return result;
}


/**
 * ClearManagers function
 */
void PimTest::ClearManagers()
{
    MWTS_ENTER;

    // make sure that all managers are deleted
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i) {
        if(m_dataStores[i].calendarManager) {
            delete m_dataStores[i].calendarManager;
        }
        if(m_dataStores[i].contactManager) {
            delete m_dataStores[i].contactManager;
        }
    }
    m_dataStores.clear();
}


/**
 * FindDataStore function
 */
QOrganizerItemManager* PimTest::FindCalendarDataStore()
{
    MWTS_ENTER;

    qDebug()<<"Finding calendar store: "<<m_calendarStore;

    QOrganizerItemManager *manager = 0;
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i) {
        if(m_dataStores[i].dataStoreName==m_calendarStore) {
            qDebug()<<"Store found!";
            manager = m_dataStores[i].calendarManager;
            break;
        }
    }
    return manager;
}

/**
 * FindDataStore function
 */
QContactManager* PimTest::FindContactDataStore()
{
    MWTS_ENTER;

    qDebug()<<"Finding contact store: "<<m_contactStore;

    QContactManager *manager = 0;
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i) {
        if(m_dataStores[i].dataStoreName==m_contactStore) {
            qDebug()<<"Store found!";
            manager = m_dataStores[i].contactManager;
            break;
        }
    }
    return manager;
}

/**
 * AddOrganizerItemData function
 */
bool PimTest::AddOrganizerItemData(QOrganizerItem &item)
{
    MWTS_ENTER;

    // first clear item
    item.clearComments();
    //item->clearDetails();

    // verify that all is gone (type detail is left)
    if(item.comments().count()>0 || item.details().count()>1 /*|| !item->isEmpty()*/ ) {
        qDebug()<<"Item clearing failed";
        return false;
    }

    // Add item information y
    item.addComment(m_comment);
    // set description
    item.setDescription(m_description);
    // set display label
    item.setDisplayLabel(m_label);
    return true;
}


/**
 * VerifyOrganizerItemData function
 */
bool PimTest::VerifyOrganizerItemData(QOrganizerItem *item)
{
    MWTS_ENTER;

    // veridy comments
    if(!item->comments().contains(m_comment)) {
        qDebug()<<"Comment addition failed";
        return false;
    }

    // verify description
    if(item->description()!=m_description) {
        qDebug()<<"Description addition failed";
        return false;
    }

    // verify label
    if(item->displayLabel()!=m_label) {
        qDebug()<<"Display label addition failed";
        return false;
    }
    qDebug()<<"OrganizerItem Data verified (base class of all organizer items)";
    return true;
}


/**
 * Creates a specific calendar item
 *
 * NOT TESTTED YET
 * exception rules, recurrrence rules, set exception rules, set recurrence rules,
 *
 */
bool PimTest::CreateEventItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerEvent item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        // Event specific data
        m_dateTime = m_dateTime.currentDateTime();
        m_finishedDate = m_finishedDate.currentDateTime().addSecs(60*30); // add 30 minutes to end time
        m_dates.append(m_dateTime.date());
        m_dates.append(m_dateTime.addDays(1).date());
        m_dates.append(m_dateTime.addDays(2).date());
        m_exceptionDates.append(m_dateTime.addDays(1).date());
        m_priority = QOrganizerItemPriority::ExtremelyHighPriority;
        m_locationAddress = "test street 12a4";
        m_locationName = "Test house";
        m_locCoordinates = "17.750274;142.499852";

        qDebug()<<"Adding and verifying event data";

        // add data to item
        AddEventItemData(item);

        // verify added data
        result = VerifyEventItemData(&item);
        if(!result) {
            qDebug()<<"Item verification failed";
            return false;
        }

        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();
        qDebug()<<"Trying to save the item";

        // try to save the item
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Event item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        if(manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Modified item fetching failed. error: "<<manager->error();
            return false;
        }

        QOrganizerEvent fetchedItem = static_cast<QOrganizerEvent>(tmp);
        result = VerifyEventItemData(&fetchedItem);
        if(!result) {
            qDebug()<<"Item verification failed after saving and fetching";
            return false;
        }

    } else {
       qDebug()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
       return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(),0,m_calendarStore,0));
    createdItem = item.localId();
    return true;
}

/**
 * Modifes specific calendar item and verifies modification from the database
 */
bool PimTest::ModifyEventItem(QOrganizerItemLocalId itemId, QOrganizerItemManager *manager)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to event
    QOrganizerEvent event = static_cast<QOrganizerEvent>(tmp);

    // add modified data
    m_dateTime = m_dateTime.currentDateTime();
    m_finishedDate = m_finishedDate.currentDateTime().addSecs(60*60); // add 60 minutes to end time
    m_dates.append(m_dateTime.date().addDays(2));
    m_dates.append(m_dateTime.addDays(3).date());
    m_dates.append(m_dateTime.addDays(4).date());
    m_exceptionDates.append(m_dateTime.addDays(3).date());
    m_priority = QOrganizerItemPriority::VeryLowPriority;
    m_locationAddress = "modificated test street 12a4";
    m_locationName = "modificated Test house";
    m_locCoordinates = "80.750274;142.499852";

    qDebug()<<"Adding and verifying event data";

    // add data to item
    AddEventItemData(event);

    qDebug()<<"Make sure that contact is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.localId());
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Modified item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerEvent fetchedItem = static_cast<QOrganizerEvent>(tmp);
    result = VerifyEventItemData(&fetchedItem);
    if(!result) {
        qDebug()<<"Item verification failed after saving and fetching";
        return false;
    }

    qDebug()<<"Event item modifying succeeded";
    return true;
}




/**
 * Creates a specific calendar item
 *
 * NOT TESTED YET
 * parentlocal id
 * set parent local id
 */
bool PimTest::CreateEventOccurrenceItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerEventOccurrence item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        // Add QOrganizerEventOccurrence specific data
        m_dateTime = m_dateTime.currentDateTime();
        m_finishedDate = m_dateTime.addSecs(60*30);
        m_priority = QOrganizerItemPriority::HighPriority;
        m_locationAddress = "test street 12a4";
        m_locationName = "Test house";
        m_locCoordinates = "17.750274;142.499852";

        qDebug()<<"Creating parent item for the event occurrance";

        QOrganizerItemLocalId parentId;
        // parent item needs to be created for the occurrence
        result = CreateEventItem(manager,parentId);
        if(!result) {
            qDebug()<<"Parent item creation failed";
            return false;
        }
        m_parentId = parentId;

        qDebug()<<"Parent item created, local id: "<<m_parentId;
        qDebug()<<"Adding and verifying event occurrence data";

        // add data to the item
        AddEventOccurrenceItemData(item);

        // verify addition
        result = VerifyEventOccurrenceItemData(&item);
        if(!result) {
            qDebug()<<"Item verification failed";
            return false;
        }

        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();
        qDebug()<<"Trying to save the item";

        // try to save the event
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError ) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Event occurrence saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        QOrganizerEventOccurrence fetchedItem = static_cast<QOrganizerEventOccurrence>(tmp);
        result = VerifyEventOccurrenceItemData(&fetchedItem);
        if(!result) {
            qDebug()<<"Item verification failed after saving and fetching";
            return false;
        }
    } else {
        qDebug()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(), 0, m_calendarStore,1));
    createdItem = item.localId();
    return true;
}

/**
 * Modifes specific calendar item and verifies modification from the database
 */
bool PimTest::ModifyEventOccurrenceItem(QOrganizerItemLocalId itemId, QOrganizerItemManager *manager)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerEventOccurrence
    QOrganizerEventOccurrence event = static_cast<QOrganizerEventOccurrence>(tmp);

    // Add modified QOrganizerEventOccurrence data
    m_dateTime = m_dateTime.currentDateTime().addMSecs(60);
    m_finishedDate = m_dateTime.addSecs(50*60);
    m_priority = QOrganizerItemPriority::LowestPriority;
    m_locationAddress = "modified test street 12a4";
    m_locationName = "modified Test house";
    m_locCoordinates = "60.750274;142.499852";
    m_parentId = event.parentLocalId();

    // add data to the item
    AddEventOccurrenceItemData(event);

    qDebug()<<"Make sure that contact is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.localId());
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerEventOccurrence fetchedItem = static_cast<QOrganizerEventOccurrence>(tmp);
    result = VerifyEventOccurrenceItemData(&fetchedItem);
    if(!result) {
        qDebug()<<"Item verification failed after saving and fetching";
        return false;
    }
    qDebug()<<"Event occurrence item modifying succeeded";
    return true;
}




/**
 * Creates a specific calendar item
 */
bool PimTest::CreateJournalItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result;

    QOrganizerJournal item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        // test QOrganizerJournal specific functions. Journal has only one function
        qDebug()<<"Occurrence date: "<<item.dateTime();
        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the event
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Journal item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        if(manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item fetching failed. Error: "<<manager->error();
            return false;
        }

        result = VerifyOrganizerItemData(&tmp);
        if(!result) {
            qDebug()<<"CreateJournalItem: item verification failed after saving and fetching";
            return false;
        }

    } else {
        qDebug()<<"CreateJournalItem: "<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(), 0, m_calendarStore,2));
    createdItem = item.localId();
    return true;
}

/**
 * Creates a specific calendar item
 */
bool PimTest::CreateNoteItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result;

    QOrganizerNote item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the event
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Note item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        if(manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item fetching failed. Error: "<<manager->error();
            return false;
        }

        result = VerifyOrganizerItemData(&tmp);
        if(!result) {
            qDebug()<<"Item verification failed after saving and fetching";
            return false;
        }

    } else {
        qDebug()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(), 0, m_calendarStore,3));
    createdItem = item.localId();
    return true;
}


/**
 * Creates a specific calendar item
 * NOT TESTED YET
 * expception rules, recurrence rules
 * set exept rules,  set recurrence rules,
 */
bool PimTest::CreateTodoItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerTodo item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        // Add QOrganizerTodo specific data
        m_finishedDate = m_finishedDate.currentDateTime().addDays(2);
        m_dateTime = m_dateTime.currentDateTime();
        m_dates.append(m_dateTime.date());
        m_dates.append(m_dateTime.addDays(1).date());
        m_dates.append(m_dateTime.addDays(2).date());
        m_exceptionDates.append(m_dateTime.addDays(1).date());
        m_priority = QOrganizerItemPriority::HighPriority;
        m_progress = 50;
        m_status = QOrganizerTodoProgress::StatusInProgress;

        // add data
        AddTodoItemData(item);

        // verify addition
        result = VerifyTodoItemData(&item);
        if(!result) {
            qDebug()<<"Data verification failed";
            return false;
        }

        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the event
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Todo item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        QOrganizerTodo fetchedItem = static_cast<QOrganizerTodo>(tmp);
        bool result = VerifyTodoItemData(&fetchedItem);
        if(!result) {
            qDebug()<<"Item verification failed after saving and fetching";
            return false;
        }

    } else {
        qDebug()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(),0 ,m_calendarStore,4));
    createdItem = item.localId();
    return true;
}


/**
 * Modifes specific calendar item and verifies modification from the database
 */
bool PimTest::ModifyTodoItem(QOrganizerItemLocalId itemId, QOrganizerItemManager *manager)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"ModifyTodoItem: item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerTodo
    QOrganizerTodo event = static_cast<QOrganizerTodo>(tmp);


    // Add modified QOrganizerTodo data
    m_finishedDate = m_finishedDate.currentDateTime().addDays(3);
    m_dateTime = m_dateTime.currentDateTime();
    m_dates.append(m_dateTime.date().addDays(2));
    m_dates.append(m_dateTime.addDays(3).date());
    m_dates.append(m_dateTime.addDays(4).date());
    m_exceptionDates.append(m_dateTime.addDays(3).date());
    m_priority = QOrganizerItemPriority::HighestPriority;
    m_progress = 10;
    m_status = QOrganizerTodoProgress::StatusNotStarted;

    // add data
    AddTodoItemData(event);

    qDebug()<<"Make sure that contact is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.localId());
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerTodo fetchedItem = static_cast<QOrganizerTodo>(tmp);
    result = VerifyTodoItemData(&fetchedItem);
    if(!result) {
        qDebug()<<"Item verification failed after saving and fetching";
        return false;
     }
    qDebug()<<"Todo item modifying succeeded";
    return true;
}

/**
 * Creates a specific calendar item
 *
 * NOT TESTED YET
 * parent local id
 * set parent local id
 */
bool PimTest::CreateTodoOccurrenceItem(QOrganizerItemManager *manager, QOrganizerItemLocalId &createdItem)
{
    MWTS_ENTER;

    bool result = true;
    QOrganizerTodoOccurrence item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type())) {

        m_comment = "This is a comment";
        m_description ="this is description";
        m_label = "this is display label";

        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item)) {
            return false;
        }

        // Test QOrganizerTodoOccurrence specific functions
        m_dateTime = m_dateTime.currentDateTime();
        m_finishedDate = m_dateTime.addDays(2);
        m_status = QOrganizerTodoProgress::StatusInProgress;
        m_priority = QOrganizerItemPriority::HighPriority;

        qDebug()<<"Creating parent item for the todo occurrance";

        QOrganizerItemLocalId parentId;
        // parent item needs to be created for the occurrence
        result = CreateTodoItem(manager,parentId);
        if(!result) {
            qDebug()<<"Parent item creation failed";
            return false;
        }
        m_parentId = parentId;

        // add item data
        AddTodoOccurrenceItemData(item);

        // verify addition
        result = VerifyTodoOccurrenceItemData(&item);
        if(!result) {
            qDebug()<<"Data verification failed";
            return false;
        }

        qDebug()<<"Make sure that contact is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the event
        result = manager->saveItem(&item);
        if(!result || manager->error()!=QOrganizerItemManager::NoError) {
            qDebug()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Todo occurrance item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.localId());
        QOrganizerTodoOccurrence fetchedItem = static_cast<QOrganizerTodoOccurrence>(tmp);
        bool result = VerifyTodoOccurrenceItemData(&fetchedItem);
        if(!result) {
            qDebug()<<"Item verification failed after saving and fetching";
            return false;
        }
    } else {
        qDebug()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.localId(),0,m_calendarStore,5));
    createdItem = item.localId();
    return true;
}




/**
 * Modifes specific calendar item and verifies modification from the database
 */
bool PimTest::ModifyTodoOccurrenceItem(QOrganizerItemLocalId itemId, QOrganizerItemManager *manager)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerTodoOccurrence
    QOrganizerTodoOccurrence event = static_cast<QOrganizerTodoOccurrence>(tmp);

    // Test QOrganizerTodoOccurrence specific functions
    m_dateTime = m_dateTime.currentDateTime();
    m_finishedDate = m_dateTime.addDays(4);
    m_status = QOrganizerTodoProgress::StatusNotStarted;
    m_priority = QOrganizerItemPriority::LowestPriority;
    m_parentId = event.parentLocalId();

    // add item data
    AddTodoOccurrenceItemData(event);

    qDebug()<<"Make sure that contact is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.localId());
    if(manager->error()!=QOrganizerItemManager::NoError) {
        qDebug()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerTodoOccurrence fetchedItem = static_cast<QOrganizerTodoOccurrence>(tmp);
    result = VerifyTodoOccurrenceItemData(&fetchedItem);
    if(!result) {
        qDebug()<<"Item verification failed after saving and fetching";
        return false;
    }

    qDebug()<<"Todo occurrance item modifying succeeded";
    return true;
}

/**
 * Verifies event item data
 */
bool PimTest::VerifyEventItemData(QOrganizerEvent *item)
{
    MWTS_ENTER;

    QList<QDate> tmpDates;

    // verify start time
    if(item->startDateTime()!=m_dateTime) {
        qDebug()<<"Start date addition failed";
        return false;
    }

    // verify end time
    if(item->endDateTime()!=m_finishedDate) {
        qDebug()<<"End date addition failed";
        return false;
    }

    // verify recurrence dates
    tmpDates = item->recurrenceDates();
    if(m_dates.count()!=tmpDates.count()) {
        qDebug()<<"Recurrence date addition failed";
        return false;
    } else {
        if(m_dates[0] != tmpDates[0] ||
           m_dates[1] != tmpDates[1] ||
           m_dates[2] != tmpDates[2] ) {
            qDebug()<<"Recurrence date addition failed";
            return false;
        }
    }

    // verify exception dates
    tmpDates.clear();
    tmpDates = item->exceptionDates();
    if(m_exceptionDates.count()!=tmpDates.count()) {
        qDebug()<<"Exception date addition failed";
        return false;
    } else {
        if(m_exceptionDates[0] != tmpDates[0] ) {
            qDebug()<<"eException date addition failed";
            return false;
        }
    }

    // verify priority
    if(item->priority()!=m_priority) {
        qDebug()<<"Priority addition failed";
        return false;
    }

    // verify location address
    if(item->locationAddress()!=m_locationAddress) {
        qDebug()<<"Location address addition failed";
        return false;
    }

    // verify location name
    if(item->locationName()!=m_locationName) {
        qDebug()<<"Location name addition failed";
        return false;
    }

    // verify location coordinates
    if(item->locationGeoCoordinates()!=m_locCoordinates) {
        qDebug()<<"Geocoordinate addition failed";
        return false;
    }

    qDebug()<<"QOrganizerEvent Data verified";
    return true;
}


/**
 * Adds event item data
 */
void PimTest::AddEventItemData(QOrganizerEvent &item)
{
    MWTS_ENTER;

    // add start time
    item.setStartDateTime(m_dateTime);
    // add end time
    item.setEndDateTime(m_finishedDate);
    // set recurrence dates
    item.setRecurrenceDates(m_dates);
    // set exception dates
    item.setExceptionDates(m_exceptionDates);
    // set priority
    item.setPriority(m_priority);
    // set location address
    item.setLocationAddress(m_locationAddress);
    // set alocation name
    item.setLocationName(m_locationName);
    // set and verify location geocoordinates
    item.setLocationGeoCoordinates(m_locCoordinates);
}


/**
 * Verifies event occurrence item data
 */
bool PimTest::VerifyEventOccurrenceItemData(QOrganizerEventOccurrence *item)
{
    MWTS_ENTER;

    // verify parent id
    if(item->parentLocalId()!=m_parentId) {
        qDebug()<<"Parent id addition failed";
        return false;
    }

    // verify original date
    if(item->originalDate()!=m_dateTime.date()) {
        qDebug()<<"Original date addition failed";
        return false;
    }

    // verify end time
    if(item->endDateTime()!=m_finishedDate) {
        qDebug()<<"End date addition failed";
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority) {
        qDebug()<<"Priority addition failed";
        return false;
    }

    // verify location address
    if(item->locationAddress()!=m_locationAddress) {
        qDebug()<<"Location address addition failed";
        return false;
    }

    // verify location name
    if(item->locationName()!=m_locationName) {
        qDebug()<<"Location name addition failed";
        return false;
    }

    // verify geo coordinates
    if(item->locationGeoCoordinates()!=m_locCoordinates) {
        qDebug()<<"Geocoordinate addition failed";
        return false;
    }
    qDebug()<<"QOrganizerEventOccurrence Data verified";
    return true;
}


/**
 * Adds event occurrence item data
 */
void PimTest::AddEventOccurrenceItemData(QOrganizerEventOccurrence &item)
{
    MWTS_ENTER;

    // add parent id
    item.setParentLocalId(m_parentId);
    // Add original date
    item.setOriginalDate(m_dateTime.date());
    // add  end time
    item.setEndDateTime(m_finishedDate);
    // set priority
    item.setPriority(m_priority);
    // set and verify location address
    item.setLocationAddress(m_locationAddress);
    // set and verify location name
    item.setLocationName(m_locationName);
    // set and verify location geocoordinates
    item.setLocationGeoCoordinates(m_locCoordinates);
    // Print start date time
    qDebug()<<"Start date time: "<<item.startDateTime();
}


/**
 * Verifies event todo item data
 */
bool PimTest::VerifyTodoItemData(QOrganizerTodo *item)
{
    MWTS_ENTER;

    QList<QDate> tmpDates;

    // verify due date time
    if(item->dueDateTime()!=m_finishedDate) {
        qDebug()<<"Due date setting failed";
        return false;
    }

    // verify recurrence dates
    tmpDates = item->recurrenceDates();
    if(m_dates.count()!=tmpDates.count()) {
        qDebug()<<"Recurrence date addition failed";
        return false;
    } else {
        if(m_dates[0] != tmpDates[0] ||
           m_dates[1] != tmpDates[1] ||
           m_dates[2] != tmpDates[2] ) {
            qDebug()<<"Recurrence date addition failed";
            return false;
        }
    }

    // verify exception dates
    tmpDates = item->exceptionDates();
    if(m_dates.count()!=tmpDates.count()) {
        qDebug()<<"Exception date addition failed";
        return false;
    } else {
        if(m_dates[0] != tmpDates[0] ) {
            qDebug()<<"Exception date addition failed";
            return false;
        }
    }

    // verify priority
    if(item->priority()!=m_priority) {
        qDebug()<<"Priority addition failed";
        return false;
    }

    // verify finished date time
    if(item->finishedDateTime()!=m_finishedDate) {
        qDebug()<<"Finished date addition failed";
        return false;
    }

    // verify progress percentage
    if(item->progressPercentage()!=m_progress) {
        qDebug()<<"Progress percentage addition failed";
        return false;
    }

    // verify status
    if(item->status()!=m_status) {
        qDebug()<<"Status addition failed";
        return false;
    }
    qDebug()<<"QOrganizerTodo Data verified";
    return true;
}


/**
 * Adds event todo item data
 */
void PimTest::AddTodoItemData(QOrganizerTodo &item)
{
    MWTS_ENTER;

    // Add  due date
    item.setDueDateTime(m_finishedDate);
    // set  recurrence dates
    item.setRecurrenceDates(m_dates);
    // Print start date
    qDebug()<<"Start date time: "<<item.startDateTime();
    // set exception dates
    item.setExceptionDates(m_dates);
    // set priority
    item.setPriority(m_priority);
    // set finished date time
    item.setFinishedDateTime(m_finishedDate);
    // set progress percentage
    item.setProgressPercentage(m_progress);
    // set status setting
    item.setStatus(m_status);
}


/**
 * Verifies event todo occurrence item data
 */
bool PimTest::VerifyTodoOccurrenceItemData(QOrganizerTodoOccurrence *item)
{
    MWTS_ENTER;

    // verify parent local id
    if(item->parentLocalId()!=m_parentId) {
        qDebug()<<"Parent id setting failed";
        return false;
    }

    // verify due date time
    if(item->dueDateTime()!=m_finishedDate) {
        qDebug()<<"Due date setting failed";
        return false;
    }

    // verify status
    if(item->status()!=m_status) {
        qDebug()<<"Status addition failed";
        return false;
    }

    // verify finished date time
    if(item->finishedDateTime()!=m_finishedDate) {
        qDebug()<<"Finished date addition failed";
        return false;
    }

    // verify original date
    if(item->originalDate()!=m_dateTime.date()) {
        qDebug()<<"Original date addition failed";
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority) {
        qDebug()<<"Priority addition failed";
        return false;
    }
    qDebug()<<"QOrganizerTodoOccurrence Data verified";
    return true;
}


/**
 * Adds event todo occurrence item data
 */
void PimTest::AddTodoOccurrenceItemData(QOrganizerTodoOccurrence &item)
{
    MWTS_ENTER;

    // set parent id for this occurrence
    item.setParentLocalId(m_parentId);
    // Test and verify due date
    item.setDueDateTime(m_finishedDate);
    // Print start date
    qDebug()<<"Start date time: "<<item.startDateTime();
    // set and verify status setting
    item.setStatus(m_status);
    // set and verify finished date time
    item.setFinishedDateTime(m_finishedDate);
    // Test and verify original date
    item.setOriginalDate(m_dateTime.date());
     // set and verify priority
    item.setPriority(m_priority);
}

// eof
