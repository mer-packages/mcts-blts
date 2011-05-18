/* functions.cpp -- Test assets MIN functions
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
#include "FeedbackTest.h"

#include <MwtsCommon>
#include <iostream>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// Test class
FeedbackTest Test;


/**
 * Start effect
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if error ocuurs / state is nat changes to 'Running'
 */
LOCAL int StartEffect (MinItemParser * item)
{
        MWTS_ENTER;
        Test.StartEffect();
        return ENOERR;
}

/**
 * Pause effect
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if error ocuurs / state is nat changes to 'Paused'
 */
/*LOCAL int PauseEffect (MinItemParser * item)
{
        MWTS_ENTER;
        Test.PauseEffect();
        return ENOERR;
}*/


/**
 * Stop effect
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if error ocuurs / state is nat changes to 'Stopped'
 */
/*LOCAL int StopEffect (MinItemParser * item)
{
        MWTS_ENTER;
        Test.StopEffect();
        return ENOERR;
}*/

/**
 * Set duration
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetDuration (MinItemParser * item)
{
        MWTS_ENTER;
        int duration=0;
		if (mip_get_next_int( item, &duration) != ENOERR )
		{
                qCritical() << "No duration value given";
                return 1;
        }
        Test.SetDuration(duration);

        return ENOERR;
}

/**
 * Set intensity
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetIntensity (MinItemParser * item)
{

        MWTS_ENTER;
        double intensity=0.0;
        char* string = NULL;
		if (mip_get_next_string( item, &string) != ENOERR )
		{
                qCritical() << "No intensity value given";
                return 1;
        }
        intensity = atof(string);
        Test.SetIntensity(intensity);

        return ENOERR;
}

/**
 * Set attack duration
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetAttackTime (MinItemParser * item)
{
        MWTS_ENTER;
        int duration=0;
		if (mip_get_next_int( item, &duration) != ENOERR )
		{
                qCritical() << "No time value given";
                return 1;
        }
        Test.SetAttackTime(duration);

        return ENOERR;
}

/**
 * Set attack intensity
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetAttackIntensity (MinItemParser * item)
{

        MWTS_ENTER;
        double intensity=0.0;
        char* string = NULL;
		if (mip_get_next_string( item, &string) != ENOERR )
		{
                qCritical() << "No intensity value given";
                return 1;
        }
        intensity = atof(string);
        Test.SetAttackIntensity(intensity);

        return ENOERR;
}

/**
 * Set fade duration
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetFadeTime (MinItemParser * item)
{
        MWTS_ENTER;
        int duration=0;
		if (mip_get_next_int( item, &duration) != ENOERR )
		{
                qCritical() << "No time value given";
                return 1;
        }
        Test.SetFadeTime(duration);

        return ENOERR;
}

/**
 * Set fade intensity
 * @param item	MIN scripter parameters
 * @return ENOERR or 1 if item can't be read
 */
LOCAL int SetFadeIntensity (MinItemParser * item)
{

        MWTS_ENTER;
        double intensity=0.0;
        char* string = NULL;
		if (mip_get_next_string( item, &string) != ENOERR )
		{
                qCritical() << "No intensity value given";
                return 1;
        }
        intensity = atof(string);
        Test.SetFadeIntensity(intensity);

        return ENOERR;
}


/**
 * Function for MIN to gather our test case functions.
 * @param list	Functio pointer list, out
 * @return		ENOERR
 */
int ts_get_test_cases (DLList ** list) {
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

        ENTRYTC (*list, "StartEffect", StartEffect);
		//ENTRYTC (*list, "PauseEffect", PauseEffect);
		//ENTRYTC (*list, "StopEffect", StopEffect);
        ENTRYTC (*list, "SetDuration", SetDuration);
        ENTRYTC (*list, "SetIntensity", SetIntensity);
        ENTRYTC (*list, "SetAttackTime", SetAttackTime);
        ENTRYTC (*list, "SetAttackIntensity", SetAttackIntensity);
        ENTRYTC (*list, "SetFadeTime", SetFadeTime);
        ENTRYTC (*list, "SetFadeIntensity", SetFadeIntensity);
	return ENOERR;
}

