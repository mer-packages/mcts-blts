/* pimorganizeritemmanager.cpp -- source code for PimOrganizerItemManager class
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
#include "pimorganizeritemmanager.h"

// includes

/**
 * PimOrganizerItemManager constructor
 */
PimOrganizerItemManager::PimOrganizerItemManager(MwtsConfig &dFetcher, MwtsResult& result)
    : m_detailFetcher(dFetcher),
      m_result(result),
      m_measureOn(false)
{
    MWTS_ENTER;
}

/**
 * PimOrganizerItemManager destructor
 */
PimOrganizerItemManager::~PimOrganizerItemManager()
{
    MWTS_ENTER;

    m_dates.clear();
    m_exceptionDates.clear();
    m_items.clear();
}

/**
 * getCreatedItems function
 */
QList<PimItem> PimOrganizerItemManager::getCreatedItems(QString dataStore)
{
    MWTS_ENTER;

    qDebug()<<"Overall item count: "<<m_items.count();

    // if no data store given, return all create items
    if(dataStore=="")
    {
        return m_items;
    }
    else
    {
        // otherwise pick only "dataStore" specified items and remove from all created items
        QList<PimItem> storeItems;
        PimItem item;
        int count = m_items.count();
        // go trough all and remove items from m_dates if needed
        while(count>0)
        {
            if(m_items.count()>0)
            {
                item = m_items[0];
                if(item.dataStoreName==dataStore)
                {
                    storeItems.append(item);
                    m_items.removeAt(0);
                }
            }
            count--;
        }
        qDebug()<<"Datastore: "<<dataStore<<" item count: "<<storeItems.count();
        return storeItems;
    }
}


/**
 * setCalendarDataStore function
 */
void PimOrganizerItemManager::setCalendarDataStore(QString store)
{
    MWTS_ENTER;

    m_calendarStore = store;
}


/**
 * initializeCalendarItemData function
 */
void PimOrganizerItemManager::initializeCalendarItemData()
{
    MWTS_ENTER;

    m_dates.clear();
    m_exceptionDates.clear();

    // get QOrganizerItem specific data
    m_comment = m_detailFetcher.value("CALENDARITEMDETAILS/comment").toString();
    m_description = m_detailFetcher.value("CALENDARITEMDETAILS/description").toString();
    m_label =  m_detailFetcher.value("CALENDARITEMDETAILS/displaylabel").toString();

    // get date information
    int year = m_detailFetcher.value("CALENDARITEMDETAILS/year").toInt();
    int month = m_detailFetcher.value("CALENDARITEMDETAILS/month").toInt();
    int day = m_detailFetcher.value("CALENDARITEMDETAILS/day").toInt();
    int mins =  m_detailFetcher.value("CALENDARITEMDETAILS/minutes").toInt();

    QDate date(year,month,day);
    m_dateTime = QDateTime(date);
    m_finishedDate = m_dateTime.addSecs(mins*60); // add 30 minutes to end time
    m_dates += m_dateTime.date();
    m_dates += m_dateTime.addDays(1).date();
    m_dates += m_dateTime.addDays(2).date();
    m_exceptionDates += m_dateTime.addDays(1).date();
    m_priority = (QOrganizerItemPriority::Priority)m_detailFetcher.value("CALENDARITEMDETAILS/priority").toInt();///QOrganizerItemPriority::ExtremelyHighPriority;
    m_locationAddress = m_detailFetcher.value("CALENDARITEMDETAILS/location_address").toString();
    m_locationName = m_detailFetcher.value("CALENDARITEMDETAILS/location_name").toString();
    m_locCoordinates = m_detailFetcher.value("CALENDARITEMDETAILS/location_coordinates").toString();
    m_progress = m_detailFetcher.value("CALENDARITEMDETAILS/progress").toInt();
    m_status = (QOrganizerTodoProgress::Status)m_detailFetcher.value("CALENDARITEMDETAILS/status").toInt();

    qDebug()<<"initializeCalendarItemData:";
    qDebug()<<"m_comment "<<m_comment;
    qDebug()<<"m_description: "<<m_description;
    qDebug()<<"m_label: "<<m_label;
    qDebug()<<"year: "<<year;
    qDebug()<<"month: "<<month;
    qDebug()<<"day"<<day;
    qDebug()<<"m_dates:"<<m_dates;
    qDebug()<<"minutes "<<mins;
    qDebug()<<"m_dateTime: "<<m_dateTime;
    qDebug()<<"m_finishedDate: "<<m_finishedDate;
    qDebug()<<"m_exceptionDates: "<<m_exceptionDates;
    qDebug()<<"m_priority: "<<m_priority;
    qDebug()<<"m_locationAddress: "<<m_locationAddress;
    qDebug()<<"m_locationName: "<<m_locationName;
    qDebug()<<"m_locCoordinates: "<<m_locCoordinates;
    qDebug()<<"m_progress "<<m_progress;
    qDebug()<<"m_status "<<m_status;
    qDebug()<<"initializeCalendarItemData stop";

}

/**
 * initializeModifiedCalendarItemData function
 */
void PimOrganizerItemManager::initializeModifiedCalendarItemData()
{
    MWTS_ENTER;

    m_dates.clear();
    m_exceptionDates.clear();

    // get QOrganizerItem specific data
    m_comment = m_detailFetcher.value("CALENDARITEMDETAILS/mod_comment").toString();
    m_description = m_detailFetcher.value("CALENDARITEMDETAILS/mod_description").toString();
    m_label =  m_detailFetcher.value("CALENDARITEMDETAILS/mod_displaylabel").toString();

     // add modified data
    int year = m_detailFetcher.value("CALENDARITEMDETAILS/mod_year").toInt();
    int month = m_detailFetcher.value("CALENDARITEMDETAILS/mod_month").toInt();
    int day = m_detailFetcher.value("CALENDARITEMDETAILS/mod_day").toInt();
    int mins =  m_detailFetcher.value("CALENDARITEMDETAILS/mod_minutes").toInt();
    QDate date(year,month,day);
    m_dateTime = QDateTime(date);
    m_finishedDate = m_dateTime.addSecs(mins*60); // add 60 minutes to end time
    m_dates += date;
    m_dates += date.addDays(1);
    m_dates += date.addDays(2);
    m_exceptionDates += date.addDays(1);
    m_priority = (QOrganizerItemPriority::Priority)m_detailFetcher.value("CALENDARITEMDETAILS/mod_priority").toInt();//QOrganizerItemPriority::VeryLowPriority;
    m_locationAddress = m_detailFetcher.value("CALENDARITEMDETAILS/mod_location_address").toString();
    m_locationName = m_detailFetcher.value("CALENDARITEMDETAILS/mod_location_name").toString();
    m_locCoordinates = m_detailFetcher.value("CALENDARITEMDETAILS/mod_location_coordinates").toString();
    m_progress = m_detailFetcher.value("CALENDARITEMDETAILS/mod_progress").toInt();
    m_status = (QOrganizerTodoProgress::Status)m_detailFetcher.value("CALENDARITEMDETAILS/mod_status").toInt();

    qDebug()<<"initializeModifiedCalendarItemData:";
    qDebug()<<"m_comment "<<m_comment;
    qDebug()<<"m_description: "<<m_description;
    qDebug()<<"m_label: "<<m_label;
    qDebug()<<"year: "<<year;
    qDebug()<<"month: "<<month;
    qDebug()<<"day"<<day;
    qDebug()<<"m_dates:"<<m_dates;
    qDebug()<<"minutes "<<mins;
    qDebug()<<"m_dateTime: "<<m_dateTime;
    qDebug()<<"m_finishedDate: "<<m_finishedDate;
    qDebug()<<"m_exceptionDates: "<<m_exceptionDates;
    qDebug()<<"m_priority: "<<m_priority;
    qDebug()<<"m_locationAddress: "<<m_locationAddress;
    qDebug()<<"m_locationName: "<<m_locationName;
    qDebug()<<"m_locCoordinates: "<<m_locCoordinates;
    qDebug()<<"m_progress "<<m_progress;
    qDebug()<<"m_status "<<m_status;
    qDebug()<<"initializeModifiedCalendarItemData stop";

}

/**
 * saveItem function
 */
bool PimOrganizerItemManager::saveItem(QOrganizerManager& manager, QOrganizerItem& item)
{
    MWTS_ENTER;

    bool result;
    // no measurement
    if(!m_measureOn)
    {
        result = manager.saveItem(&item);
    }
    else
    {
        int elapsed=0;
        QString details;
        QString res;
        // measure saving time
        m_time.start();
        result = manager.saveItem(&item);
        elapsed = m_time.elapsed();
        details.sprintf(" item saving with %d details",item.details().count());
        res.append(item.type());
        res.append(details);
        m_result.AddMeasure(res,elapsed,"ms");
    }
    return result;
}


/**
 * AddOrganizerItemData function
 */
bool PimOrganizerItemManager::AddOrganizerItemData(QOrganizerItem &item)
{
    MWTS_ENTER;

    // first clear item
    item.clearComments();
    //item->clearDetails();

    // verify that all is gone (type detail is left)
    if(item.comments().count()>0 || item.details().count()>1 /*|| !item->isEmpty()*/ )
    {
        qCritical()<<"Item clearing failed";
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
bool PimOrganizerItemManager::VerifyOrganizerItemData(QOrganizerItem *item)
{
    MWTS_ENTER;

    // This is the only way to check whether the detail is found anymore. (details may be stripped when pruning)
    QOrganizerItemComment comment;
    QOrganizerItemDescription desc;
    QOrganizerItemDisplayLabel label;
    QList<QOrganizerItemDetail> details = item->details();

    // veridy comments
    if(!item->comments().contains(m_comment) && details.contains(comment))
    {
        qCritical()<<"Comment addition failed, added: "<<m_comment<<" got: "<<item->comments();
        return false;
    }

    // verify description
    if(item->description()!=m_description && details.contains(desc))
    {
        qCritical()<<"Description addition failed, added: "<<m_description<<" got: "<<item->description();
        return false;
    }

    // verify label
    if(item->displayLabel()!=m_label && details.contains(label))
    {
        qCritical()<<"Display label addition failed, added: "<<m_label<<" got: "<<item->displayLabel();
        return false;
    }
    qDebug()<<"OrganizerItem Data verified (base class of all organizer items)";
    return true;
}

/**
 * CreateAndVerifyEventItem function
 *
 * NOT TESTTED YET
 * exception rules, recurrrence rules, set exception rules, set recurrence rules,
 *
 */
bool PimOrganizerItemManager::CreateAndVerifyEventItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerEvent item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        qDebug()<<"Adding and verifying event data";

        // add data to item
        AddEventItemData(item);

        // verify added data
        result = VerifyEventItemData(&item);
        if(!result)
        {
            qCritical()<<"Item verification failed";
            return false;
        }

        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();
        qDebug()<<"Trying to save the item";

        // try to save the event
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Event item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        if(manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Modified item fetching failed. error: "<<manager->error();
            return false;
        }

        QOrganizerEvent fetchedItem = static_cast<QOrganizerEvent>(tmp);
        result = VerifyEventItemData(&fetchedItem);
        if(!result)
        {
            qCritical()<<"Item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
       qCritical()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
       return false;
    }
    // add item to cache
    m_items.append(PimItem(item.id(),0,m_calendarStore,0));
    createdItem = item.id();
    return true;
}

/**
 * CreateAndVerifyEventOccurrenceItem function
 *
 */
bool PimOrganizerItemManager::CreateAndVerifyEventOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerEventOccurrence item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        qDebug()<<"Creating parent item for the event occurrance";

        QOrganizerItemId parentId;
        // parent item needs to be created for the occurrence
        result = CreateAndVerifyEventItem(manager,parentId);
        if(!result)
        {
            qCritical()<<"Parent item creation failed";
            return false;
        }
        // remove parent item from cache, event occurrence "owns" it
        m_items.removeLast();
        m_parentId = parentId;

        qDebug()<<"Parent item created, local id: "<<m_parentId;
        qDebug()<<"Adding and verifying event occurrence data";

        // add data to the item
        AddEventOccurrenceItemData(item);

        // verify addition
        result = VerifyEventOccurrenceItemData(&item);
        if(!result)
        {
            qCritical()<<"Item verification failed";
            return false;
        }

        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);
        // reset the parent id
        item.setParentId(m_parentId);

        qDebug()<<"Detail count after pruning: "<<item.details().count();
        qDebug()<<"Trying to save the item";

        // try to save the event occurrence
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError )
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Event occurrence saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        QOrganizerEventOccurrence fetchedItem = static_cast<QOrganizerEventOccurrence>(tmp);
        result = VerifyEventOccurrenceItemData(&fetchedItem);
        if(!result)
        {
            qCritical()<<"Item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
        qCritical()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add to cache
    m_items.append(PimItem(item.id(), 0, m_calendarStore,1));
    createdItem = item.id();
    return true;
}

/**
 * CreateAndVerifyJournalItem function
 */
bool PimOrganizerItemManager::CreateAndVerifyJournalItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerJournal item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        // test QOrganizerJournal specific functions. Journal has only one function
        qDebug()<<"Occurrence date: "<<item.dateTime();
        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the journal
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Journal item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        if(manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item fetching failed. Error: "<<manager->error();
            return false;
        }

        result = VerifyOrganizerItemData(&tmp);
        if(!result)
        {
            qCritical()<<"CreateJournalItem: item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
        qCritical()<<"CreateJournalItem: "<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.id(), 0, m_calendarStore,2));
    createdItem = item.id();
    return true;
}


/**
 * CreateAndVerifyNoteItem function
 */
bool PimOrganizerItemManager::CreateAndVerifyNoteItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerNote item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the note
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Note item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        if(manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item fetching failed. Error: "<<manager->error();
            return false;
        }

        result = VerifyOrganizerItemData(&tmp);
        if(!result)
        {
            qCritical()<<"Item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
        qCritical()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.id(), 0, m_calendarStore,3));
    createdItem = item.id();
    return true;
}

/**
 * CreateAndVerifyTodoItem function
 * NOT TESTED YET
 * expception rules, recurrence rules
 * set exept rules,  set recurrence rules,
 */
bool PimOrganizerItemManager::CreateAndVerifyTodoItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result;
    QOrganizerTodo item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        // todo specific
        m_finishedDate = m_dateTime.addDays(2);
        qDebug()<<"m_finishedDate changed: "<<m_finishedDate;

        // add data
        AddTodoItemData(item);

        // verify addition
        result = VerifyTodoItemData(&item);
        if(!result)
        {
            qCritical()<<"Data verification failed";
            return false;
        }

        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the todo
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Todo item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        QOrganizerTodo fetchedItem = static_cast<QOrganizerTodo>(tmp);
        bool result = VerifyTodoItemData(&fetchedItem);
        if(!result)
        {
            qCritical()<<"Item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
        qCritical()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add item to cache
    m_items.append(PimItem(item.id(),0 ,m_calendarStore,4));
    createdItem = item.id();
    return true;
}


/**
 * CreateAndVerifyTodoOccurrenceItem function
 */
bool PimOrganizerItemManager::CreateAndVerifyTodoOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem)
{
    MWTS_ENTER;

    bool result = true;
    QOrganizerTodoOccurrence item;

    // first check that manager supports this
    if(manager->supportedItemTypes().contains(item.type()))
    {
        // Add organizer item specific data and verify it
        if(!AddOrganizerItemData(item) ||
           !VerifyOrganizerItemData(&item))
        {
            return false;
        }

        qDebug()<<"Creating parent item for the todo occurrance";

        QOrganizerItemId parentId;
        // parent item needs to be created for the occurrence
        result = CreateAndVerifyTodoItem(manager,parentId);
        if(!result)
        {
            qCritical()<<"Parent item creation failed";
            return false;
        }
        // remove parent item from cache, event occurrence "owns" it
        m_items.removeLast();
        m_parentId = parentId;

        // occurrence specific
        m_finishedDate = m_dateTime.addDays(2);
        qDebug()<<"m_finishedDate changed: "<<m_finishedDate;

        // add item data
        AddTodoOccurrenceItemData(item);

        // verify addition
        result = VerifyTodoOccurrenceItemData(&item);
        if(!result)
        {
            qCritical()<<"Data verification failed";
            return false;
        }

        qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
        qDebug()<<"Detail count before pruning: "<<item.details().count();

        item = manager->compatibleItem(item);
        // reset the parent id
        item.setParentId(m_parentId);

        qDebug()<<"Detail count after pruning: "<<item.details().count();

        // try to save the todo occurrence
        result = saveItem(*manager,item);
        if(!result || manager->error()!=QOrganizerManager::NoError)
        {
            qCritical()<<"Item saving failed, error: "<<manager->error();
            return false;
        }

        qDebug()<<"Todo occurrance item saving succeeded";

        // saving succeeded, fetch item and verify data
        QOrganizerItem tmp = manager->item(item.id());
        QOrganizerTodoOccurrence fetchedItem = static_cast<QOrganizerTodoOccurrence>(tmp);
        bool result = VerifyTodoOccurrenceItemData(&fetchedItem);
        if(!result)
        {
            qCritical()<<"Item verification failed after saving and fetching";
            return false;
        }
    }
    else
    {
        qCritical()<<item.type()<<" not supported by the datastore: "<<manager->managerName();
        return false;
    }
    // add to cache
    m_items.append(PimItem(item.id(),0,m_calendarStore,5));
    createdItem = item.id();
    return true;
}

/**
 * ModifyAndVerifyEventItem function
 */
bool PimOrganizerItemManager::ModifyAndVerifyEventItem(QOrganizerManager *manager, QOrganizerItemId itemId)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to event
    QOrganizerEvent event = static_cast<QOrganizerEvent>(tmp);

    qDebug()<<"Adding and verifying modified event data";

    // add data to item
    AddEventItemData(event);

    qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.id());
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Modified item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerEvent fetchedItem = static_cast<QOrganizerEvent>(tmp);
    result = VerifyEventItemData(&fetchedItem);
    if(!result)
    {
        qCritical()<<"Item verification failed after saving and fetching";
        return false;
    }

    qDebug()<<"Event item modifying succeeded";
    return true;
}

/**
 * ModifyAndVerifyEventOccurrenceItem function
 */
bool PimOrganizerItemManager::ModifyAndVerifyEventOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId itemId)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerEventOccurrence
    QOrganizerEventOccurrence event = static_cast<QOrganizerEventOccurrence>(tmp);
    m_parentId = event.parentId();

    // add data to the item
    AddEventOccurrenceItemData(event);

    qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.id());
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerEventOccurrence fetchedItem = static_cast<QOrganizerEventOccurrence>(tmp);
    result = VerifyEventOccurrenceItemData(&fetchedItem);
    if(!result)
    {
        qCritical()<<"Item verification failed after saving and fetching";
        return false;
    }
    qDebug()<<"Event occurrence item modifying succeeded";
    return true;
}


/**
 * ModifyAndVerifyTodoItem function
 */
bool PimOrganizerItemManager::ModifyAndVerifyTodoItem(QOrganizerManager *manager, QOrganizerItemId itemId)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"ModifyTodoItem: item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerTodo
    QOrganizerTodo event = static_cast<QOrganizerTodo>(tmp);

    // todo specific
    m_finishedDate = m_dateTime.addDays(3);
    qDebug()<<"m_finishedDate changed: "<<m_finishedDate;

    // add data
    AddTodoItemData(event);

    qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.id());
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerTodo fetchedItem = static_cast<QOrganizerTodo>(tmp);
    result = VerifyTodoItemData(&fetchedItem);
    if(!result)
    {
        qCritical()<<"Item verification failed after saving and fetching";
        return false;
     }
    qDebug()<<"Todo item modifying succeeded";
    return true;
}


/**
 * ModifyAndVerifyTodoOccurrenceItem function
 */
bool PimOrganizerItemManager::ModifyAndVerifyTodoOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId itemId)
{
    MWTS_ENTER;

    bool result;

    // fetch the item
    QOrganizerItem tmp = manager->item(itemId);
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }
    // cast to QOrganizerTodoOccurrence
    QOrganizerTodoOccurrence event = static_cast<QOrganizerTodoOccurrence>(tmp);

    // todo specific
    m_finishedDate = m_dateTime.addDays(4);
    qDebug()<<"m_finishedDate changed: "<<m_finishedDate;
    m_parentId = event.parentId();

    // add item data
    AddTodoOccurrenceItemData(event);

    qDebug()<<"Make sure that calendar item is suitable for the manager by pruning";
    qDebug()<<"Detail count before pruning: "<<event.details().count();

    event = manager->compatibleItem(event);

    qDebug()<<"Detail count after pruning: "<<event.details().count();

    // try to save the event
    result = manager->saveItem(&event);
    if(!result || manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item saving failed, error: "<<manager->error();
        return false;
    }

    // saving succeeded, fetch item and verify data
    tmp = manager->item(event.id());
    if(manager->error()!=QOrganizerManager::NoError)
    {
        qCritical()<<"Item fetching failed. Error: "<<manager->error();
        return false;
    }

    QOrganizerTodoOccurrence fetchedItem = static_cast<QOrganizerTodoOccurrence>(tmp);
    result = VerifyTodoOccurrenceItemData(&fetchedItem);
    if(!result)
    {
        qCritical()<<"Item verification failed after saving and fetching";
        return false;
    }

    qDebug()<<"Todo occurrance item modifying succeeded";
    return true;
}

/**
 * Verifies event item data
 */
bool PimOrganizerItemManager::VerifyEventItemData(QOrganizerEvent *item)
{
    MWTS_ENTER;

    QSet<QDate> tmpDates;

    // This is the only way to check whether the detail is found anymore. (details may be stripped when pruning)
    QOrganizerEventTime startend;
    QOrganizerItemLocation location;
    QOrganizerItemPriority priority;
    QOrganizerItemRecurrence recexp;
    QList<QOrganizerItemDetail> details = item->details();

    // verify start time
    if(item->startDateTime()!=m_dateTime && details.contains(startend))
    {
        qCritical()<<"Start date addition failed, added: "<<m_dateTime<<" got: "<<item->startDateTime();
        return false;
    }

    // verify end time
    if(item->endDateTime()!=m_finishedDate && details.contains(startend))
    {
        qCritical()<<"End date addition failed, added: "<<m_finishedDate<<" got: "<<item->endDateTime();
        return false;
    }

    // verify recurrence dates
    tmpDates = item->recurrenceDates();
    if(m_dates!=tmpDates && details.contains(recexp))
    {
        qCritical()<<"Recurrence date addition failed, added: "<<m_dates<<" got: "<<tmpDates;
        return false;
    }

    // verify exception dates
    tmpDates.clear();
    tmpDates = item->exceptionDates();
    if(m_exceptionDates!=tmpDates  && details.contains(recexp))
    {
        qCritical()<<"Exception date addition failed, added: "<<m_exceptionDates<<" got: "<<tmpDates;
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority  && details.contains(priority))
    {
        qCritical()<<"Priority addition failed, added: "<<m_priority<<" got: "<<item->priority();
        return false;
    }

    /* Not found from new version
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
    */
    // verify location coordinates
    if(item->location()!=m_locCoordinates && details.contains(location))
    {
        qCritical()<<"Geocoordinate addition failed, added: "<<m_locCoordinates<<" got: "<<item->location();
        return false;
    }

    qDebug()<<"QOrganizerEvent Data verified";
    return true;
}


/**
 * Adds event item data
 */
void PimOrganizerItemManager::AddEventItemData(QOrganizerEvent &item)
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
    //item.setLocationAddress(m_locationAddress);
    // set alocation name
    //item.setLocationName(m_locationName);
    // set and verify location geocoordinates
    //item.setLocationGeoCoordinates(m_locCoordinates);
    item.setLocation(m_locCoordinates);
}


/**
 * Verifies event occurrence item data
 */
bool PimOrganizerItemManager::VerifyEventOccurrenceItemData(QOrganizerEventOccurrence *item)
{
    MWTS_ENTER;

    // This is the only way to check whether the detail is found anymore. (details may be stripped when pruning)
    QOrganizerItemParent parentid;
    QOrganizerEventTime startend;
    QOrganizerItemPriority priority;
    QOrganizerItemLocation location;
    QList<QOrganizerItemDetail> details = item->details();

    // verify parent id
    if(item->parentId()!=m_parentId && details.contains(parentid))
    {
        qCritical()<<"Parent id addition failed, added: "<<m_parentId<<" got: "<<item->parentId();
        return false;
    }

    // verify original date
    if(item->originalDate()!=m_dateTime.date() && details.contains(parentid))
    {
        qCritical()<<"Original date addition failed, added: "<<m_dateTime.date()<<" got: "<<item->originalDate();
        return false;
    }

    // verify end time
    if(item->endDateTime()!=m_finishedDate && details.contains(startend))
    {
        qCritical()<<"End date addition failed, added: "<<m_finishedDate<<" got: "<<item->endDateTime();
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority && details.contains(priority))
    {
        qCritical()<<"Priority addition failed, added: "<<m_priority<<" got: "<<item->priority();
        return false;
    }

    /* Not found from current version
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
    */
    // verify location coordinates
    if(item->location()!=m_locCoordinates && details.contains(location))
    {
        qCritical()<<"Geocoordinate addition failed, added: "<<m_locCoordinates<<" got: "<<item->location();
        return false;
    }

    qDebug()<<"QOrganizerEventOccurrence Data verified";
    return true;
}


/**
 * Adds event occurrence item data
 */
void PimOrganizerItemManager::AddEventOccurrenceItemData(QOrganizerEventOccurrence &item)
{
    MWTS_ENTER;

    // add parent id
    item.setParentId(m_parentId);
    // Add original date
    item.setOriginalDate(m_dateTime.date());
    // add  end time
    item.setEndDateTime(m_finishedDate);
    // set priority
    item.setPriority(m_priority);
    // set and verify location address
    //item.setLocationAddress(m_locationAddress);
    // set and verify location name
    //item.setLocationName(m_locationName);
    // set and verify location geocoordinates
    //item.setLocationGeoCoordinates(m_locCoordinates);
    item.setLocation(m_locCoordinates);

    // Print start date time
    qDebug()<<"Start date time: "<<item.startDateTime();
}


/**
 * Verifies event todo item data
 */
bool PimOrganizerItemManager::VerifyTodoItemData(QOrganizerTodo *item)
{
    MWTS_ENTER;

    QSet<QDate> tmpDates;

    // This is the only way to check whether the detail is found anymore. (details may be stripped when pruning)
    QOrganizerTodoTime todotime;
    QOrganizerItemRecurrence recexp;
    QOrganizerItemPriority priority;
    QOrganizerTodoProgress progress;
    QList<QOrganizerItemDetail> details = item->details();

    // verify due date time
    if(item->dueDateTime()!=m_finishedDate && details.contains(todotime))
    {
        qCritical()<<"Due date setting failed, added: "<<m_finishedDate<<" got: "<<item->dueDateTime();
        return false;
    }

    // verify recurrence dates
    tmpDates = item->recurrenceDates();
    if(m_dates!=tmpDates && details.contains(recexp))
    {
        qCritical()<<"Recurrence date addition failed, added: "<<m_dates<<" got: "<<tmpDates;
        return false;
    }

    // verify exception dates
    tmpDates = item->exceptionDates();
    if(m_dates!=tmpDates && details.contains(recexp))
    {
        qCritical()<<"Exception date addition failed, added: "<<m_dates<<" got: "<<tmpDates;
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority && details.contains(priority))
    {
        qCritical()<<"Priority addition failed, added: "<<m_priority<<" got: "<<item->priority();
        return false;
    }

    // verify finished date time
    if(item->finishedDateTime()!=m_finishedDate && details.contains(progress))
    {
        qCritical()<<"Finished date addition failed, added: "<<m_finishedDate<<" got: "<<item->finishedDateTime();
        return false;
    }

    // verify progress percentage
    if(item->progressPercentage()!=m_progress && details.contains(progress))
    {
        qCritical()<<"Progress percentage addition failed, added: "<<m_progress<<" got: "<<item->progressPercentage();
        return false;
    }

    // verify status
    if(item->status()!=m_status && details.contains(progress))
    {
        qCritical()<<"Status addition failed, added: "<<m_status<<" got: "<<item->status();
        return false;
    }
    qDebug()<<"QOrganizerTodo Data verified";
    return true;
}


/**
 * Adds event todo item data
 */
void PimOrganizerItemManager::AddTodoItemData(QOrganizerTodo &item)
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
bool PimOrganizerItemManager::VerifyTodoOccurrenceItemData(QOrganizerTodoOccurrence *item)
{
    MWTS_ENTER;

    // This is the only way to check whether the detail is found anymore. (details may be stripped when pruning)
    QOrganizerItemParent parentid;
    QOrganizerTodoTime todotime;
    QOrganizerTodoProgress progress;
    QOrganizerItemPriority priority;
    QList<QOrganizerItemDetail> details = item->details();

     // verify parent local id
    if(item->parentId()!=m_parentId && details.contains(parentid))
    {
        qCritical()<<"Parent id setting failed, added: "<<m_parentId<<" got: "<<item->parentId();
        return false;
    }

    // verify due date time
    if(item->dueDateTime()!=m_finishedDate && details.contains(todotime))
    {
        qCritical()<<"Due date setting failed, added: "<<m_finishedDate<<" got: "<<item->dueDateTime();
        return false;
    }

    // verify status
    if(item->status()!=m_status && details.contains(progress))
    {
        qCritical()<<"Status addition failed, added: "<<m_status<<" got: "<<item->status();
        return false;
    }

    // verify finished date time
    if(item->finishedDateTime()!=m_finishedDate && details.contains(progress))
    {
        qCritical()<<"Finished date addition failed, added: "<<m_finishedDate<<" got: "<<item->finishedDateTime();
        return false;
    }

    // verify original date
    if(item->originalDate()!=m_dateTime.date() && details.contains(parentid))
    {
        qCritical()<<"Original date addition failed, added: "<<m_dateTime.date()<<" got: "<<item->originalDate();
        return false;
    }

    // verify priority
    if(item->priority()!=m_priority && details.contains(priority))
    {
        qCritical()<<"Priority addition failed, added: "<<m_priority<<" got: "<<item->priority();
        return false;
    }
    qDebug()<<"QOrganizerTodoOccurrence Data verified";
    return true;
}


/**
 * Adds event todo occurrence item data
 */
void PimOrganizerItemManager::AddTodoOccurrenceItemData(QOrganizerTodoOccurrence &item)
{
    MWTS_ENTER;

    // set parent id for this occurrence
    item.setParentId(m_parentId);
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

