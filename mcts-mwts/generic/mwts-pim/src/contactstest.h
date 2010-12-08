/* contactstest.h -- Header for ContactsTest class
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


#ifndef CONTACTSTEST_H
#define CONTACTSTEST_H

// Includes
#include <QObject>
#include <MwtsCommon>
#include <qmobilityglobal.h>
#include <QContact>
#include <QContactManager>
#include <QContactFilter>
#include "PimItem.h"
#include "PimDataStore.h"

// namespace
QTM_USE_NAMESPACE

// forwards
class PimContactDetailManager;

/**
  * ContactsTest class
  */
class ContactsTest : public QObject
{
    Q_OBJECT

public: // constructor & destructor

    /**
     * Constructor
     */
     ContactsTest(MwtsConfig &dFetcher,MwtsResult &result);

     /**
      * Destructor
      */
     ~ContactsTest();

public: // new functions

     /**
      * Sets iteration count
      */
     inline void SetIterationCount(int n) { MWTS_ENTER; m_iterationCount = n; }

     /**
      * Sets measuring on/off
      */
     inline void SetMeasureState(bool state) { m_measureOn = state; }

     /**
      * Sets the number of items to be created
      */
     void SetNumberOfCreatedItems(int n);

     /**
      * Sets the calendar data store to be used
      */
     void SetContactDatastore(QString name);

     /**
      * Creates a specific contact datastore. If no name is provided, a default store is used
      */
     void CreateContactDatastore();

     /**
      * Function lists all the found contact datastores
      * from the device and creates them by usin name and URI
      */
      void CreateAvailableContactDatastores();

     /**
      * Function searches contacts based on different QContactFilters
      */
      void SearchContacts();

     /**
      * Creates contact(s) with default details defined in QtM contacts
      * Number of created contacts is defined in m_numberOfItems
      */
     void CreateContacts();

     /**
      * Modifies details of the created contact
      */
     void ModifyContacts();

     /**
      * Removes created items from datastore.
      */
     void RemoveContacts(bool ignoreErrors = false);

     /**
      * Imports contact from vCard
      */
     void ImportContactFromVcard();

     /**
      * Exports contact to vCard
      */
     void ExportContactToVcard();

 private: // new functions

     /**
      * Saves contact to data store
      */
     bool saveContact(QContactManager& manager, QContact& contact);

     /**
      * Removes contact from data store
      */
     bool removeContact(QContactManager& manager, QContactLocalId& contactId);

     /**
      * Removes created datastores
      */
     void ClearManagers();

     /**
      * Finds created contact datastores based on name.
      */
     QContactManager* FindContactDataStore();

private: // data

     // for measuring
     QTime m_time;

     // for fetching variables
     MwtsConfig& m_dataFetcher;

     // for writing results
     MwtsResult& m_result;

     // defines whether measurement are made
     bool m_measureOn;

     // Used data store names
     QString m_contactStore;

     // created items
     QList<PimItem> m_items;

     // created datastores
     QList<PimDataStore> m_dataStores;

     // number of created items
     int m_numberOfItems;

     // iteration count
     int m_iterationCount;

     // detail manager
     PimContactDetailManager *m_detailManager;
};

#endif // CONTACTSTEST_H
