/* pimorganizeritemmanager.h -- Header for PimOrganizerItemManager class
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

#ifndef PimOrganizerItemManager_H
#define PimOrganizerItemManager_H

// Includes
#include <MwtsCommon>
#include <qmobilityglobal.h>
#include <QOrganizerManager>
#include <QOrganizerItemId>
#include <QOrganizerEvent>
#include <QOrganizerEventOccurrence>
#include <QOrganizerJournal>
#include <QOrganizerNote>
#include <QOrganizerTodo>
#include <QOrganizerTodoOccurrence>
#include "PimItem.h"

// namespace
QTM_USE_NAMESPACE

/**
 * Helper class for managing calendar default items
 */
class PimOrganizerItemManager
{
public: // constructor & destructor

    /**
     * PimOrganizerItemManager constructor
     */
    PimOrganizerItemManager(MwtsConfig &dFetcher, MwtsResult& result);

    /**
     * PimOrganizerItemManager constructor
     */
    ~PimOrganizerItemManager();

public: // new functions

    /**
     * Sets measuring on/off
     */
    inline void SetMeasureState(bool state) { m_measureOn = state; }

    /**
     * Returns array of created items of this data store. If data store name not provided, all items are returned
     */
    QList<PimItem> getCreatedItems(QString dataStore="");

    /**
     * Used data store
     */
    void setCalendarDataStore(QString store);

    /**
     * Fetches calendar data from MwtsConfig
     */
    void initializeCalendarItemData();

    /**
     * Fetches modified calendar data from MwtsConfig
     */
    void initializeModifiedCalendarItemData();

    /**
     * Calendar item creation functions
     */
    bool CreateAndVerifyEventItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
    bool CreateAndVerifyEventOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
    bool CreateAndVerifyJournalItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
    bool CreateAndVerifyNoteItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
    bool CreateAndVerifyTodoItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);
    bool CreateAndVerifyTodoOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId &createdItem);

    /**
     * Calendar item modification functions, each item have specific fields to fill
     */
    bool ModifyAndVerifyEventItem(QOrganizerManager *manager, QOrganizerItemId itemId);
    bool ModifyAndVerifyEventOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId itemId);
    bool ModifyAndVerifyTodoItem(QOrganizerManager *manager, QOrganizerItemId itemId);
    bool ModifyAndVerifyTodoOccurrenceItem(QOrganizerManager *manager, QOrganizerItemId itemId);

private: // new functions

    /**
     * Saves the item to data store
     */
    bool saveItem(QOrganizerManager& manager, QOrganizerItem& item);

    /**
     * QOrganizerItem functionalities
     */
    bool AddOrganizerItemData(QOrganizerItem &item);
    bool VerifyOrganizerItemData(QOrganizerItem *item);

    /**
     * Data addition functions
     */
    void AddEventItemData(QOrganizerEvent &item);
    void AddEventOccurrenceItemData(QOrganizerEventOccurrence &item);
    void AddTodoItemData(QOrganizerTodo &item);
    void AddTodoOccurrenceItemData(QOrganizerTodoOccurrence &item);

    /**
     * Data verification functions
     */
    bool VerifyEventItemData(QOrganizerEvent *item);
    bool VerifyEventOccurrenceItemData(QOrganizerEventOccurrence *item);
    bool VerifyTodoItemData(QOrganizerTodo *item);
    bool VerifyTodoOccurrenceItemData(QOrganizerTodoOccurrence *item);

private: // data

    // for measuring time
    QTime m_time;

    // for fetching data
    MwtsConfig &m_detailFetcher;

    // for writing results
    MwtsResult& m_result;

    // defines whether measurement are made
    bool m_measureOn;

    // created items
    QList<PimItem> m_items;

    // used store
    QString m_calendarStore;

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
};

#endif // PimOrganizerItemManager_H
