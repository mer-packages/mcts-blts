/*
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
#include "LocationTest.h"

#include <MwtsCommon>

const char *module_date = __DATE__;
const char *module_time = __TIME__;

LocationTest Test;


LOCAL int SetPositioningMethod(MinItemParser * item)
{
	char* string=NULL;
	if(mip_get_next_string( item, &string) != ENOERR )
	{
		qCritical() << "no parameter for SetPositioningMethod given";
		return 1;
	}


	if(0==strcmp(string, "ALL"))
		Test.SetPositioningMethod(METHOD_ALL);
	else if(0==strcmp(string, "SATELLITE"))
		Test.SetPositioningMethod(METHOD_SATELLITE);
	else if(0==strcmp(string, "NON_SATELLITE"))
		Test.SetPositioningMethod(METHOD_NON_SATELLITE);
	else
	{
		qCritical() << "Invalid parameter for SetPositioningMethod : " << string;
	 	return 1;
	}
	return 0;

}

LOCAL int SetHotMode(MinItemParser * item)
{
	char* string=NULL;
	if(mip_get_next_string( item, &string) != ENOERR )
	{
		qCritical() << "no parameter for SetHotMode given";
		return 1;
	}


	if(0==strcmp(string, "HOT"))
		Test.SetHotMode(MODE_HOT);
	else if(0==strcmp(string, "COLD"))
		Test.SetHotMode(MODE_COLD);
	else
	{
		qCritical() << "Invalid parameter for SetHotMode : " << string;
	 	return 1;
	}
	return 0;

}


LOCAL int GetLocationFix (MinItemParser * item)
{
	MWTS_ENTER;
	Test.GetLocationFix();
	return ENOERR;
}


LOCAL int TestLocationFix (MinItemParser * item)
{
	MWTS_ENTER;
	Test.TestLocationFix();
	return ENOERR;
}


LOCAL int CalculateDistances (MinItemParser * item)
{
	MWTS_ENTER;
	Test.CalculateDistances();
	return ENOERR;
}

int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);

	ENTRYTC (*list, "SetPositioningMethod", SetPositioningMethod);
	ENTRYTC (*list, "SetHotMode", SetHotMode);
	ENTRYTC (*list, "GetLocationFix", GetLocationFix);
	ENTRYTC (*list, "TestLocationFix", TestLocationFix);
	ENTRYTC (*list, "CalculateDistances", CalculateDistances);

	return ENOERR;
}

