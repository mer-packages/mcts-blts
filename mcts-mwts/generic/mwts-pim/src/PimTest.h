/* PimTest.h -- Header for PimTest class
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

#ifndef _INCLUDED_PIM_TEST_H
#define _INCLUDED_PIM_TEST_H

#include <MwtsCommon>
#include <qmobilityglobal.h>
#include <QOrganizerItem>
#include <QOrganizerManager>
#include <QOrganizerEvent>
#include <QOrganizerEventOccurrence>
#include <QOrganizerJournal>
#include <QOrganizerNote>
#include <QOrganizerTodo>
#include <QOrganizerTodoOccurrence>
#include <QContact>
#include <QContactManager>

QTM_USE_NAMESPACE

/**
  * Helper class for storing calendar items
  */
class PimItem {
public:
    PimItem(QOrganizerItemId iId=QOrganizerItemId(), QContactLocalId cId=-1, QString name="default", int iType=-1)
    {
        itemLocalId = iId;
        contactLocalId = cId;
        dataStoreName = name;
        itemType = iType;
    }
public: // data
    int itemType;
    QString dataStoreName;
    QOrganizerItemId itemLocalId;
    QContactLocalId contactLocalId;
};

/**
  * Helper class for storing calendar and contacts data stores
  */
class PimDataStore {
public:
    PimDataStore(QOrganizerManager *mngr=0, QContactManager *cntcMngr=0 ,QString name="default")
    {
        dataStoreName = name;
        calendarManager = mngr;
        contactManager = cntcMngr;
    }
public: // data
    QString dataStoreName;
    QOrganizerManager *calendarManager;
    QContactManager *contactManager;
};



/**
 * Pim test class
 *
 */
class PimTest : public MwtsTest
{
	Q_OBJECT
public:

        /*
         * Constructor for pim test class
	 */
        PimTest();

        /*
         * Destructor for pim test class
	 */
        virtual ~PimTest();

        /*
	 * Overridden functions for MwtsTest class
	 * OnInitialize is called before test execution
	 */
	void OnInitialize();

        /*
	 * Overridden functions for MwtsTest class
	 * OnUninitialize is called after test execution
	 */
	void OnUninitialize();

public:

        /*
         * Sets the number of items to be created
         */
        void SetNumberOfCreatedItems(int n);

        /*
         * Sets the contact data store to be used
         */
        void SetCalendarDatastore(QString name);

        /*
         * Sets the calendar data store to be used
         */
        void SetContactDatastore(QString name);


        /*
         * Sets wanted calendar item type.
         * Event = 0,
         * EventOccurrence,
         * Journal,
         * Note,
         * Todo,
         * TodoOccurrence
         */
        void SetCalendarItemType(int type);

        // ORGANIZER SPECIFIC FUNCTIONS

       /*
        * Functions lists all the found calendar datastores
        * from the device and creates them by usin name and URI
        */
        void CreateAvailableCalendarDatastores();

        /*
         * Creates certain amount of calendar items. Use SetNumberOfCreatedItems
         */
        void TestCalendarItemCreation();

        /*
         * Creates certain amount of items and deletes them.  Use SetNumberOfCreatedItems
         */
        void TestCalendarItemCreationDeleting();

        /*
         * Creates certain amount of items, modifies them and deletes them.  Use SetNumberOfCreatedItems
         */
        void TestCalendarItemCreationModifyingDeleting();

        /*
          * Searches items based on different search filters
          */
        void TestItemSearching();

        // CONTACTS AND VERSIT FUNCTIONS

       /*
        * Functions lists all the found contact datastores
        * from the device and creates them by usin name and URI
        */
        void CreateAvailableContactDatastores();

       /*
        * Imports contact from vCard and verifies contact
        */
        void TestImportingVcardToContact();

       /*
        * Exports vCard from contact and verifies
        */
        void TestExportingVcardFromContact();

       /*
        * Creates certain amount of contacts to contact store
        */
        void TestContactItemCreationDeleting();

       /*
        * Creates certain amount of contacts, modifies them, and adds to database
        */
        void TestContactItemCreationModifyingDeleting();

private:

        /*
         * Creates a specific contact datastore. If no name is provided, a default store is used
         */
        bool CreateContactDatastore();

        /*
         * Creates a specific calendar datastore. If no name is provided, a default store is used
         */
        bool CreateCalendarDatastore();

        /*
         * Creates wanted calendar items and adds it to wanted datastore. If wanted store is not defined, a default store is used
         * Check CalendarItemTypes definition
         */
        bool CreateCalendarItems();

        /*
         * Imports hardcoded vCard to contact
         */
        bool ImportContact(QContact &contact);

        /*
         * Creates contact with default details defined in QtM contacts
         */
        bool CreateContacts();

        /*
          * Modifies details of the created contact
          */
        bool ModifyContacts();

        /*
         * Removes created items from datastore. If no name is provided, a default store is cleared
         */
        bool RemoveCreatedItemsFromCalendar();
        bool RemoveCreatedItemsFromContacts();

        /*
         * Modifies items in the database and verifies the changes. If wanted store is not defined, a default store is used
         */
        bool ModifyCalendarItems();

        /*
         * Removes created datastores
         */
        void ClearManagers();

        /*
         * Finds datastores based on name.
         */
        QOrganizerManager* FindCalendarDataStore();
        QContactManager* FindContactDataStore();

        /*
         * Adds default details to a list
         */
        void AddContactInformation(QContact &target);

        /*
         * QOrganizerItem functionalities
         */
        bool AddOrganizerItemData(QOrganizerItem &item);
        bool VerifyOrganizerItemData(QOrganizerItem *item);

        /*
         * Calendar item creation functions, each item have specific fields to fill
         */
        bool CreateEventItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
        bool CreateEventOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
        bool CreateJournalItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
        bool CreateNoteItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
        // createdItem contains pointer to newly created item
        bool CreateTodoItem(QOrganizerManager *manager, QOrganizerItemId &createdItem );
        bool CreateTodoOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);

        /*
         * Calendar item modification functions, each item have specific fields to fill
         */
        bool ModifyEventItem(QOrganizerItemId itemId, QOrganizerManager *manager);
        bool ModifyEventOccurrenceItem(QOrganizerItemId itemId, QOrganizerManager *manager);
        bool ModifyTodoItem(QOrganizerItemId itemId, QOrganizerManager *manager);
        bool ModifyTodoOccurrenceItem(QOrganizerItemId itemId, QOrganizerManager *manager);

        /*
         * Data addition functions
         */
        void AddEventItemData(QOrganizerEvent &item);
        void AddEventOccurrenceItemData(QOrganizerEventOccurrence &item);
        void AddTodoItemData(QOrganizerTodo &item);
        void AddTodoOccurrenceItemData(QOrganizerTodoOccurrence &item);

        /*
         * Data verification functions
         */
        bool VerifyEventItemData(QOrganizerEvent *item);
        bool VerifyEventOccurrenceItemData(QOrganizerEventOccurrence *item);
        bool VerifyTodoItemData(QOrganizerTodo *item);
        bool VerifyTodoOccurrenceItemData(QOrganizerTodoOccurrence *item);

private:    // data

    // calendar item types
    enum CalendarItemTypes {
        Event = 0,
        EventOccurrence,
        Journal,
        Note,
        Todo,
        TodoOccurrence
    };

    // Used data store names
    QString m_contactStore;
    QString m_calendarStore;

    // Used calendar type
    int m_calendarItemType;

    // created items
    QList<PimItem> m_items;

    // created datastores
    QList<PimDataStore> m_dataStores;

    // number of created items
    int m_numberOfItems;

    // Calendar item data
    int m_progress;
    QDateTime m_dateTime;
    QDateTime m_finishedDate;
    QSet<QDate> m_dates;
    QSet<QDate> m_exceptionDates;
    QOrganizerItemPriority::Priority m_priority;
    QOrganizerTodoProgress::Status m_status;
    QString m_locationAddress;
    QString m_locationName;
    QString m_locCoordinates;
    QString m_comment;
    QString m_description;
    QString m_label;
    QOrganizerItemId m_parentId;
    // vCard data
    QByteArray m_inputVCard;
};

#endif //#ifndef _INCLUDED_PIM_TEST_H

