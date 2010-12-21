/* PimTest MIN interface
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

#include "stable.h"
#include "interface.h"
#include "contactstest.h"
#include "organizertest.h"
#include "PimTest.h"
#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test classes
PimTest test;


/***** Min scripter functions *****/

/**
 * Pim SetNumberOfItems function.
 * Sets number of created items (calendar and contact)
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetNumberOfItems (MinItemParser *item)
{
        MWTS_ENTER;

        char* string=NULL;
        int count=0;

        // get target
        if(mip_get_next_string( item, &string) != ENOERR )
        {
                qCritical() << "No target given";
                return ENOERR;
        }

        // get actual number of items
        if(mip_get_next_int( item, &count) != ENOERR )
        {
                qCritical() << "No item count given";
                return ENOERR;
        };

        QString target(string);
        if(target=="contacts")
        {
            test.ContactsTestImpl().SetNumberOfCreatedItems(count);
        }
        else if(target=="calendar")
        {
            test.OrganizerTestImpl().SetNumberOfCreatedItems(count);
        }
        else
        {
            qCritical()<<"Unknown target: "<<target;
        }

        return ENOERR;
}

/**
 * Pim SetIterationCount function.
 * Sets number of iterations to be performed
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetIterationCount (MinItemParser *item)
{
    MWTS_ENTER;

    char* string=NULL;
    int count=0;

    // get target
    if(mip_get_next_string( item, &string) != ENOERR )
    {
            qCritical() << "No target given";
            return ENOERR;
    }

    // get iteration count
    if(mip_get_next_int( item, &count) != ENOERR )
    {
            qCritical() << "No iteration count given";
            return ENOERR;
    };

    QString target(string);
    if(target=="contacts")
    {
        test.ContactsTestImpl().SetIterationCount(count);
    }
    else if(target=="calendar")
    {
        test.OrganizerTestImpl().SetIterationCount(count);
    }
    else
    {
        qCritical()<<"Unknown target: "<<target;
    }
    return 0;
}

/**
 * Pim SetMeasureState function.
 * Sets measuring on/off
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetMeasureState (MinItemParser *item)
{
    MWTS_ENTER;

    char* string=NULL;
    char* state=NULL;

    // get target
    if(mip_get_next_string( item, &string) != ENOERR)
    {
            qCritical() << "No target given";
            return ENOERR;
    }

    // get state
    if(mip_get_next_string( item, &state) != ENOERR)
    {
            qCritical() << "No measure state given";
            return ENOERR;
    };

    QString target(string);
    QString mState(state);

    // on / off
    bool measureOn = mState=="on";
    if(target=="contacts")
    {
        test.ContactsTestImpl().SetMeasureState(measureOn);
    }
    else if(target=="calendar")
    {
        test.OrganizerTestImpl().SetMeasureState(measureOn);
    }
    else
    {
        qCritical()<<"Unknown target: "<<target;
    }
    return 0;
}

/**
 * Pim SetCalendarItemType function.
 * Sets calendar item type to be created
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetCalendarItemType(MinItemParser *item)
{
        MWTS_ENTER;
        int itemType;
        char* string=NULL;
        if(mip_get_next_string( item, &string) != ENOERR )
        {
                qCritical() << "No calendar item type given";
                return ENOERR;
        }

        QString s(string);
        if(s=="event")
        {
            qDebug()<<"Created calendar item type: event";
            itemType = OrganizerTest::CalEvent;
        }
        else if(s=="eventoccurrence")
        {
            qDebug()<<"Created calendar item type: event occurrence";
            itemType = OrganizerTest::CalEventOccurrence;
        }
        else if(s=="journal")
        {
            qDebug()<<"Created calendar item type: journal";
            itemType = OrganizerTest::CalJournal;
        }
        else if(s=="note")
        {
            qDebug()<<"Created calendar item type: note";
            itemType = OrganizerTest::CalNote;
        }
        else if(s=="todo")
        {
            qDebug()<<"Created calendar item type: todo";
            itemType = OrganizerTest::CalTodo;
        }
        else if(s=="todooccurrence")
        {
            qDebug()<<"Created calendar item type: todo occurrence";
            itemType = OrganizerTest::CalTodoOccurrence;
        }
        else
        {
            qCritical()<<"Unknown calendar item type: "<<s;
            return ENOERR;
        }
        test.OrganizerTestImpl().SetCalendarItemType(itemType);
        return ENOERR;
}


/**
 * Pim SetCalendarDataStore function.
 * Sets calendar data store to be used
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetCalendarDataStore (MinItemParser *item)
{
        MWTS_ENTER;

        char* string=NULL;
        if(mip_get_next_string( item, &string) != ENOERR )
        {
                qCritical() << "No calendar data store name given";
                return ENOERR;
        }
        QString s(string);
        test.OrganizerTestImpl().SetCalendarDatastore(s);
        return ENOERR;
}


/**
 * Pim SetContactDataStore function.
 * Sets contact data store to be used
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SetContactDataStore (MinItemParser *item)
{
        MWTS_ENTER;

        char* string=NULL;
        if(mip_get_next_string( item, &string) != ENOERR )
        {
                qCritical() << "No contact data store name given";
                return ENOERR;
        }
        QString s(string);
        test.ContactsTestImpl().SetContactDatastore(s);
        return ENOERR;
}

/**
 * Pim CreateCalendarDataStore function.
 * Creates calendar data store based on name set in SetCalendarDataStore
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateCalendarDataStore (MinItemParser */*item*/)
{
        MWTS_ENTER;
        test.OrganizerTestImpl().CreateCalendarDatastore();
        return ENOERR;
}


/**
 * Pim CreateContactDataStore function.
 * Creates contact data store based on name set in SetContactDataStore
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateContactDataStore (MinItemParser *item)
{
        MWTS_ENTER;
        test.ContactsTestImpl().CreateContactDatastore();
        return ENOERR;
}

/**
 * Pim CreateAvailableCalendarDatastores function.
 * Creates all datastores found from the device by using name and URI.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateAvailableCalendarDatastores (MinItemParser * /*item*/)
{
	MWTS_ENTER;
        test.OrganizerTestImpl().CreateAvailableCalendarDatastores();
        return ENOERR;
}

/**
 * Pim CreateAvailableContactDatastores function.
 * Creates all datastores found from the device by using name and URI.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateAvailableContactDatastores (MinItemParser * /*item*/)
{
        MWTS_ENTER;
        test.ContactsTestImpl().CreateAvailableContactDatastores();
        return ENOERR;
}

/**
 * Pim ExportContactToVcard function.
 * Creates a new contact and exports vcard from it
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ExportContactToVcard (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().ExportContactToVcard();
    return ENOERR;
}

/**
 * Pim ImportContactFromVcard function.
 * Imports and saves contact from vcard
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ImportContactFromVcard (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().ImportContactFromVcard();
    return ENOERR;
}

/**
 * Pim ExportCalendarItemToIcalendar function.
 * Creates a new calendar item and exports iCalendar from it
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ExportCalendarItemToIcalendar (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().ExportCalendarItemToIcalendar();
    return ENOERR;
}

/**
 * Pim ImportCalendarItemFromIcalendar function.
 * Imports and saves calendar item from iCalendar
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ImportCalendarItemFromIcalendar (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().ImportCalendarItemFromIcalendar();
    return ENOERR;
}


/**
 * Pim CreateContacts function.
 * Creates new contacts. Number of contacts is defined in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateContacts (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().CreateContacts();
    return ENOERR;
}


/**
 * Pim ModifyContacts function.
 * Modifies created contacts
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ModifyContacts (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().ModifyContacts();
    return ENOERR;
}

/**
 * Pim RemoveContacts function.
 * Removes created contacts
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int RemoveContacts (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().RemoveContacts();
    return ENOERR;
}

/**
 * Pim SearchContacts function.
 * Searched contacts based on various filters
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SearchContacts (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.ContactsTestImpl().SearchContacts();
    return ENOERR;
}



/**
 * Pim CreateCalendarItems function.
 * Creates new calendar items. Item type is defined in
 * SetCalendarItemType. Used datastore is defined in SetCalendarDataStore,
 * and number of created items in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateCalendarItems (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().CreateCalendarItems();
    return ENOERR;
}

/**
 * Pim ModifyCalendarItems function.
 * Modifies calendar items. Item type is defined in
 * SetCalendarItemType. Used datastore is defined in SetCalendarDataStore,
 * and number of created items in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ModifyCalendarItems (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().ModifyCalendarItems();
    return ENOERR;
}

/**
 * Pim RemoveCalendarItems function.
 * Removes created items from data store
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int RemoveCalendarItems (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().RemoveCalendarItems();
    return ENOERR;
}


/**
 * Pim SearchDefaultCalendarItems function.
 * Searched default calendar items by using default filters
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SearchDefaultCalendarItems (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    test.OrganizerTestImpl().SearchDefaultCalendarItems();
    return ENOERR;
}


/**
 * Function for MIN to gather our test case functions.
 * @param list	Functio pointer list, out
 * @return		ENOERR
 */
int ts_get_test_cases (DLList ** list)
{
    MwtsMin::DeclareFunctions(list);
    ENTRYTC (*list, "SetNumberOfItems", SetNumberOfItems);
    ENTRYTC (*list, "SetIterationCount", SetIterationCount);
    ENTRYTC (*list, "SetMeasureState", SetMeasureState);
    ENTRYTC (*list, "SetCalendarItemType", SetCalendarItemType);
    ENTRYTC (*list, "SetCalendarDataStore", SetCalendarDataStore);
    ENTRYTC (*list, "SetContactDataStore", SetContactDataStore);
    ENTRYTC (*list, "CreateCalendarDataStore", CreateCalendarDataStore);
    ENTRYTC (*list, "CreateContactDataStore", CreateContactDataStore);
    ENTRYTC (*list, "CreateAvailableCalendarDatastores", CreateAvailableCalendarDatastores);
    ENTRYTC (*list, "CreateAvailableContactDatastores", CreateAvailableContactDatastores);
    ENTRYTC (*list, "ExportContactToVcard", ExportContactToVcard);
    ENTRYTC (*list, "ImportContactFromVcard", ImportContactFromVcard);
    ENTRYTC (*list, "ExportCalendarItemToIcalendar", ExportCalendarItemToIcalendar);
    ENTRYTC (*list, "ImportCalendarItemFromIcalendar", ImportCalendarItemFromIcalendar);
    ENTRYTC (*list, "CreateContacts", CreateContacts);
    ENTRYTC (*list, "ModifyContacts", ModifyContacts);
    ENTRYTC (*list, "RemoveContacts", RemoveContacts);
    ENTRYTC (*list, "SearchContacts", SearchContacts);
    ENTRYTC (*list, "CreateCalendarItems", CreateCalendarItems);
    ENTRYTC (*list, "ModifyCalendarItems", ModifyCalendarItems);
    ENTRYTC (*list, "RemoveCalendarItems", RemoveCalendarItems);
    ENTRYTC (*list, "SearchDefaultCalendarItems", SearchDefaultCalendarItems);
    return ENOERR;
}

