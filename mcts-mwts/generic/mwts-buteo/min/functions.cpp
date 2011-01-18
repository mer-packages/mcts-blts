/* functions.cpp -- min interface implementation

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
#include "ButeoTest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
ButeoTest Test;

/**
 * just demonstrating how to create MIN functions.
 * @param item	MIN scripter parameters
 * @return		ENOERR
 */

/*
* Initiates syncing to target profile. Profile must be created beforehand
*/
LOCAL int Sync (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;
        //char* account_name = NULL;

        char* profile_name = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_name))
        {
                qCritical() << "Missing parameter: account_name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.Sync(profile_name);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

/*
 * creates profile for device to device bluetooth sync. Parameters come from config-file
 */
LOCAL int CreateBtProfile (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_name = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_name))
        {
                qCritical() << "Missing parameter: profile_path";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateBtProfile(profile_name);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int CreateGoogleProfile (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_path = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_path))
        {
                qCritical() << "Missing parameter: profile_path";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateGoogleProfile(profile_path);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int CreateMemotooProfile (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_path = NULL;
        //char* profile_type = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_path))
        {
                qCritical() << "Missing parameter: profile_path";
                MWTS_LEAVE;
                return 1;
        }

        /*
        if (ENOERR != mip_get_next_string(item, &profile_type))
        {
                qCritical() << "Missing parameter: profile_type";
                MWTS_LEAVE;
                return 1;
        } */

        success = Test.CreateMemotooProfile(profile_path);
        qDebug() << "THIS IS FUNCTIONS!";

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int CreateMobicalProfile (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_path = NULL;
        //char* profile_type = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_path))
        {
                qCritical() << "Missing parameter: profile_path";
                MWTS_LEAVE;
                return 1;
        }

        /*
        if (ENOERR != mip_get_next_string(item, &profile_type))
        {
                qCritical() << "Missing parameter: profile_type";
                MWTS_LEAVE;
                return 1;
        } */

        success = Test.CreateMobicalProfile(profile_path);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int CreateOviProfile (MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_path = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_path))
        {
                qCritical() << "Missing parameter: profile_path";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.CreateOviProfile(profile_path);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}



LOCAL int ListProfiles(MinItemParser * item)
{
        MWTS_ENTER;

        bool success = true;

        Test.ListProfiles();

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int RemoveProfile(MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        char* profile_name = NULL;

        if (ENOERR != mip_get_next_string(item, &profile_name))
        {
                qCritical() << "Missing parameter: profile_name";
                MWTS_LEAVE;
                return 1;
        }

        success = Test.RemoveProfile(profile_name);

        g_pResult->StepPassed( __FUNCTION__, success );
        return ENOERR;
}

LOCAL int RemoveProfiles(MinItemParser * item)
{
        MWTS_ENTER;

        bool success = false;

        success = Test.RemoveProfiles();

        g_pResult->StepPassed( __FUNCTION__, success );
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

        ENTRYTC (*list, "Sync", Sync);
        ENTRYTC (*list, "RemoveProfile", RemoveProfile);
        ENTRYTC (*list, "RemoveProfiles", RemoveProfiles);
        ENTRYTC (*list, "ListProfiles", ListProfiles);
        ENTRYTC (*list, "CreateBtProfile", CreateBtProfile);
        ENTRYTC (*list, "CreateGoogleProfile", CreateGoogleProfile);
        ENTRYTC (*list, "CreateMemotooProfile", CreateMemotooProfile);
        ENTRYTC (*list, "CreateMobicalProfile", CreateMobicalProfile);
        ENTRYTC (*list, "CreateOviProfile", CreateOviProfile);

	return ENOERR;
}

