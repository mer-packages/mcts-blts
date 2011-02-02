/* contactstest.cpp -- source code for ContactsTest class
 *
 * This file is part of MCTS
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com;
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

// class header
#include "contactstest.h"

// includes
#include <QContactActionFilter>
#include <QContactChangeLogFilter>
#include <QContactDetailFilter>
#include <QContactDetailRangeFilter>
#include <QContactIntersectionFilter>
#include <QContactInvalidFilter>
#include <QContactLocalIdFilter>
#include <QContactRelationshipFilter>
#include <QContactUnionFilter>
#include "versit.h"
#include "pimcontactdetailmanager.h"


/**
 * Constructor for ContactsTest class
 */
 ContactsTest::ContactsTest(MwtsConfig &dFetcher,
                            MwtsResult &result)
     : m_dataFetcher(dFetcher),
       m_result(result),
       m_measureOn(false)
 {
     MWTS_ENTER;

     // one item is always created
     m_numberOfItems = 1;

     // iteration count is always at least 1
     m_iterationCount = 1;

     // create detail manager
     m_detailManager = new PimContactDetailManager(m_dataFetcher);

     MWTS_LEAVE;
 }

 /**
  * Destructor for ContactsTest class
  */
ContactsTest::~ContactsTest()
{
    MWTS_ENTER;

    qDebug()<<"--------------------------";
    qDebug()<<"Make sure that everything is removed from data store, and all data stores are deleted";

    RemoveContacts(true);
    ClearManagers();

    m_items.clear();
    m_dataStores.clear();

    // detail manager
    delete m_detailManager;

    MWTS_LEAVE;
}


/**
 * SetNumberOfCreatedItems function
 */
void ContactsTest::SetNumberOfCreatedItems(int n)
{
    MWTS_ENTER;
    m_numberOfItems = n;
    qDebug()<<"Number of wanted items set to: "<<m_numberOfItems;
}

/**
 * SetContactDatastore function
 */
void ContactsTest::SetContactDatastore(QString name)
{
    MWTS_ENTER;
    m_contactStore = name;
    qDebug()<<"Contact datastore set to: "<<m_contactStore;
}

/**
 * CreateContactDatastore function
 */
void ContactsTest::CreateContactDatastore()
{
    MWTS_ENTER;

    QContactManager *manager = 0;
    if(m_contactStore == "default")
    {
        qDebug()<<"Creating default store";
        manager = new QContactManager();
    }
    else
    {
        qDebug()<<"Creating store: "<<m_contactStore;
        manager = new QContactManager(m_contactStore);
    }

    // if creation fails
    if(!manager || manager->error() != QContactManager::NoError)
    {
        qCritical()<<"Contact data store creation failed, error: "<<manager->error();
    }
    else
    {
        qDebug()<<"Contact data store creation succeeded";
        m_dataStores.append(PimDataStore(0,manager,m_contactStore));
    }
}


/**
 * CreateAvailableContactDatastores function
 */
void ContactsTest::CreateAvailableContactDatastores()
{
    MWTS_ENTER;

    qDebug()<<"Fetching available contact datastores";

    bool result;
    QStringList list = QContactManager::availableManagers();

    qDebug()<<"Found datastores:";

    const int count = list.count();
    for(int i=0;i<count;++i)
    {
        qDebug()<<""<<i<<" "<<list[i];
    }

    // if there is available stores
    if(count>0)
    {
        QString name;
        QString uri;
        int version;
        QString parseName;
        QMap<QString,QString> parameters;
        QMap<QString,QString> parseParameters;
        QContactManager *manager = 0;
        QContactManager *uriManager = 0;
        qDebug()<<"Creating found datastores";

        for(int i=0;i<count;++i)
        {
           name = list[i].toLower();
           qDebug()<<"----------------";
           qDebug()<<i<<": Datastore name: "<<name;
           manager = new QContactManager(name);

           // if manager can be created
           if(manager)
           {
               qDebug()<<"Manager created";
               // validate name
               if(name != manager->managerName().toLower())
               {
                   qCritical()<<"Name "<<manager->managerName()<<" invalid";
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
               result = QContactManager::parseUri(uri, &parseName, &parseParameters);
               if(!result ||
                  parseName != name ||
                  parseParameters != parameters)
               {
                   qCritical()<<"Uri parsing failed";
                   break;
               }

               // get version
               version = manager->managerVersion();
               qDebug()<<"Version: "<<version;

               qDebug()<<"Create manager again from uri: "<<uri;
               uriManager = QContactManager::fromUri(uri);
               if(!uriManager)
               {
                   qCritical()<<"Manager is NULL";
                   break;
               }
               else
               {
                   qDebug()<<"Manager created";
                   // verify managers
                   if(manager->managerName().toLower() != uriManager->managerName().toLower() ||
                      manager->managerVersion() != uriManager->managerVersion() ||
                      manager->managerUri() != uriManager->managerUri() ||
                      manager->managerParameters() != uriManager->managerParameters())
                   {
                       qCritical()<<"Manager validation failed";
                       break;
                   }
                   else
                   {
                       qDebug()<<"Uri creation validated";
                   }
               }
           }
           else
           {
               qCritical()<<"Manager is NULL";
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
}

/**
 * SearchContacts function
 *
 * NOT TESTED
 * QContactRelationshipFilter
 */
void ContactsTest::SearchContacts()
{
    MWTS_ENTER;

    QContactManager *manager = FindContactDataStore();
    if(!manager)
    {
        qCritical()<<"Contact store not created";
        return;
    }

    // test default filters
    QList<QContactFilter> filterList;

    QDateTime time;
    int day = m_dataFetcher.value("CONTACTDETAILS/day").toInt();
    int month = m_dataFetcher.value("CONTACTDETAILS/month").toInt();
    int year = m_dataFetcher.value("CONTACTDETAILS/year").toInt();
    QDate tmp(year,month,day);
    time.setDate(tmp);

    QString actionName = m_dataFetcher.value("CONTACTSEARCH/action_name").toString();
    QString defName = m_dataFetcher.value("CONTACTSEARCH/definition_name").toString();
    QContactChangeLogFilter::EventType type = (QContactChangeLogFilter::EventType)m_dataFetcher.value("CONTACTSEARCH/event_type").toInt();
    QContactFilter::MatchFlags flag = (QContactFilter::MatchFlags)m_dataFetcher.value("CONTACTSEARCH/match_flag").toInt();
    int rMax = m_dataFetcher.value("CONTACTSEARCH/range_max").toInt();
    int rMin = m_dataFetcher.value("CONTACTSEARCH/range_min").toInt();
    QContactDetailRangeFilter::RangeFlag rflag = (QContactDetailRangeFilter::RangeFlag)m_dataFetcher.value("CONTACTSEARCH/range_flag").toInt();

    qDebug()<<"Time : "<<time;
    qDebug()<<"Action name: "<<actionName;
    qDebug()<<"detail definition: "<<defName;
    qDebug()<<"Event type: "<<type;
    qDebug()<<"Match flag: "<<(int)flag;
    qDebug()<<"Range min: "<<rMin;
    qDebug()<<"Range max: "<<rMax;
    qDebug()<<"Range flag: "<<(int)rflag;

    // define filters
    QContactActionFilter action;
    action.setActionName(actionName);
    filterList.append(action);

    QContactChangeLogFilter changelog;
    changelog.setEventType(type);
    changelog.setSince(time);
    filterList.append(changelog);

    QContactDetailFilter detail;
    detail.setDetailDefinitionName(defName);
    detail.setMatchFlags(flag);
    filterList.append(detail);

    QContactDetailRangeFilter range;
    range.setDetailDefinitionName(defName);
    range.setMatchFlags(flag);
    range.setRange(rMin, rMax, rflag);
    filterList.append(range);

    QContactIntersectionFilter intersect;
    QList<QContactFilter> filters;
    filters.append(detail);
    filters.append(range);
    intersect.setFilters(filters);
    filterList.append(intersect);

    QContactInvalidFilter invalid;
    filterList.append(invalid);

    QContactLocalIdFilter localId;
    QList<QContactLocalId> ids;
    if(m_items.count()>=2)
    {
        ids.append(m_items[0].contactLocalId);
        ids.append(m_items[1].contactLocalId);
    }
    filterList.append(localId);

    //QContactRelationshipFilter relation;
    //filterList.append(relation);

    QContactUnionFilter unionfilter;
    unionfilter.setFilters(filters);
    filterList.append(unionfilter);

    QContactFilter filter;
    QList<QContact> list;
    const int filterCount = filterList.count();
    for(int i=0;i<filterCount;++i)
    {
        filter = filterList[i];
        // if filter is supported
        if(manager->isFilterSupported(filter))
        {
            list = manager->contacts(filter);
            // if the searching fails
            if(manager->error()!=QContactManager::NoError)
            {
                qDebug()<<"Search filter: "<<filter.type()<<" failed, error: "<<manager->error();
            }
            else
            {
                qDebug()<<"Found: "<<list.count()<<" items with filter: "<<filter.type();
            }
        }
        else
        {
            qDebug()<<"Filter "<<filter.type()<<" not supported by the datastore: "<<m_contactStore;
        }
    }
}


/**
 * CreateContacts function
 */
void ContactsTest::CreateContacts()
{
    MWTS_ENTER;

    bool result;

    // initialize data
    m_detailManager->initializeContactData();

    // find manager
    QContactManager *manager = FindContactDataStore();
    if(!manager)
    {
        qCritical()<<"Contact data store not created";
        return;
    }

    int counter = 0;
    // For iteration, in FUTE cases the m_iterationCount is always 1
    for(int iter=0;iter<m_iterationCount;++iter)
    {
        // Number of created items
        for(int i=0;i<m_numberOfItems;++i)
        {
             counter += iter+i;
             qDebug()<<"-------------------------------";
             qDebug()<<"Creating contact no. "<<counter;

             QContact contact;
             result = m_detailManager->addDefaultContactDetails(contact);
             if(!result) {
                 qCritical()<<"Detail addition failed";
                 break;
             }

             qDebug()<<"Prune contact for the contact datastore (makes sure that contact can be added).";
             qDebug()<<"Detail count before pruning: "<<contact.details().count();

             contact = manager->compatibleContact(contact);

             qDebug()<<"Detail count after pruning: "<<contact.details().count();
             qDebug()<<"Saving the contact";

             // Try to save the contact
             result = saveContact(*manager,contact);
             if(!result || manager->error()!=QContactManager::NoError)
             {
                 qCritical()<<"Contact addition failed, error"<<manager->error();
                 break;
             }

             // try to fetch the contact and verify data
             QContact tmp = manager->contact(contact.localId());
             if(manager->error()!=QContactManager::NoError)
             {
                 qCritical()<<"Contact fetching failed, error: "<<manager->error();
                 break;
             }
             // verify details. (false parameter means that details modified by the database are not verified. e.g. time stamp, guid))
             m_detailManager->verifyContactDetails(tmp,false);
             qDebug()<<"Contact no. "<<counter<<" saved successfully";

             // add to cache
             m_items.append(PimItem(QOrganizerItemId(),contact.localId(),m_contactStore));
        }   // number of contacts
    } // iteration
    qDebug()<<m_items.count()<<" contacts created";
}

/**
 * SaveContact function
 */
bool ContactsTest::saveContact(QContactManager& manager, QContact& contact)
{
    bool result;
    // no measurement
    if(!m_measureOn)
    {
        result = manager.saveContact(&contact);
    }
    else
    {
        int elapsed=0;
        QString res;
        // measure saving time
        m_time.start();
        result = manager.saveContact(&contact);
        elapsed = m_time.elapsed();
        res.sprintf("Contact saving with %d details",contact.details().count());
        m_result.AddMeasure(res,elapsed,"ms");
    }
    return result;
}


/**
 * Modify contact information
 */
void ContactsTest::ModifyContacts()
{
    MWTS_ENTER;

    bool result;
    QContact storeContact;

    // get manager
    QContactManager *manager = FindContactDataStore();

    int counter = 0;
    const int count = m_items.count();
    for(int i=0;i<count;++i)
    {
        // modify only items that this asset has added
        if(m_contactStore==m_items[i].dataStoreName)
        {
           counter += i+1;
           qDebug()<<"-------------------------------";
           qDebug()<<"Modifying contact no. "<<counter;

           // fetch contact from the contact database
           storeContact = manager->contact(m_items[i].contactLocalId);

           // if error occurs
           if(manager->error()!=QContactManager::NoError)
           {
               qCritical()<<"Contact fetching failed!";
               break;
           }

           // clear all details
           storeContact.clearDetails();

           // add new details
           result = m_detailManager->addDefaultContactDetails(storeContact);
           if(!result)
           {
               qCritical()<<"Detail addition failed";
               break;
           }

           qDebug()<<"Make sure that contact is suitable for the manager by pruning";
           qDebug()<<"Detail count before pruning: "<<storeContact.details().count();

           storeContact = manager->compatibleContact(storeContact);

           qDebug()<<"Detail count after pruning: "<<storeContact.details().count();

           result = manager->saveContact(&storeContact);
           if(!result || manager->error()!=QContactManager::NoError)
           {
               qCritical()<<"Contact information update failed, error"<<manager->error();
               break;
           }
           qDebug()<<"Contact no. "<<counter<<" modification succeeded";
           } // if
    } // for
}

/**
 * RemoveContacts function
 */
void ContactsTest::RemoveContacts(bool ignoreErrors)
{
    MWTS_ENTER;

    bool result;
    // find datastore.
    QContactManager *manager = FindContactDataStore();
    if(!manager)
    {
        if(!ignoreErrors)
        {
            qCritical()<<"Contact store not created";
        }
        return;
    }

    // remove all items from this manager
    const int count = m_items.count();
    PimItem item;

    qDebug()<<"Number of items in the cache: "<<m_items.count();

    for(int i=0; i<count;++i)
    {
        item = m_items[i];
        if(m_contactStore==item.dataStoreName)
        {
            result = removeContact(*manager, item.contactLocalId);
            if(!result || manager->error()!=QContactManager::NoError)
            {
                if(!ignoreErrors)
                {
                    qCritical()<<"Item removing failed from: "<<m_contactStore<<" error: "<<manager->error();
                }
            }
            else
            {
                // don't stop even if the removing from the datastore fails
                qDebug()<<"Contact item: "<<(i+1)<<" removed";
                qDebug()<<"-------------------------------";
            }
        } // if
    } // for
    qDebug()<<"Overall items left after removing in the cache: "<<m_items.count();
}

/**
 * removeContact function
 */
bool ContactsTest::removeContact(QContactManager& manager, QContactLocalId& contactId)
{
    bool result;
    // no measurement
    if(!m_measureOn)
    {
        result = manager.removeContact(contactId);
    }
    else
    {
        int elapsed=0;
        // measure removing time
        m_time.start();
        result = manager.removeContact(contactId);
        elapsed = m_time.elapsed();
        m_result.AddMeasure("Contact removing",elapsed,"ms");
    }
    return result;
}


/**
  * Imports contact from vCard
  */
void ContactsTest::ImportContactFromVcard()
{
    MWTS_ENTER;
    QContactManager *manager = FindContactDataStore();
    if(!manager)
    {
        qCritical()<<"Contact data store not created";
        return;
    }

    Versit importer;
    QList<QContact> contactList;
    QString vcardFile = g_pConfig->value("VCARD/input_file").toString();
    bool result = importer.ImportContactsFromVcard(contactList,vcardFile);
    if(!result)
    {
        qCritical()<<"Contact importing failed";
        return;
    }

    qDebug()<<"Contact importing succeeded, details...";
    QContact contact;
    const int count = contactList.count();
    for(int i=0;i<count;++i)
    {
        // check compatibility
        contact = contactList[i];
        qDebug()<<"Detail count before pruning: "<<contact.details().count();
        contact = manager->compatibleContact(contact);
        qDebug()<<"Detail count after pruning: "<<contact.details().count();
    }
}

/**
  * Exports contact to vCard
  */
void ContactsTest::ExportContactToVcard()
{
    MWTS_ENTER;

    QList<QContact> contactList;
    QContact contact;
    Versit exporter;

    QContactManager *manager = FindContactDataStore();
    if(!manager)
    {
        qCritical()<<"Contact data store not created";
        return;
    }

    const int count = m_items.count();
    if(count==0)
    {
        qWarning()<<"There are no contacts created";
    }

    for(int i=0;i<count;++i)
    {
        //  export only contacts created to this data store
        if(m_items[i].dataStoreName==m_contactStore)
        {
            // get contact
            contact = manager->contact(m_items[i].contactLocalId);
            if(manager->error()!=QContactManager::NoError)
            {
                qCritical()<<"Contact fetching from data store failed, error: "<<manager->error();
                break;
            }
            contactList.append(contact);
        }
    } // for

    // import all found contacts
    bool result = exporter.ExportContactsToVcard(contactList);
    if(!result)
    {
        qCritical()<<"Exporting failed";
    }
    else
    {
        qDebug()<<"Exporting succeeded";
    }
}


/**
 * ClearManagers function
 */
void ContactsTest::ClearManagers()
{
    MWTS_ENTER;

    // make sure that all managers are deleted
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i)
    {
        if(m_dataStores[i].contactManager)
        {
            qDebug()<<"Deleting manager: "<<m_dataStores[i].contactManager->managerName();
            delete m_dataStores[i].contactManager;
        }
    }
}


/**
 * FindDataStore function
 */
QContactManager* ContactsTest::FindContactDataStore()
{
    MWTS_ENTER;

    qDebug()<<"Finding contact store: "<<m_contactStore;

    QContactManager *manager = 0;
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i)
    {
        if(m_dataStores[i].dataStoreName==m_contactStore)
        {
            qDebug()<<"Store found!";
            manager = m_dataStores[i].contactManager;
            break;
        }
    }
    return manager;
}

// eof
