/* functions.cpp -- Test assets MIN functions

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
#include "interface.h"
#include "PimTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
PimTest Test;



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

        int count=0;
        if(mip_get_next_int( item, &count) != ENOERR )
        {
                qCritical() << "No iteration count given";
                return 1;
        };
        Test.SetNumberOfCreatedItems(count);
        return ENOERR;
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
                MWTS_ERROR("Unable to get needed device parameter");
                return 0;
        }

        /*
        Event = 0,
        EventOccurrence = 1
        Journal = 2
        Note = 3
        Todo = 4
        TodoOccurrence = 5
        */
        QString s(string);
        if(s=="event") {
            qDebug()<<"Created calendar item type: event";
            itemType = 0;
        } else if(s=="eventoccurrence") {
            qDebug()<<"Created calendar item type: event occurrence";
            itemType = 1;
        } else if(s=="journal") {
            qDebug()<<"Created calendar item type: journal";
            itemType = 2;
        } else if(s=="note") {
            qDebug()<<"Created calendar item type: note";
            itemType = 3;
        } else if(s=="todo") {
            qDebug()<<"Created calendar item type: todo";
            itemType = 4;
        } else if(s=="todooccurrence") {
            qDebug()<<"Created calendar item type: todo occurrence";
            itemType = 5;
        } else {
            qCritical()<<"***************************************** Unknown calendar item type: "<<s<<"********************************************";
            return 0;
        }
        Test.SetCalendarItemType(itemType);
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
                MWTS_ERROR("Unable to get needed device parameter");
                return 0;
        }
        QString s(string);
        qDebug()<<"Set calendar store: "<<s;
        Test.SetCalendarDatastore(s);
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
                MWTS_ERROR("Unable to get needed device parameter");
                return 0;
        }
        QString s(string);
        qDebug()<<"Set contact store: "<<s;
        Test.SetContactDatastore(s);
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
        Test.CreateAvailableCalendarDatastores();
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
        Test.CreateAvailableContactDatastores();
        return ENOERR;
}

/**
 * Pim ExportContactFromVcard function.
 * Creates a new contact and exports vcard from it
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ExportContactFromVcard (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestExportingVcardFromContact();
    return ENOERR;
}

/**
 * Pim ImportVcardToContact function.
 * Imports and saves contact from vcard
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int ImportVcardToContact (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestImportingVcardToContact();
    return ENOERR;
}

/**
 * Pim CreateAndDeleteContact function.
 * Creates a new contact and deletes it. Amount of contacts is defined in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateAndDeleteContact (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestContactItemCreationDeleting();
    return ENOERR;
}


/**
 * Pim CreateModifyAndDeleteContacts function.
 * Creates a new contact, modifies it and deletes it. Amount of contacts is defined in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateModifyAndDeleteContacts (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestContactItemCreationModifyingDeleting();
    return ENOERR;
}

/**
 * Pim CreateAndDeleteCalendarItem function.
 * Creates a new calendar item and deletes it. Item type is defined in
 * SetCalendarItemType, datastore in SetCalendarDataStore, and amount in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateAndDeleteCalendarItem (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestCalendarItemCreationDeleting();
    return ENOERR;
}

/**
 * Pim CreateModifyAndDeleteCalendarItem function.
 * Creates a new calendar item, modifies it and removes it. Item type is defined in
 * SetCalendarItemType, datastore in SetCalendarDataStore, and amount in SetNumberOfItems
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int CreateModifyAndDeleteCalendarItem (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestCalendarItemCreationModifyingDeleting();
    return ENOERR;
}



/**
 * Pim SearchDefaultCalendarItems function.
 * Creates default calendar items and searched them by using default filters
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */
LOCAL int SearchDefaultCalendarItems (MinItemParser * /*item*/)
{
    MWTS_ENTER;
    Test.TestItemSearching();
    return ENOERR;
}

/**
 * Function for MIN to gather our test case functions.
 * @param list	Functio pointer list, out
 * @return		ENOERR
 */
int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

        ENTRYTC (*list, "SetNumberOfItems", SetNumberOfItems);
        ENTRYTC (*list, "SetCalendarItemType", SetCalendarItemType);
        ENTRYTC (*list, "SetCalendarDataStore", SetCalendarDataStore);
        ENTRYTC (*list, "SetContactDataStore", SetContactDataStore);
        ENTRYTC (*list, "CreateAvailableCalendarDatastores", CreateAvailableCalendarDatastores);
        ENTRYTC (*list, "CreateAvailableContactDatastores", CreateAvailableContactDatastores);
        ENTRYTC (*list, "ExportContactFromVcard", ExportContactFromVcard);
        ENTRYTC (*list, "ImportVcardToContact", ImportVcardToContact);
        ENTRYTC (*list, "CreateAndDeleteContact", CreateAndDeleteContact);
        ENTRYTC (*list, "CreateModifyAndDeleteContacts", CreateModifyAndDeleteContacts);
        ENTRYTC (*list, "CreateAndDeleteCalendarItem", CreateAndDeleteCalendarItem);
        ENTRYTC (*list, "CreateModifyAndDeleteCalendarItem", CreateModifyAndDeleteCalendarItem);
        ENTRYTC (*list, "SearchDefaultCalendarItems", SearchDefaultCalendarItems);

        return ENOERR;
}

