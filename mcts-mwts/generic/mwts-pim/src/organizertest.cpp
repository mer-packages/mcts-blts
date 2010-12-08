/* organizertest.cpp -- source code for OrganizerTest class
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
#include "organizertest.h"

// includes
#include <qtorganizer.h>
#include "pimorganizeritemmanager.h"

/**
 * Constructor for OrganizerTest class
 */
OrganizerTest::OrganizerTest(MwtsConfig &dFetcher,MwtsResult &result)
    : m_dataFetcher(dFetcher),
      m_result(result),
      m_measureOn(false)
{
        MWTS_ENTER;

        // create item manager
        m_itemManager = new PimOrganizerItemManager(m_dataFetcher,m_result);
        // one item is always created
        m_numberOfItems = 1;
        // iter count is always at least 1
        m_iterationCount = 1;

        MWTS_LEAVE;
}

/**
 * Destructor for OrganizerTest class
 */
OrganizerTest::~OrganizerTest()
{
        MWTS_ENTER;

        qDebug()<<"--------------------------";
        qDebug()<<"Make sure that everything is removed from data store, and all data stores are deleted";

        // make sure that everything is cleared, ignore errors
        RemoveCalendarItems(true);
        ClearManagers();
        m_dataStores.clear();

        // delete manager
        delete m_itemManager;

        MWTS_LEAVE;
}

/**
 * SetNumberOfCreatedItems function
 */
void OrganizerTest::SetNumberOfCreatedItems(int n)
{
    MWTS_ENTER;
    m_numberOfItems = n;
    qDebug()<<"Number of wanted items set to: "<<m_numberOfItems;
}

/**
 * SetMeasureState function
 */
void OrganizerTest::SetMeasureState(bool state)
{
    MWTS_ENTER;

    m_measureOn = state;
    m_itemManager->SetMeasureState(state);
    qDebug()<<"Measure state set to: "<<state;
}


/**
 * SetCalendarDatastore function
 */
void OrganizerTest::SetCalendarDatastore(QString name)
{
    MWTS_ENTER;
    m_calendarStore = name;
    qDebug()<<"Calendar datastore set to: "<<m_calendarStore;
}

/**
 * SetCalendarItemType function
 */
void OrganizerTest::SetCalendarItemType(int type)
{
    m_calendarItemType = type;
}


/**
 * CreateCalendarDatastore function
 */
void OrganizerTest::CreateCalendarDatastore()
{
    MWTS_ENTER;

    QOrganizerManager *manager = 0;
    if(m_calendarStore == "default")
    {
        qDebug()<<"Creating default store";
        manager = new QOrganizerManager();
    }
    else
    {
        qDebug()<<"Creating store: "<<m_calendarStore;
        manager = new QOrganizerManager(m_calendarStore);
    }

    // if creation fails
    if(!manager || manager->error() != QOrganizerManager::NoError)
    {
        qCritical()<<"Calendar store creation failed, error: "<<manager->error();
    }
    else
    {
        qDebug()<<"Store created successfully";
        m_dataStores.append(PimDataStore(manager,0,m_calendarStore));
    }
}

/**
 * CreateAvailableCalendarDatastores function
 */
void OrganizerTest::CreateAvailableCalendarDatastores()
{
    MWTS_ENTER;

    qDebug()<<"Fetching available calendar datastores";

    bool result;
    QStringList list = QOrganizerManager::availableManagers();

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
        QOrganizerManager *manager = 0;
        QOrganizerManager *uriManager = 0;
        qDebug()<<"Creating found datastores";

        for(int i=0;i<count;++i)
        {
           name = list[i].toLower();
           qDebug()<<"----------------";
           qDebug()<<i<<": Datastore name: "<<name;
           manager = new QOrganizerManager(name);

           if(manager)
           {
               qDebug()<<"Manager created";
               // validate name
               if(name != manager->managerName().toLower())
               {
                   qCritical()<<"Name "<<manager->managerName()<<" invalid";
                   break;
               }

                //Not found anymore in new version qDebug()<<"Supported data types: "<<manager->supportedDataTypes();
               qDebug()<<"Supported item types: "<<manager->supportedItemTypes();

               // get datastore parameters
               parameters = manager->managerParameters();
               qDebug()<<"Parameters: "<<parameters;

               // get datastore uri
               uri = manager->buildUri(name,parameters);
               qDebug()<<"Build uri: "<<uri;

               // validate uri
               result = QOrganizerManager::parseUri(uri, &parseName, &parseParameters);
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
               uriManager = QOrganizerManager::fromUri(uri);
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
           } else {
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
 * SearchDefaultCalendarItems function
 *
 * NOT TESTED
 * QOrganizerItemIntersectionFilter, QOrganizerItemUnionFilter, QOrganizerItemDateTimePeriodFilter (not found)
 */
void OrganizerTest::SearchDefaultCalendarItems()
{
    MWTS_ENTER;

    QOrganizerManager *manager = FindCalendarDataStore();
    if(!manager)
    {
        qCritical()<<"Calendar store not created";
        return;
    }

    // define filters
    QDateTime time;
    int day = m_dataFetcher.value("CALENDARITEMDETAILS/day").toInt();
    int month = m_dataFetcher.value("CALENDARITEMDETAILS/month").toInt();
    int year = m_dataFetcher.value("CALENDARITEMDETAILS/year").toInt();
    QDate tmp(year,month,day);
    time.setDate(tmp);

    QString detailDef = m_dataFetcher.value("CALENDARSEARCH/definition_name").toString();
    QString detailValue = m_dataFetcher.value("CALENDARSEARCH/definition_value").toString();
    int maxD = m_dataFetcher.value("CALENDARSEARCH/max_days").toInt();
    QDateTime maxDays = time.addDays(maxD);

    qDebug()<<"start time: "<<time;
    qDebug()<<"end time: "<<maxDays;
    qDebug()<<"detail definition: "<<detailDef;
    qDebug()<<"detail definition value: "<<detailValue;

    // test default filters
    QList<QOrganizerItemFilter> filterList;
    filterList.append(QOrganizerItemChangeLogFilter());

    QOrganizerItemDetailFilter b;
    b.setDetailDefinitionName(detailDef);
    QVariant input(detailValue);
    b.setValue(input);
    filterList.append(b);

    QOrganizerItemDetailRangeFilter c;
    c.setDetailDefinitionName(detailDef);
    QVariant min(time);
    QVariant max(maxDays);
    c.setRange(min,max);
    filterList.append(c);

    QOrganizerItemInvalidFilter e;
    filterList.append(e);

    // get all items
    QList<PimItem> items = m_itemManager->getCreatedItems();
    if(items.count()>=2)
    {
        QOrganizerItemIdFilter f;
        f.insert(items[0].itemLocalId);
        f.insert(items[1].itemLocalId);
        filterList.append(f);
    }

    QOrganizerItemFilter filter;

    QList<QOrganizerItemId> list;
    const int filterCount = filterList.count();
    for(int i=0;i<filterCount;++i)
    {
        filter = filterList[i];
        // if filter is supported
        if(manager->isFilterSupported(filter))
        {
            list = manager->itemIds(filter);
            // if the searching fails
            if(manager->error()!=QOrganizerManager::NoError)
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
            qDebug()<<"Filter "<<filter.type()<<" not supported by the datastore: "<<m_calendarStore;
        }
    }
}


/**
 * CreateCalendarItems function
 */
void OrganizerTest::CreateCalendarItems()
{
    MWTS_ENTER;

    bool result;
    QOrganizerItemId createdId; // <- not used in this

    // find manager
    QOrganizerManager *manager = FindCalendarDataStore();
    if(!manager)
    {
        qCritical()<<"Calendar store not created";
        return;
    }

    // initialize default data
    m_itemManager->setCalendarDataStore(m_calendarStore);
    m_itemManager->initializeCalendarItemData();

    int counter = 0;
    // For iteration, in FUTE cases the m_iterationCount is always 1
    for(int iter=0;iter<m_iterationCount;++iter)
    {
        // number of items
        for(int i=0;i<m_numberOfItems;++i)
        {
            counter += iter+i;
            qDebug()<<"-------------------------------";
            qDebug()<<"Creating item no. "<<counter<<" of type: "<<m_calendarItemType;

            // define type
            switch(m_calendarItemType)
            {
                case CalEvent:
                    result = m_itemManager->CreateAndVerifyEventItem(manager, createdId);
                    break;
                case CalEventOccurrence:
                    result = m_itemManager->CreateAndVerifyEventOccurrenceItem(manager, createdId);
                    break;
                case CalJournal:
                    result = m_itemManager->CreateAndVerifyJournalItem(manager, createdId);
                    break;
                case CalNote:
                    result = m_itemManager->CreateAndVerifyNoteItem(manager, createdId);
                    break;
                case CalTodo:
                    result = m_itemManager->CreateAndVerifyTodoItem(manager, createdId);
                    break;
                case CalTodoOccurrence:
                    result = m_itemManager->CreateAndVerifyTodoOccurrenceItem(manager, createdId);
                    break;
                default:
                    qCritical()<<"Invalid calendar item type";
                    result = false;
                    break;
            }
            // creation result
            if(!result)
            {
                qCritical()<<"Calendar item creation failed to datastore: "<<m_calendarStore;
                break;
            }
            else
            {
                qDebug()<<"Item "<<counter<<" was successfully created to datastore: "<<m_calendarStore;
            }
        } // item count
    } // iteration
}


/**
 * ModifyItems function
 */
void OrganizerTest::ModifyCalendarItems()
{
    MWTS_ENTER;

    bool result;
    PimItem pimCalItem;

    // find datastore.
    QOrganizerManager *manager = FindCalendarDataStore();
    if(!manager)
    {
        qCritical()<<"Calendar store not created";
        return;
    }

    // initialize modified data
    m_itemManager->setCalendarDataStore(m_calendarStore);
    m_itemManager->initializeModifiedCalendarItemData();

    // get all items
    QList<PimItem> items = m_itemManager->getCreatedItems();

    int counter = 0;
    // modify only items that this asset has added
    const int count = items.count();
    for(int i=0;i<count;++i)
    {
        pimCalItem = items[i];
        // modify items from one datastore  (type is defined in creation phase)
        if(pimCalItem.itemType==m_calendarItemType &&
           pimCalItem.dataStoreName==m_calendarStore)
        {
            counter += i+1;
            qDebug()<<"-------------------------------";
            qDebug()<<"Modifying item: "<<counter<<" type: "<<pimCalItem.itemType<<" in datastore: "<<pimCalItem.dataStoreName;

            switch(m_calendarItemType)
            {
                case CalEvent:
                    result = m_itemManager->ModifyAndVerifyEventItem(manager, pimCalItem.itemLocalId);
                    break;
                case CalEventOccurrence:
                    result = m_itemManager->ModifyAndVerifyEventOccurrenceItem(manager, pimCalItem.itemLocalId);
                    break;
                case CalJournal:
                    result = true;  // no difference to QOrganizerItem, modification is verified in creation phace
                    qDebug()<<"Journal item modifying succeeded";
                    break;
                case CalNote:
                    result = true;  // no difference to QOrganizerItem, modification is verified in creation phace
                    qDebug()<<"Note item modifying succeeded";
                    break;
                case CalTodo:
                    result = m_itemManager->ModifyAndVerifyTodoItem(manager, pimCalItem.itemLocalId);
                    break;
                case CalTodoOccurrence:
                    result = m_itemManager->ModifyAndVerifyTodoOccurrenceItem(manager, pimCalItem.itemLocalId);
                    break;
                default:
                    qCritical()<<"Invalid calendar item type";
                    result = false;
                    break;
            }

            // if item modification fails
            if(!result)
            {
                qCritical()<<"Item modification failed";
                break;
            }
            else
            {
                qDebug()<<"Item "<<counter<<" modification succeeded";
            }
        }
        else
        {
            qDebug()<<"Current datastore: "<<m_calendarStore<<" and item type: "<<m_calendarItemType;
            qDebug()<<"Ignoring item: "<<i+1<<" wrong type: "<<pimCalItem.itemType<<" or datastore: "<<pimCalItem.dataStoreName;
        }
    } // for
}


/**
 * RemoveCalendarItems function
 */
void OrganizerTest::RemoveCalendarItems(bool ignoreErrors)
{
    MWTS_ENTER;

    bool result;

    // find manager
    QOrganizerManager *manager = FindCalendarDataStore();
    if(!manager)
    {
        if(!ignoreErrors)
        {
            qCritical()<<"Calendar store not created";
        }
        return;
    }

    // get created items for this data store
    QList<PimItem> items = m_itemManager->getCreatedItems(m_calendarStore);

    // remove all items from this manager
    int counter = 1;
    PimItem item;

    qDebug()<<"Number of items to be removed from data store: "<< items.count();

    while(!items.empty())
    {
        item = items.first();
        result = removeItem(*manager,item.itemLocalId);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            if(!ignoreErrors)
            {
                qCritical()<<"Item removing failed from: "<<m_calendarStore<<" error: "<<manager->error();
            }
        }
        else
        {
            // don't stop even if the removing from the datastore fails
            qDebug()<<"Calendar item: "<<counter<<" removed";
            qDebug()<<"-------------------------------";
        }
        items.removeFirst();
        counter++;
    }
    qDebug()<<"Overall items left after removing from this store: "<<m_itemManager->getCreatedItems().count();
}

/**
 * removeItem function
 */
bool OrganizerTest::removeItem(QOrganizerManager& manager, QOrganizerItemId& itemId)
{
    bool result;
    // no measurement
    if(!m_measureOn)
    {
        result = manager.removeItem(itemId);
    }
    else
    {
        int elapsed=0;
        // measure removing time
        m_time.start();
        result = manager.removeItem(itemId);
        elapsed = m_time.elapsed();
        m_result.AddMeasure("Calendar item removing",elapsed,"ms");
    }
    return result;
}


/**
 * FindDataStore function
 */
QOrganizerManager* OrganizerTest::FindCalendarDataStore()
{
    MWTS_ENTER;

    qDebug()<<"Finding calendar store: "<<m_calendarStore;

    QOrganizerManager *manager = 0;
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i)
    {
        if(m_dataStores[i].dataStoreName==m_calendarStore)
        {
            qDebug()<<"Store found!";
            manager = m_dataStores[i].calendarManager;
            break;
        }
    }
    return manager;
}

/**
 * ClearManagers function
 */
void OrganizerTest::ClearManagers()
{
    MWTS_ENTER;

    // make sure that all managers are deleted
    const int count = m_dataStores.count();
    for(int i=0;i<count;++i)
    {
        if(m_dataStores[i].calendarManager)
        {
            qDebug()<<"Deleting manager: "<<m_dataStores[i].calendarManager->managerName();
            delete m_dataStores[i].calendarManager;
        }
    }
}

// eof
