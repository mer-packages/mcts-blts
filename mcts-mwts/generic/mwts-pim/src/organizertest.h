/* organizertest.h -- Header for OrganizerTest class
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


#ifndef ORGANIZERTEST_H
#define ORGANIZERTEST_H

// Includes
#include <MwtsCommon>
#include <QObject>
#include <qmobilityglobal.h>
#include "PimItem.h"
#include "PimDataStore.h"

// namescape
QTM_USE_NAMESPACE

// forwards
class PimOrganizerItemManager;

/**
 * OrganzerTest class
 */
class OrganizerTest : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    OrganizerTest(MwtsConfig &dFetcher,MwtsResult &result);

    /**
     * Destructor
     */
    ~OrganizerTest();

public: // new functions

    /**
     * Sets iteration count
     */
    inline void SetIterationCount(int n) { MWTS_ENTER; m_iterationCount = n; }

    /**
     * Sets measuring on/off
     */
    void SetMeasureState(bool state);

    /**
     * Sets the number of items to be created
     */
    void SetNumberOfCreatedItems(int n);

    /**
     * Sets the contact data store to be used
     */
    void SetCalendarDatastore(QString name);

    /**
     * Sets wanted calendar item type.
     * Event = 0,
     * EventOccurrence,
     * Journal,
     * Note,
     * Todo,
     * TodoOccurrence
     */
    void SetCalendarItemType(int type);

    /**
     * Creates a specific calendar datastore.
     */
    void CreateCalendarDatastore();

    /**
     * Functions lists all the found calendar datastores
     * from the device and creates them by usin name and URI
     */
    void CreateAvailableCalendarDatastores();

    /**
     * Searches default items based on different search filters
     */
    void SearchDefaultCalendarItems();

    /**
     * Creates wanted calendar items and adds it to wanted datastore. If wanted store is not defined, a default store is used
     * Check CalendarItemTypes definition
     * Number of created items is defined in m_numberOfItems
     */
    void CreateCalendarItems();

    /**
     * Modifies items in the database and verifies the changes.
     */
    void ModifyCalendarItems();

    /**
     * Removes created items from datastore. ignoreErrors variable is for clearing items at the destruction phase.
     */
    void RemoveCalendarItems(bool ignoreErrors = false);

    /**
     * Imports calendar item from iCalendar file
     */
    void ImportCalendarItemFromIcalendar();

    /**
     * Exports created calendar item to iCalendar format
     */
    void ExportCalendarItemToIcalendar();

private: // new methods

     /**
      * Removes item from manager
      */
     bool removeItem(QOrganizerManager& manager, QOrganizerItemId& itemId);

     /**
      * Finds created calendar datastores based on name.
      */
     QOrganizerManager* FindCalendarDataStore();

     /**
      * Removes created datastores
      */
     void ClearManagers();

public:     // data
     /**
      * default calendar item types
      */
     enum CalendarItemTypes {
         CalEvent = 0,
         CalEventOccurrence,
         CalJournal,
         CalNote,
         CalTodo,
         CalTodoOccurrence
     };

private: // data

     // for time measuring
     QTime m_time;

     // for fetching variables
     MwtsConfig& m_dataFetcher;

     // for writing results
     MwtsResult &m_result;

     // defines whether measurement are made
     bool m_measureOn;

     // used datastore
     QString m_calendarStore;

     // Used calendar type
     int m_calendarItemType;

     // created datastores
     QList<PimDataStore> m_dataStores;

     // number of created items
     int m_numberOfItems;

     // iteration count
     int m_iterationCount;

     // item manager
     PimOrganizerItemManager *m_itemManager;
};

#endif // ORGANIZERTEST_H
